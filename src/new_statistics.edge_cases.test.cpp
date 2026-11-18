#include "mb/alloc-statistics/new_statistics.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>

namespace as = mb::alloc_statistics;

namespace {
#define AS_CHECK(condition) \
    do { \
        if (!(condition)) { \
            std::cerr << "Assertion failed at " << __FILE__ << ':' << __LINE__ << ": " #condition << '\n'; \
            return EXIT_FAILURE; \
        } \
    } while (false)

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4'324)
#endif
struct alignas(64) OverAligned {
    unsigned char value;
};
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

void pin_heap_allocation([[maybe_unused]] void* p) noexcept {
#if defined(__GNUC__)
    asm volatile("" : : "g"(p) : "memory");
#endif
}
} // namespace

int main() {
    const auto& stats = as::Statistics::instance();
    constexpr auto max_size = std::numeric_limits<std::size_t>::max();

    // Header arithmetic must not wrap an impossible request into a tiny allocation.
    bool scalar_threw = false;
    void* scalar = nullptr;
    try {
        scalar = ::operator new(max_size);
    } catch (const std::bad_alloc&) {
        scalar_threw = true;
    }
    if (scalar)
        ::operator delete(scalar);
    AS_CHECK(scalar_threw);

    bool array_threw = false;
    void* array = nullptr;
    try {
        array = ::operator new[](max_size);
    } catch (const std::bad_alloc&) {
        array_threw = true;
    }
    if (array)
        ::operator delete[](array);
    AS_CHECK(array_threw);

    void* const nothrow_scalar = ::operator new(max_size, std::nothrow);
    if (nothrow_scalar)
        ::operator delete(nothrow_scalar);
    AS_CHECK(nothrow_scalar == nullptr);

    void* const nothrow_array = ::operator new[](max_size, std::nothrow);
    if (nothrow_array)
        ::operator delete[](nothrow_array);
    AS_CHECK(nothrow_array == nullptr);

#if defined(__cpp_aligned_new)
    constexpr std::size_t allocation_size{128};
    constexpr auto alignment = std::align_val_t{64};

    auto allocated_before = stats.allocated_size();
    auto deallocated_before = stats.deallocated_size();
    auto new_calls_before = stats.new_calls();
    auto delete_calls_before = stats.delete_calls();

    void* const aligned = ::operator new(allocation_size, alignment);
    AS_CHECK(reinterpret_cast<std::uintptr_t>(aligned) % static_cast<std::size_t>(alignment) == 0U);
    AS_CHECK(stats.new_calls() == new_calls_before + 1U);
    AS_CHECK(stats.allocated_size() == allocated_before + as::Bytes{allocation_size});

    // Exercise the sized, aligned replacement where the compiler exposes sized deallocation.
#if defined(__cpp_sized_deallocation)
    ::operator delete(aligned, allocation_size, alignment);
#else
    ::operator delete(aligned, alignment);
#endif
    AS_CHECK(stats.delete_calls() == delete_calls_before + 1U);
    AS_CHECK(stats.deallocated_size() == deallocated_before + as::Bytes{allocation_size});

    allocated_before = stats.allocated_size();
    deallocated_before = stats.deallocated_size();
    new_calls_before = stats.new_calls();
    delete_calls_before = stats.delete_calls();

    void* const aligned_array = ::operator new[](allocation_size, alignment, std::nothrow);
    AS_CHECK(aligned_array != nullptr);
    AS_CHECK(reinterpret_cast<std::uintptr_t>(aligned_array) % static_cast<std::size_t>(alignment) == 0U);
    AS_CHECK(stats.new_calls() == new_calls_before + 1U);
    AS_CHECK(stats.allocated_size() == allocated_before + as::Bytes{allocation_size});

    ::operator delete[](aligned_array, alignment);
    AS_CHECK(stats.delete_calls() == delete_calls_before + 1U);
    AS_CHECK(stats.deallocated_size() == deallocated_before + as::Bytes{allocation_size});

    allocated_before = stats.allocated_size();
    deallocated_before = stats.deallocated_size();
    new_calls_before = stats.new_calls();
    delete_calls_before = stats.delete_calls();

    auto* const over_aligned = new OverAligned;
    pin_heap_allocation(over_aligned);
    AS_CHECK(stats.new_calls() == new_calls_before + 1U);
    AS_CHECK(stats.allocated_size() == allocated_before + as::Bytes{sizeof(OverAligned)});
    delete over_aligned;
    AS_CHECK(stats.delete_calls() == delete_calls_before + 1U);
    AS_CHECK(stats.deallocated_size() == deallocated_before + as::Bytes{sizeof(OverAligned)});

    allocated_before = stats.allocated_size();
    deallocated_before = stats.deallocated_size();
    new_calls_before = stats.new_calls();
    delete_calls_before = stats.delete_calls();
    std::size_t expected_new_calls{};
    as::Bytes expected_allocated;
    constexpr std::size_t allocation_sizes[]{0U, 1U, 7U, 31U, 128U, 1'024U};
    for (std::size_t current_alignment = 1U; current_alignment <= 1'024U; current_alignment *= 2U) {
        const auto current_align_val = std::align_val_t{current_alignment};
        for (const auto size : allocation_sizes) {
            void* const first = ::operator new(size, current_align_val, std::nothrow);
            void* const second = ::operator new[](size, current_align_val, std::nothrow);
            AS_CHECK(first != nullptr);
            AS_CHECK(second != nullptr);
            AS_CHECK(first != second);
            AS_CHECK(reinterpret_cast<std::uintptr_t>(first) % current_alignment == 0U);
            AS_CHECK(reinterpret_cast<std::uintptr_t>(second) % current_alignment == 0U);
            std::fill_n(static_cast<std::byte*>(first), size, std::byte{0x5a});
            std::fill_n(static_cast<std::byte*>(second), size, std::byte{0xa5});

            expected_new_calls += 2U;
            expected_allocated += as::Bytes{size * 2U};
            ::operator delete(first, current_align_val);
            ::operator delete[](second, current_align_val);
        }
    }
    AS_CHECK(stats.new_calls() == new_calls_before + expected_new_calls);
    AS_CHECK(stats.delete_calls() == delete_calls_before + expected_new_calls);
    AS_CHECK(stats.allocated_size() == allocated_before + expected_allocated);
    AS_CHECK(stats.deallocated_size() == deallocated_before + expected_allocated);

    void* const aligned_overflow = ::operator new(max_size, alignment, std::nothrow);
    if (aligned_overflow)
        ::operator delete(aligned_overflow, alignment);
    AS_CHECK(aligned_overflow == nullptr);
#endif

    return EXIT_SUCCESS;
}
