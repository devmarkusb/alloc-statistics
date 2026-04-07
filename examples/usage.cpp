#include "mb/alloc-statistics/alloc-statistics.hpp"

#include <iostream>

int main() {
    std::cout << mb::alloc_statistics::sum(2, 1) << '\n';
    return 0;
}
