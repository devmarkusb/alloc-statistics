#include "mb/alloc-statistics/new_statistics.hpp"

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <memory>
#include <new>

// Round up to a whole number of fundamental-alignment units so raw + header_size
// retains malloc's alignment guarantee and leaves room for the recorded size.
constexpr size_t max_alignment = alignof(std::max_align_t);
constexpr size_t header_size = ((sizeof(size_t) + max_alignment - 1U) / max_alignment) * max_alignment;
static_assert(header_size % max_alignment == 0U);

#ifdef __clang__
#pragma clang diagnostic push
#ifdef __has_warning
#if __has_warning("-Wallocator-wrappers")
#pragma clang diagnostic ignored "-Wallocator-wrappers"
#endif
#endif
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#endif

namespace mb::alloc_statistics {
Statistics& Statistics::instance() {
    static Statistics stats;
    return stats;
}
} // namespace mb::alloc_statistics

namespace {
// NOLINTNEXTLINE(llvm-prefer-static-over-anonymous-namespace)
[[nodiscard]] void* allocate_with_stats(size_t size_in_bytes) noexcept {
    if (size_in_bytes > std::numeric_limits<size_t>::max() - header_size)
        return nullptr;

    void* const raw = std::malloc(header_size + size_in_bytes); // NOLINT(cppcoreguidelines-no-malloc,hicpp-no-malloc)
    if (!raw)
        return nullptr;
    mb::alloc_statistics::Statistics::instance().new_call(mb::alloc_statistics::Bytes{size_in_bytes}, raw);
    return static_cast<char*>(raw) + header_size; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
}

// NOLINTNEXTLINE(llvm-prefer-static-over-anonymous-namespace)
void* allocate_or_throw(size_t size_in_bytes) {
    if (void* const p = allocate_with_stats(size_in_bytes))
        return p;
    throw std::bad_alloc{};
}

#ifdef __cpp_aligned_new
// NOLINTNEXTLINE(llvm-prefer-static-over-anonymous-namespace)
void* allocate_aligned_with_stats(size_t size_in_bytes, std::align_val_t alignment_value) noexcept {
    const auto alignment = static_cast<size_t>(alignment_value);
    if (alignment == 0U || (alignment & (alignment - 1U)) != 0U)
        return nullptr;

    // new_call() stores the size at the malloc result. An additional raw-pointer
    // field immediately before the aligned user pointer lets aligned delete find it.
    constexpr size_t prefix_size = header_size + sizeof(void*);
    size_t total_size = prefix_size;
    if (size_in_bytes > std::numeric_limits<size_t>::max() - total_size)
        return nullptr;
    total_size += size_in_bytes;
    if (alignment - 1U > std::numeric_limits<size_t>::max() - total_size)
        return nullptr;
    total_size += alignment - 1U;

    void* const raw = std::malloc(total_size); // NOLINT(cppcoreguidelines-no-malloc,hicpp-no-malloc)
    if (!raw)
        return nullptr;

    void* aligned = static_cast<char*>(raw) + prefix_size; // NOLINT
    size_t space = total_size - prefix_size;
    if (!std::align(alignment, size_in_bytes, aligned, space)) {
        std::free(raw); // NOLINT(cppcoreguidelines-no-malloc,hicpp-no-malloc)
        return nullptr;
    }

    std::memcpy(static_cast<char*>(aligned) - sizeof(raw), &raw, sizeof(raw)); // NOLINT
    mb::alloc_statistics::Statistics::instance().new_call(mb::alloc_statistics::Bytes{size_in_bytes}, raw);
    return aligned;
}

// NOLINTNEXTLINE(llvm-prefer-static-over-anonymous-namespace)
void* allocate_aligned_or_throw(size_t size_in_bytes, std::align_val_t alignment) {
    if (void* const p = allocate_aligned_with_stats(size_in_bytes, alignment))
        return p;
    throw std::bad_alloc{};
}

// NOLINTNEXTLINE(llvm-prefer-static-over-anonymous-namespace)
void deallocate_aligned_with_stats(const void* p) noexcept {
    if (!p)
        return;

    void* raw = nullptr;
    std::memcpy(&raw, static_cast<const char*>(p) - sizeof(raw), sizeof(raw)); // NOLINT
    mb::alloc_statistics::Statistics::instance().delete_call(raw);
    std::free(raw); // NOLINT(cppcoreguidelines-no-malloc,hicpp-no-malloc)
}
#endif
} // namespace

// NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name)
void* operator new(size_t size_in_bytes) {
    return allocate_or_throw(size_in_bytes);
}

// NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name)
void* operator new(size_t size_in_bytes, const std::nothrow_t&) noexcept {
    return allocate_with_stats(size_in_bytes);
}

// NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name)
void* operator new[](size_t size_in_bytes) {
    return allocate_or_throw(size_in_bytes);
}

// NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name)
void* operator new[](size_t size_in_bytes, const std::nothrow_t&) noexcept {
    return allocate_with_stats(size_in_bytes);
}

void operator delete(void* p) noexcept {
    if (!p)
        return;
    void* const raw = static_cast<char*>(p) - header_size; // NOLINT
    mb::alloc_statistics::Statistics::instance().delete_call(raw);
    std::free(raw); // NOLINT(cppcoreguidelines-no-malloc,hicpp-no-malloc)
}

void operator delete[](void* p) noexcept {
    if (!p)
        return;
    void* const raw = static_cast<char*>(p) - header_size; // NOLINT
    mb::alloc_statistics::Statistics::instance().delete_call(raw);
    std::free(raw); // NOLINT(cppcoreguidelines-no-malloc,hicpp-no-malloc)
}

// Sized deallocation overloads (C++14): forward to the unsized versions.
// Standard headers can hide these declarations when the compiler's sized-deallocation
// feature is disabled, even though the replacement functions remain valid.
#ifndef __cpp_sized_deallocation
void operator delete(void*, size_t) noexcept;
void operator delete[](void*, size_t) noexcept;
#endif

void operator delete(void* p, size_t /*unused*/) noexcept {
    ::operator delete(p);
}

void operator delete[](void* p, size_t /*unused*/) noexcept {
    ::operator delete[](p);
}

#ifdef __cpp_aligned_new
// NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name)
void* operator new(size_t size_in_bytes, std::align_val_t alignment) {
    return allocate_aligned_or_throw(size_in_bytes, alignment);
}

// NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name)
void* operator new(size_t size_in_bytes, std::align_val_t alignment, const std::nothrow_t&) noexcept {
    return allocate_aligned_with_stats(size_in_bytes, alignment);
}

// NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name)
void* operator new[](size_t size_in_bytes, std::align_val_t alignment) {
    return allocate_aligned_or_throw(size_in_bytes, alignment);
}

// NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name)
void* operator new[](size_t size_in_bytes, std::align_val_t alignment, const std::nothrow_t&) noexcept {
    return allocate_aligned_with_stats(size_in_bytes, alignment);
}

// The language fixes global deallocation parameters as void*, not const void*.
// NOLINTNEXTLINE(misc-const-correctness)
void operator delete(void* p, std::align_val_t /*unused*/) noexcept {
    deallocate_aligned_with_stats(p);
}

// NOLINTNEXTLINE(misc-const-correctness)
void operator delete[](void* p, std::align_val_t /*unused*/) noexcept {
    deallocate_aligned_with_stats(p);
}

// Sized, aligned deallocation overloads (C++17): forward to the unsized versions.
#ifndef __cpp_sized_deallocation
void operator delete(void*, size_t, std::align_val_t) noexcept;
void operator delete[](void*, size_t, std::align_val_t) noexcept;
#endif

void operator delete(void* p, size_t /*unused*/, std::align_val_t alignment) noexcept {
    ::operator delete(p, alignment);
}

void operator delete[](void* p, size_t /*unused*/, std::align_val_t alignment) noexcept {
    ::operator delete[](p, alignment);
}
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif
