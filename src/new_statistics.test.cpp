#include "mb/alloc-statistics/new_statistics.hpp"
#include <iostream>
#include <memory>
#include <vector>

namespace as = mb::alloc_statistics;

namespace {
#define AS_ASSERT_THROW(cond) \
    do { \
        if (!(cond)) \
            throw std::runtime_error("Assertion failed: " #cond); \
    } while (false)

/** GCC at high optimization can remove `new`/`delete` pairs when no *standard-observable* use of the
    storage remains; our Statistics hooks are not visible to the optimizer, so counts would stay at 0. */
void pin_heap_allocation([[maybe_unused]] void* p) noexcept {
#if defined(__GNUC__)
    asm volatile("" : : "g"(p) : "memory");
#endif
}
} // namespace

int main() {
    try {
        std::cout << "Running main() from " << __FILE__ << "\n";

        const auto& memstats = as::Statistics::instance();

        AS_ASSERT_THROW(memstats.new_calls() == 0);
        AS_ASSERT_THROW(memstats.delete_calls() == 0);
        AS_ASSERT_THROW(memstats.peak_size() == as::Bytes{});

        size_t exp_new_calls{};
        size_t exp_delete_calls{};
        as::Bytes exp_peak_size{};
        constexpr size_t n10{10};
        constexpr size_t b44{44};
#if defined(_MSC_VER) && !defined(NDEBUG)
        constexpr as::Bytes vector_constr_heap_alloc_size{16};
#else
        constexpr as::Bytes vector_constr_heap_alloc_size{};
#endif
        {
            // RAII so we don't leak on assertion failure (e.g. during CMake test discovery)
            std::unique_ptr<int> p1(new int); // NOLINT
            pin_heap_allocation(p1.get());
            AS_ASSERT_THROW(memstats.new_calls() == 1U);
            AS_ASSERT_THROW(memstats.peak_size() == as::Bytes{sizeof(int)});

            auto impl_correction_ofs = memstats.allocated_size();
            {
                std::vector<int> l;
                impl_correction_ofs = memstats.allocated_size() - impl_correction_ofs;
                AS_ASSERT_THROW(impl_correction_ofs == vector_constr_heap_alloc_size);
                AS_ASSERT_THROW(!l.capacity());
                l.reserve(n10);
                exp_new_calls = 2U + (impl_correction_ofs ? 1U : 0U);
                AS_ASSERT_THROW(memstats.new_calls() == exp_new_calls);
                exp_peak_size = as::Bytes{b44} + impl_correction_ofs;
                AS_ASSERT_THROW(memstats.peak_size() == exp_peak_size);
                for (size_t i = 0; i < n10 / 2U; ++i)
                    l.push_back(static_cast<int>(i));
            }
            exp_delete_calls = 1U + (impl_correction_ofs ? 1U : 0U);
            AS_ASSERT_THROW(memstats.delete_calls() == exp_delete_calls);

            p1.reset(); // calls delete, so stats still updated
            ++exp_delete_calls;
            AS_ASSERT_THROW(memstats.delete_calls() == exp_delete_calls);

            AS_ASSERT_THROW(memstats.peak_size() == as::Bytes{b44} + impl_correction_ofs);

            std::unique_ptr<int[]> p2(new int[n10]); // NOLINT
            pin_heap_allocation(p2.get());
            ++exp_new_calls;
            AS_ASSERT_THROW(memstats.new_calls() == exp_new_calls);
            p2.reset(); // calls delete[], stats still updated
            ++exp_delete_calls;
            AS_ASSERT_THROW(memstats.delete_calls() == exp_delete_calls);
        }

        AS_ASSERT_THROW(memstats.new_calls() == exp_new_calls);
        AS_ASSERT_THROW(memstats.delete_calls() == exp_delete_calls);
        AS_ASSERT_THROW(memstats.peak_size() == exp_peak_size);

        std::cout << "PASSED (" << __FILE__ << ")\n";

        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        std::cerr << "FAILED: " << e.what() << "\n";
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "FAILED: unknown exception\n";
        return EXIT_FAILURE;
    }
}
