#include "mb/alloc-statistics/alloc-statistics.hpp"

#include "gtest/gtest.h"

namespace mb::alloc_statistics {
TEST(SumTest, example) {
    EXPECT_EQ(sum(40, 2), 42);
}

TEST(SumTest, neg) {
    EXPECT_EQ(sum(40, -42), -2);
}

TEST(SumTest, zero) {
    EXPECT_EQ(sum(0, 5), 5);
}
} // namespace mb::alloc_statistics
