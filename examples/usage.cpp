#include "mb/alloc-statistics/new_statistics.hpp"

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>

namespace as = mb::alloc_statistics;

namespace {
struct Snapshot {
    std::size_t new_calls{};
    std::size_t delete_calls{};
    as::Bytes allocated_size{};
    as::Bytes deallocated_size{};
    as::Bytes peak_size{};
};

// NOLINTNEXTLINE(llvm-prefer-static-over-anonymous-namespace)
[[nodiscard]] Snapshot snapshot() noexcept {
    const auto& stats = as::Statistics::instance();
    return Snapshot{
        .new_calls = stats.new_calls(),
        .delete_calls = stats.delete_calls(),
        .allocated_size = stats.allocated_size(),
        .deallocated_size = stats.deallocated_size(),
        .peak_size = stats.peak_size(),
    };
}

// NOLINTNEXTLINE(llvm-prefer-static-over-anonymous-namespace)
void pin_heap_allocation(const void* p) noexcept {
    const void* const volatile pinned = p;
    static_cast<void>(pinned);
}

// NOLINTNEXTLINE(llvm-prefer-static-over-anonymous-namespace)
void run_tracked_workload() {
    constexpr std::size_t value_count{128};
    constexpr int initial_value{42};
    constexpr std::size_t buffer_size{4'096};
    constexpr auto marker_byte = std::byte{0x2a};

    const auto value = std::make_unique<int>(initial_value);
    pin_heap_allocation(value.get());

    std::vector<int> values;
    values.reserve(value_count);
    pin_heap_allocation(values.data());
    for (std::size_t i = 0; i != value_count; ++i)
        values.push_back(static_cast<int>(i));

    std::vector<std::byte> buffer(buffer_size);
    pin_heap_allocation(buffer.data());
    buffer.front() = marker_byte;
    pin_heap_allocation(&buffer.front());
}

struct AllocationReport {
    AllocationReport() {
        as::Statistics::instance().reset();
    }

    ~AllocationReport() {
        try {
            const auto stats = snapshot();
            const auto outstanding = stats.allocated_size - stats.deallocated_size;

            std::cout << "tracked new calls: " << stats.new_calls << '\n';
            std::cout << "tracked delete calls: " << stats.delete_calls << '\n';
            std::cout << "allocated bytes: " << stats.allocated_size << '\n';
            std::cout << "deallocated bytes: " << stats.deallocated_size << '\n';
            std::cout << "outstanding bytes: " << outstanding << '\n';
            std::cout << "peak bytes: " << stats.peak_size << '\n';
        } catch (...) {
        }
    }
};
} // namespace

int main() {
    try {
        const AllocationReport report;
        run_tracked_workload();
        return EXIT_SUCCESS;
    } catch (...) {
        return EXIT_FAILURE;
    }
}
