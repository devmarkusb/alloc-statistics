# CMake project `mb.alloc-statistics` — options `MB_ALLOC_STATISTICS_*`, C++ `mb::alloc_statistics`, headers under `include/mb/alloc-statistics/`.

cmake_minimum_required(VERSION 3.30...4.3)

project(
    mb.alloc-statistics # CMake Project Name, which is also the name of the top-level
    # targets (e.g., library, executable, etc.).
    DESCRIPTION "alloc-statistics library"
    LANGUAGES CXX
    VERSION 1.0.0
)

option(
    MB_ALLOC_STATISTICS_HEADER_ONLY
    "Build as a header-only (INTERFACE) library. When OFF, builds a static library with compiled sources under src/. Values: { ON, OFF }."
    OFF
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

if(MB_ALLOC_STATISTICS_HEADER_ONLY)
    add_library(mb.alloc-statistics INTERFACE)
else()
    add_library(mb.alloc-statistics STATIC)
endif()

add_library(mb::alloc-statistics ALIAS mb.alloc-statistics)

mb_devenv_set_target_defaults(mb.alloc-statistics)

if(MB_ALLOC_STATISTICS_HEADER_ONLY)
    target_compile_definitions(
        mb.alloc-statistics
        INTERFACE MB_ALLOC_STATISTICS_HEADER_ONLY=1
    )
    target_sources(
        mb.alloc-statistics
        INTERFACE
            FILE_SET HEADERS BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include"
    )
    set(_mb_alloc_statistics_fs_scope INTERFACE)
else()
    target_compile_definitions(
        mb.alloc-statistics
        PUBLIC MB_ALLOC_STATISTICS_HEADER_ONLY=0
    )
    target_sources(
        mb.alloc-statistics
        PUBLIC FILE_SET HEADERS BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include"
    )
    set(_mb_alloc_statistics_fs_scope PUBLIC)
endif()

add_subdirectory(include/mb/alloc-statistics)

add_subdirectory(src)

mb_devenv_install_library(mb.alloc-statistics)

if(MB_ALLOC_STATISTICS_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
