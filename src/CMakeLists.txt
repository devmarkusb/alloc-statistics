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
    add_executable(mb.alloc-statistics-test src/new_statistics.test.cpp)
    ul_add_simple_test(mb.alloc-statistics-test)
    set_tests_properties(
        mb.alloc-statistics-test
        PROPERTIES ENVIRONMENT "LSAN_OPTIONS=detect_leaks=0"
    )
    target_link_libraries(mb.alloc-statistics-test PRIVATE mb.alloc-statistics)
    ul_set_target_defaults(mb.alloc-statistics-test)

    add_executable(
        mb.alloc-statistics-test_applied
        src/new_statistics.applied.test.cpp
    )
    ul_add_simple_test(mb.alloc-statistics-test_applied)
    set_tests_properties(
        mb.alloc-statistics-test_applied
        PROPERTIES ENVIRONMENT "LSAN_OPTIONS=detect_leaks=0"
    )
    target_link_libraries(mb.alloc-statistics-test_applied PRIVATE mb.alloc-statistics)
    ul_set_target_defaults(mb.alloc-statistics-test_applied)

    add_executable(
        mb.alloc-statistics-test_gtest
        src/new_statistics.applied.gtest.test.cpp
    )
    ul_add_test(mb.alloc-statistics-test_gtest)
    target_link_libraries(mb.alloc-statistics-test_gtest PRIVATE GTest::gtest GTest::gtest_main mb.alloc-statistics)
    ul_set_target_defaults(mb.alloc-statistics-test_gtest)
endif()
