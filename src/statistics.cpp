#include "mb/alloc-statistics/new_statistics.hpp"

#include <atomic>
#include <cstddef>

namespace mb::alloc_statistics {
void Statistics::new_call(Bytes size, void* p) noexcept {
    new_calls_.fetch_add(1, std::memory_order_relaxed);
    const size_t current = current_size_.fetch_add(size.value, std::memory_order_seq_cst) + size.value;
    // Use the size at this allocation's atomic addition, not a later load: another thread
    // may deallocate memory in between and make a later load miss the actual peak.
    size_t prev = peak_size_.load(std::memory_order_relaxed);
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
