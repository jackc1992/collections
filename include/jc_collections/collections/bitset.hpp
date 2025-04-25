#ifndef JC_BITSET_H
#define JC_BITSET_H
#include <bit>
#include <jc_collections/util.h>

namespace jc::collections
{

template <size_t capacity>
class scalar_bitset
{

public:
    i64 get_and_set() noexcept
    {
        i64 idx = 0;
        // get the byte to check
        for (; idx < arr_len(); idx++)
        {
            if (bits[idx] < max_val)
            {
                break;
            }
        }
        // return -1 if there's no free bytes
        if (idx >= arr_len()) [[unlikely]]
        {
            return -1;
        }

        // index of the bit to be set
        const i64 i = std::countr_one(bits[idx]);

        // toggle the bit
        bits[idx] |= (1 << i);

        return (64 * idx) + i;
    }

    void unset_bit(const size_t pos) noexcept
    {
        if (pos >= capacity)
        {
            return;
        }
        const size_t arr_index = pos >> 6;
        const size_t remainder = pos & 63;
        const size_t mask      = 1 << remainder;

        bits[arr_index] &= ~mask;
    }

    void set_bit(const size_t pos) noexcept
    {
        if (pos >= capacity)
        {
            return;
        }

        const size_t arr_index = pos >> 6;
        const size_t remainder = pos & 63;
        const size_t mask      = 1 << remainder;

        bits[arr_index] |= mask;
    }

private:
    static constexpr u64 max_val = static_cast<u64>(-1);
    static constexpr size_t arr_len()
    {
        return int_ceil(capacity, 64);
    }

    u64 bits[arr_len()] {};
};
} // namespace jc::collections

#endif
