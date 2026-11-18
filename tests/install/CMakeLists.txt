cmake_minimum_required(VERSION 3.30)
project(mb.alloc-statistics.install-runtime-test LANGUAGES CXX)

find_package(mb.alloc-statistics CONFIG REQUIRED)

enable_testing()
add_executable(mb.alloc-statistics-install-runtime-test main.cpp)
target_link_libraries(
    mb.alloc-statistics-install-runtime-test
    PRIVATE mb::alloc-statistics
)
add_test(
    NAME installed-allocation-canary
    COMMAND mb.alloc-statistics-install-runtime-test
)
