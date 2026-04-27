#ifndef CPP_LIB_TEMPLATE_HPP_BGYBGIYUXGAFYUGAYUF7564
#define CPP_LIB_TEMPLATE_HPP_BGYBGIYUXGAFYUGAYUF7564

#include <cstdint>

namespace mb::alloc_statistics {
/// Sum of \p a and \p b. Overflow follows signed 32-bit arithmetic rules (undefined if out of range).
int32_t sum(int32_t a, int32_t b);
} // namespace mb::alloc_statistics

#endif
