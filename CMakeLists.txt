# CMake project `mb.alloc-statistics` — options `MB_ALLOC_STATISTICS_*`, C++ `mb::alloc_statistics`, headers under `include/mb/alloc-statistics/`.

cmake_minimum_required(VERSION 3.30...4.3)

project(
    mb.alloc-statistics # CMake Project Name, which is also the name of the top-level
    # targets (e.g., library, executable, etc.).
    DESCRIPTION "alloc-statistics library"
    LANGUAGES CXX
    VERSION 1.0.0
)

# [CMAKE.SKIP_TESTS]
option(
    MB_ALLOC_STATISTICS_BUILD_TESTS
    "Enable building tests and test infrastructure. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

# [CMAKE.SKIP_EXAMPLES]
option(
    MB_ALLOC_STATISTICS_BUILD_EXAMPLES
    "Enable building examples. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

add_subdirectory(devenv)
list(PREPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/devenv/cmake")

if(MB_ALLOC_STATISTICS_BUILD_TESTS)
    include(mb-devenv-ctest-root)
endif()
include(mb-devenv-defaults)
include(mb-devenv-install-library-config)

# From fetched https://github.com/devmarkusb/pre-commit (see fetchcontent-lockfile.json).
mb_pre_commit_setup()

add_library(mb.alloc-statistics STATIC)

add_library(mb::alloc-statistics ALIAS mb.alloc-statistics)

#target_compile_features(mb.alloc-statistics PUBLIC cxx_std_20)

mb_devenv_set_target_defaults(mb.alloc-statistics)

add_subdirectory(include/mb/alloc-statistics)

add_subdirectory(src)

mb_devenv_install_library(mb.alloc-statistics)

if(MB_ALLOC_STATISTICS_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
