if(NOT MB_ALLOC_STATISTICS_HEADER_ONLY)
    file(GLOB_RECURSE sources CONFIGURE_DEPENDS "*.cpp")
    list(FILTER sources EXCLUDE REGEX "\\.test\\.cpp$")
    target_sources(mb.alloc-statistics PRIVATE ${sources})
endif()

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
mb_devenv_add_test(mb.alloc-statistics.test PRIVATE mb::alloc-statistics)
