#include "mb/alloc-statistics/new_statistics.hpp"

#include <array>
#include <barrier>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <thread>

// Keep this bookkeeping-only executable independent of the interposition object.
namespace mb::alloc_statistics {
Statistics& Statistics::instance() {
    static Statistics stats;
    return stats;
}
} // namespace mb::alloc_statistics

namespace as = mb::alloc_statistics;

namespace {
#define AS_CHECK(condition) \
    do { \
        if (!(condition)) { \
            std::fprintf(stderr, "Assertion failed at %s:%d: %s\n", __FILE__, __LINE__, #condition); \
            return EXIT_FAILURE; \
        } \
    } while (false)

constexpr std::size_t thread_count{4};
constexpr std::size_t allocation_size{100};

struct ConcurrentResult {
    bool allocations_ok{};
    bool deallocations_ok{};
};

// This target deliberately does not link the global new/delete replacements. That keeps
// runtime-owned thread allocations out of these focused counters and permits TSan linking.
ConcurrentResult run_concurrent_test() {
    auto& stats = as::Statistics::instance();
    stats.reset();

    std::barrier<> ready(static_cast<std::ptrdiff_t>(thread_count + 1U));
    std::barrier<> run(static_cast<std::ptrdiff_t>(thread_count + 1U));
    std::barrier<> allocated(static_cast<std::ptrdiff_t>(thread_count + 1U));
    std::barrier<> release_allocations(static_cast<std::ptrdiff_t>(thread_count + 1U));
    std::barrier<> done(static_cast<std::ptrdiff_t>(thread_count + 1U));
    std::barrier<> finish(static_cast<std::ptrdiff_t>(thread_count + 1U));

    std::array<std::thread, thread_count> threads;
    for (auto& thread : threads) {
        thread = std::thread([&] {
            std::size_t header{};
            ready.arrive_and_wait();
            run.arrive_and_wait();
            stats.new_call(as::Bytes{allocation_size}, &header);
            allocated.arrive_and_wait();
            release_allocations.arrive_and_wait();
            stats.delete_call(&header);
            done.arrive_and_wait();
            finish.arrive_and_wait();
        });
    }

    ConcurrentResult result{};
    ready.arrive_and_wait();
    run.arrive_and_wait();
    allocated.arrive_and_wait();
    result.allocations_ok =
        stats.new_calls() == thread_count && stats.delete_calls() == 0U
        && stats.allocated_size() == as::Bytes{thread_count * allocation_size}
        && stats.deallocated_size() == as::Bytes{} && stats.peak_size() == as::Bytes{thread_count * allocation_size};

    release_allocations.arrive_and_wait();
    done.arrive_and_wait();
    result.deallocations_ok =
        stats.new_calls() == thread_count && stats.delete_calls() == thread_count
        && stats.allocated_size() == as::Bytes{thread_count * allocation_size}
        && stats.deallocated_size() == as::Bytes{thread_count * allocation_size}
        && stats.allocated_size() - stats.deallocated_size() == as::Bytes{}
        && stats.peak_size() == as::Bytes{thread_count * allocation_size};

    finish.arrive_and_wait();
    for (auto& thread : threads)
        thread.join();
    return result;
}
} // namespace

int main() {
    auto& stats = as::Statistics::instance();

    stats.reset();
    AS_CHECK(stats.new_calls() == 0U);
    AS_CHECK(stats.delete_calls() == 0U);
    AS_CHECK(stats.allocated_size() == as::Bytes{});
    AS_CHECK(stats.deallocated_size() == as::Bytes{});
    AS_CHECK(stats.peak_size() == as::Bytes{});

    std::size_t completed_header{};
    stats.new_call(as::Bytes{64}, &completed_header);
    AS_CHECK(stats.new_calls() == 1U);
    AS_CHECK(stats.allocated_size() == as::Bytes{64});
    AS_CHECK(stats.peak_size() == as::Bytes{64});
    stats.delete_call(&completed_header);
    AS_CHECK(stats.delete_calls() == 1U);
    AS_CHECK(stats.deallocated_size() == as::Bytes{64});
    AS_CHECK(stats.allocated_size() - stats.deallocated_size() == as::Bytes{});

    // Valid reset: the completed allocation is no longer live and no other thread is active.
    stats.reset();
    AS_CHECK(stats.new_calls() == 0U);
    AS_CHECK(stats.delete_calls() == 0U);
    AS_CHECK(stats.allocated_size() == as::Bytes{});
    AS_CHECK(stats.deallocated_size() == as::Bytes{});
    AS_CHECK(stats.peak_size() == as::Bytes{});

    const auto result = run_concurrent_test();
    AS_CHECK(result.allocations_ok);
    AS_CHECK(result.deallocations_ok);
    return EXIT_SUCCESS;
}
