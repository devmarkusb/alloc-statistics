#include "mb/alloc-statistics/new_statistics.hpp"
#include <exception>
#include <iostream>

namespace as = mb::alloc_statistics;

namespace {
#define AS_ASSERT_TERMINATE(cond) \
    do { \
        if (!(cond)) \
            std::terminate(); \
    } while (false)

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

            std::cout << "new calls: " << new_calls << "\n";
            std::cout << "delete calls: " << delete_calls << "\n";
#if defined(__linux__)
#if !defined(NDEBUG)
            AS_ASSERT_TERMINATE(new_calls - delete_calls == 0);
#endif
#else
            // untested
            AS_ASSERT_TERMINATE(new_calls - delete_calls == 0);
#endif
            std::cout << "allocated size: " << allocated_size << "\n";
            std::cout << "deallocated size: " << deallocated_size << "\n";
            const auto alloc_dealloc_diff = allocated_size - deallocated_size;
            std::cout << "allocated size minus deallocated size: " << alloc_dealloc_diff << "\n";
#if !defined(__APPLE__)
            AS_ASSERT_TERMINATE(alloc_dealloc_diff == as::Bytes{});
#else
            // untested
            AS_ASSERT_TERMINATE(alloc_dealloc_diff == as::Bytes{});
#endif
            std::cout << "peak mem usage: " << peak_size << "\n";
        } catch (...) {
        }
    }
};
} // namespace

int main(int /*unused*/, char** /*unused*/) {
    try {
        // Our definition of 'global'. By the way, a real global data's construction and destruction time is beyond
        // our control, so that would never yield proper new/delete statistics.
        const AGlobalDestructor global;
        return EXIT_SUCCESS;
    } catch (...) {
        return EXIT_FAILURE;
    }
}
