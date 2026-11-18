#include "mb/alloc-statistics/new_statistics.hpp"

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <new>

namespace as = mb::alloc_statistics;

int main() {
    const auto& stats = as::Statistics::instance();
    constexpr std::size_t allocation_size{32};

    const auto new_calls_before = stats.new_calls();
    const auto delete_calls_before = stats.delete_calls();
    const auto allocated_before = stats.allocated_size();
    const auto deallocated_before = stats.deallocated_size();

    void* const p = ::operator new(allocation_size);
    const auto new_calls_after = stats.new_calls();
    const auto allocated_after = stats.allocated_size();
    ::operator delete(p);
    const auto delete_calls_after = stats.delete_calls();
    const auto deallocated_after = stats.deallocated_size();

    const bool passed = new_calls_after == new_calls_before + 1U && delete_calls_after == delete_calls_before + 1U
                        && allocated_after == allocated_before + as::Bytes{allocation_size}
                        && deallocated_after == deallocated_before + as::Bytes{allocation_size};
    if (!passed)
        return EXIT_FAILURE;

    std::cout << "PASSED (" << __FILE__ << ")\n";
    return EXIT_SUCCESS;
}
