file(GLOB_RECURSE sources CONFIGURE_DEPENDS "*.cpp")
list(FILTER sources EXCLUDE REGEX "\\.test\\.cpp$")

target_sources(mb.alloc-statistics PRIVATE ${sources})

if(NOT MB_DEVENV_SANITIZER STREQUAL "TSan")
    include(mb-devenv-compile-check)
    mb_devenv_add_compile_check(
        mb.alloc-statistics.compile-check
        PRIVATE mb::alloc-statistics
        HEADER "mb/alloc-statistics/alloc-statistics.hpp"
    )
endif()

if(NOT MB_ALLOC_STATISTICS_BUILD_TESTS)
    return()
endif()

# Bookkeeping-only test: deliberately does not link the allocation interposition object.
# In particular, this gives TSan a target without duplicate global new/delete definitions.
add_executable(
    mb.alloc-statistics-test_statistics
    statistics.test.cpp
    statistics.cpp
)
target_include_directories(
    mb.alloc-statistics-test_statistics
    PRIVATE "${PROJECT_SOURCE_DIR}/include"
)
target_compile_features(mb.alloc-statistics-test_statistics PRIVATE cxx_std_20)
mb_devenv_set_target_essentials(mb.alloc-statistics-test_statistics)
mb_devenv_add_simple_test(mb.alloc-statistics-test_statistics)

if(MB_DEVENV_SANITIZER STREQUAL "TSan")
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
endif()

# Plain-main test: allocation failure and aligned-allocation edge cases. Run with sanitizers
# enabled as well; leak detection remains active for this target.
add_executable(
    mb.alloc-statistics-test_newstats_edge_cases
    new_statistics.edge_cases.test.cpp
)
target_link_libraries(
    mb.alloc-statistics-test_newstats_edge_cases
    PRIVATE mb::alloc-statistics
)
mb_devenv_set_target_essentials(mb.alloc-statistics-test_newstats_edge_cases)
mb_devenv_add_simple_test(mb.alloc-statistics-test_newstats_edge_cases)

# Plain-main test: applied stats with a positive allocation canary. Run with sanitizers too.
add_executable(mb.alloc-statistics-test_applied new_statistics.applied.test.cpp)
target_link_libraries(
    mb.alloc-statistics-test_applied
    PRIVATE mb::alloc-statistics
)
mb_devenv_set_target_essentials(mb.alloc-statistics-test_applied)
mb_devenv_add_simple_test(mb.alloc-statistics-test_applied)

# GTest allocation canary.
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
