#include "mb/alloc-statistics/new_statistics.hpp"
#include <cstdlib>
#include <new>

namespace {
// Must be at least alignof(std::max_align_t) so the user pointer returned by operator new
// retains the alignment guarantee from malloc, and at least sizeof(size_t) to hold the size.
constexpr size_t k_header_size = sizeof(size_t) > alignof(std::max_align_t) ? sizeof(size_t)
                                                                            : alignof(std::max_align_t);
} // namespace

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#endif

namespace mb::alloc_statistics {
void Statistics::new_call(Bytes size, void* p) noexcept {
    new_calls_.fetch_add(1, std::memory_order_relaxed);
    current_size_.fetch_add(size.value, std::memory_order_seq_cst);
    // Atomically update peak_size_ to max(peak_size_, current_size_) via CAS loop.
    size_t prev = peak_size_.load(std::memory_order_relaxed);
    const size_t current = current_size_.load(std::memory_order_seq_cst);
    while (current > prev && !peak_size_.compare_exchange_weak(prev, current, std::memory_order_relaxed)) {
    }
    allocated_size_.fetch_add(size.value, std::memory_order_relaxed);
    *static_cast<size_t*>(p) = size.value; // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
}

void Statistics::delete_call(const void* p) noexcept {
    delete_calls_.fetch_add(1, std::memory_order_relaxed);
    const auto size = *static_cast<const size_t*>(p); // NOLINT
    current_size_.fetch_sub(size, std::memory_order_seq_cst);
    deallocated_size_.fetch_add(size, std::memory_order_relaxed);
}
} // namespace mb::alloc_statistics

namespace {
[[nodiscard]] void* allocate_with_stats(size_t size_in_bytes) noexcept {
    void* const raw = std::malloc(k_header_size + size_in_bytes); // NOLINT(cppcoreguidelines-no-malloc)
    if (!raw)
        return nullptr;
    mb::alloc_statistics::Statistics::instance().new_call(mb::alloc_statistics::Bytes{size_in_bytes}, raw);
    return static_cast<char*>(raw) + k_header_size; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
}
} // namespace

void* operator new(size_t size_in_bytes) {
    void* const p = allocate_with_stats(size_in_bytes);
    if (!p)
        throw std::bad_alloc{};
    return p;
}

void* operator new(size_t size_in_bytes, const std::nothrow_t&) noexcept {
    return allocate_with_stats(size_in_bytes);
}

void* operator new[](size_t size_in_bytes) {
    void* const p = allocate_with_stats(size_in_bytes);
    if (!p)
        throw std::bad_alloc{};
    return p;
}

void* operator new[](size_t size_in_bytes, const std::nothrow_t&) noexcept {
    return allocate_with_stats(size_in_bytes);
}

void operator delete(void* p) noexcept {
    if (!p)
        return;
    void* const raw = static_cast<char*>(p) - k_header_size; // NOLINT
    mb::alloc_statistics::Statistics::instance().delete_call(raw);
    std::free(raw); // NOLINT(cppcoreguidelines-no-malloc)
}

void operator delete[](void* p) noexcept {
    if (!p)
        return;
    void* const raw = static_cast<char*>(p) - k_header_size; // NOLINT
    mb::alloc_statistics::Statistics::instance().delete_call(raw);
    std::free(raw); // NOLINT(cppcoreguidelines-no-malloc)
}

// Sized deallocation overloads (C++14): forward to the unsized versions.
void operator delete(void* p, size_t /*unused*/) noexcept;

void operator delete(void* p, size_t /*unused*/) noexcept {
    ::operator delete(p);
}

void operator delete[](void* p, size_t /*unused*/) noexcept;

void operator delete[](void* p, size_t /*unused*/) noexcept {
    ::operator delete[](p);
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif
