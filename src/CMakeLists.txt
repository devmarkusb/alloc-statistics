file(GLOB_RECURSE sources CONFIGURE_DEPENDS "*.cpp")
list(FILTER sources EXCLUDE REGEX "\\.test\\.cpp$")

target_sources(mb.alloc-statistics PRIVATE ${sources})

include(mb-devenv-compile-check)
mb_devenv_add_compile_check(
    mb.alloc-statistics.compile-check
    PRIVATE mb::alloc-statistics
    HEADER "mb/alloc-statistics/alloc-statistics.hpp"
)

if(NOT MB_ALLOC_STATISTICS_BUILD_TESTS)
    return()
endif()

include(mb-devenv-googletest)

if(NOT MB_DEVENV_SANITIZER)
    # Plain-main test: core new/delete statistics tracking
    add_executable(mb.alloc-statistics-test_newstats new_statistics.test.cpp)
    target_link_libraries(
        mb.alloc-statistics-test_newstats
        PRIVATE mb::alloc-statistics
    )
    mb_devenv_set_target_essentials(mb.alloc-statistics-test_newstats)
    mb_devenv_add_simple_test(mb.alloc-statistics-test_newstats)
    set_tests_properties(
        mb.alloc-statistics-test_newstats
        PROPERTIES ENVIRONMENT "LSAN_OPTIONS=detect_leaks=0"
    )

    # Plain-main test: applied stats (whole-process allocation accounting)
    add_executable(
        mb.alloc-statistics-test_applied
        new_statistics.applied.test.cpp
    )
    target_link_libraries(
        mb.alloc-statistics-test_applied
        PRIVATE mb::alloc-statistics
    )
    mb_devenv_set_target_essentials(mb.alloc-statistics-test_applied)
    mb_devenv_add_simple_test(mb.alloc-statistics-test_applied)
    set_tests_properties(
        mb.alloc-statistics-test_applied
        PROPERTIES ENVIRONMENT "LSAN_OPTIONS=detect_leaks=0"
    )

    # GTest test with custom main: applied stats measured around GTest's own allocations
    add_executable(
        mb.alloc-statistics-test_gtest
        new_statistics.applied.gtest.test.cpp
    )
    target_link_libraries(
        mb.alloc-statistics-test_gtest
        PRIVATE GTest::gtest mb::alloc-statistics
    )
    mb_devenv_set_target_essentials(mb.alloc-statistics-test_gtest)
    gtest_discover_tests(mb.alloc-statistics-test_gtest)
endif()
