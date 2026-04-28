#include "mb/alloc-statistics/new_statistics.hpp"
#include "gtest/gtest.h"
#include <iostream>

namespace as = mb::alloc_statistics;

namespace {
struct AGlobalDestructor {
    AGlobalDestructor() {
        as::Statistics::instance().reset();
    }

    ~AGlobalDestructor() {
        try {
            const auto& memstats = as::Statistics::instance();

            const auto new_calls{memstats.new_calls()};
            const auto delete_calls{memstats.delete_calls()};
            const auto allocated_size{memstats.allocated_size()};
            const auto deallocated_size{memstats.deallocated_size()};
            const auto peak_size = memstats.peak_size();

            // GTest leaks memory; hard assertions here are left commented-out until a robust
            // compensation strategy is in place.
            std::cout << "new calls: " << new_calls << "\n";
            std::cout << "delete calls: " << delete_calls << "\n";
            std::cout << "allocated size: " << allocated_size << "\n";
            std::cout << "deallocated size: " << deallocated_size << "\n";
            const auto alloc_dealloc_diff = allocated_size - deallocated_size;
            std::cout << "allocated size minus deallocated size: " << alloc_dealloc_diff << "\n";
            std::cout << "peak mem usage: " << peak_size << "\n";
        } catch (...) {
        }
    }
};
} // namespace

TEST(NewStatisticsApplied, Smoke) {
    SUCCEED();
}

int main(int argc, char** argv) {
    try {
        // Our definition of 'global'. By the way, a real global data's construction and destruction time is beyond
        // our control, so that would never yield proper new/delete statistics.
        const AGlobalDestructor global;
        testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    } catch (...) {
        return EXIT_FAILURE;
    }
}
