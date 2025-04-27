#ifndef JC_POOL_ALLOC_H
#define JC_POOL_ALLOC_H

#include <cassert>
#include <jc_collections/collections/bitset.hpp>
#include <jc_collections/memory/abstract_allocator.hpp>

namespace jc::memory
{

template <typename T, size_t amount, typename Allocator>
class cached_pool_allocator
{
    union next_or_t {
        size_t next_;
        T item_;
    };

public:
    using pointer            = T *;
    using const_pointer      = const T *;
    using void_pointer       = void *;
    using const_void_pointer = const void *;
    using size_type          = size_t;
    using difference_type    = ptrdiff_t;

    explicit cached_pool_allocator(Allocator &allocator) noexcept : allocator_(allocator)
    {
        current_bit_ = static_cast<size_t>(bits_.get_and_set());
    }

    ~cached_pool_allocator()
    {
    }

    T *allocate(const size_t n) noexcept
    // only dealing with one allocation and deallocation atm
    {
        assert(n <= 256, "this allocator only supports allocations up to 256");
        assert(n > 0, "allocation amount must be positive");
        if (n == 256)
        {
            const i64 index = (bits_.get_and_set());
            if (index < 0) [[unlikely]]
            {
                return nullptr;
            }

            const size_t slab_idx         = static_cast<size_t>(index);
            pool_allocation_count_[index] = n;
            return &items_[slab_idx * 256];
        }
        if (pool_allocation_count_[current_bit_] <= 256 - n)
        {
            const size_t idx                     = (current_bit_ * 256) + pool_allocation_count_[current_bit_];
            pool_allocation_count_[current_bit_] = n;
            return &items_[idx];
        }
        // guaranteed to be first here
        const i64 index = (bits_.get_and_set());
        if (index < 0) [[unlikely]]
        {
            return nullptr;
        }
        current_bit_                         = static_cast<size_t>(index);
        const pointer ptr                    = &items_[current_bit_ * 256];
        pool_allocation_count_[current_bit_] = n;
        return ptr;
    }
    void deallocate(T *p, const size_t n) noexcept
    {
        if (p == nullptr || n == 0)
        {
            return;
        }
        assert(n <= 256, "max allocation/deallocation is 256");
        assert(p >= items_ && (p + n) <= (items_ + amount), "Pointer out of bounds!");

        const ptrdiff_t offset = p - items_;

        const size_t index = static_cast<size_t>(offset) >> 8;

        pool_free_count_[index] += static_cast<u8>(n);
        if (current_bit_ == index && pool_free_count_[index] == pool_allocation_count_[index]) [[unlikely]]
        {
            pool_free_count_[index]       = 0;
            pool_allocation_count_[index] = 0;
        }
        else if (pool_free_count_[index] == pool_allocation_count_[index]) [[unlikely]]
        {
            bits_.unset_bit(index);
            pool_free_count_[index]       = 0;
            pool_allocation_count_[index] = 0;
        }
    }
    static size_t max_size() noexcept
    {
        return 256;
    }

private:
    static constexpr size_t bitset_item_count()
    {
        // each bit is a representation of 256
        return int_ceil(amount, 256);
    }
    size_t current_bit_ = 0;
    // tracks which pools are "in use"
    collections::scalar_bitset<bitset_item_count(bitset_item_count())> bits_;
    // increment this when an item is used from a pool
    u8 pool_allocation_count_[bitset_item_count()] = {0};
    // increment this when an item is returned to a pool (when free count == allocation count, the pool is empty)
    u8 pool_free_count_[bitset_item_count()] = {0};
    T items_[amount];
    abstract_allocator<Allocator, T> allocator_;
};
} // namespace jc::memory

struct Node
{
    int x;
    int y;
    Node *next;
};

#endif
