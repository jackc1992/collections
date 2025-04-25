#ifndef UTIL_H
#define UTIL_H
#include <cstdint>

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

constexpr u8 u8_max = static_cast<u8>(-1);

[[nodiscard]] constexpr size_t int_ceil(const size_t amount, const size_t divisor) noexcept
{
    static_assert(divisor > 0, "int_ceil divisor must be positive");
    static_assert(amount + divisor < static_cast<size_t>(-1), "int_ceil overflow: amount + divisor is too large");

    return (amount + divisor - 1) / divisor;
}
#endif // UTIL_H
