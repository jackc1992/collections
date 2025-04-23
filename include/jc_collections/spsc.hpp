#ifndef JC_SPSC_H
#define JC_SPSC_H

#include <array>
#include <atomic>
#include <bit>
#include <cstddef>
#include <cstring>
#include <new>
#include <optional>
#include <type_traits>

namespace JC_SPSC
{

template <std::size_t sz>
concept is_power_of_two = std::has_single_bit(sz);

/*
 * This class is provided with the expectation that memory is managed externally. It is intended to be used
 * in a low latency environment with arena allocation or some other form of allocation. As a consequence,
 * items are copied to the internal buffer, and cannot be complex types with pointers to external data.
 */
template <typename T, std::size_t sz = 512>
    requires is_power_of_two<sz> && std::is_move_constructible_v<T> && std::is_move_assignable_v<T> &&
             std::is_trivially_destructible_v<T>
class simple_spsc
{
private:
    static constexpr std::size_t alignment_size  = std::max(sizeof(T), std::hardware_destructive_interference_size);
    alignas(64) std::atomic<std::size_t> writer_ = 0;
    alignas(64) std::atomic<std::size_t> reader_ = 0;
    alignas(alignment_size) std::array<T, sz> items_;

public:
    simple_spsc() = default;

    simple_spsc(const simple_spsc &)            = delete;
    simple_spsc &operator=(const simple_spsc &) = delete;
    simple_spsc(simple_spsc &&)                 = delete;
    simple_spsc &operator=(simple_spsc &&)      = delete;

    ~simple_spsc() = default;

    bool try_put(T &&element)
    {
        std::size_t idx = writer_.load(std::memory_order::relaxed);
        if (idx - reader_.load(std::memory_order::acquire) == sz)
        {
            return false;
        }
        items_[idx & (sz - 1)] = std::move(element);
        writer_.store(idx + 1, std::memory_order::release);
        return true;
    }

    void put(T &&element)
    {
        std::size_t idx = writer_.load(std::memory_order::relaxed);
        while (idx - reader_.load(std::memory_order::acquire) == sz)
        {
        }
        items_[idx & (sz - 1)] = std::move(element);
        writer_.store(idx + 1, std::memory_order::release);
    }

    std::optional<T> try_read()
    {
        std::size_t idx = reader_.load(std::memory_order::relaxed);
        if (idx == writer_.load(std::memory_order::acquire))
        {
            return {};
        }
        T element = std::move(items_[idx & (sz - 1)]);
        reader_.store(idx + 1, std::memory_order::release);
        return element;
    }

    T read()
    {
        std::size_t idx = reader_.load(std::memory_order::relaxed);
        while (idx == writer_.load(std::memory_order::acquire))
        {
        }
        T element = std::move(items_[idx & (sz - 1)]);
        reader_.store(idx + 1, std::memory_order::release);
        return element;
    }
};

/*
 * This class is provided with the expectation that memory is managed externally. It is intended to be used
 * in a low latency environment with arena allocation or some other form of allocation. As a consequence,
 * items are copied to the internal buffer, and cannot be complex types with pointers to external data.
 */
template <typename T, std::size_t sz = 512>
    requires is_power_of_two<sz> && std::is_move_constructible_v<T> && std::is_move_assignable_v<T> &&
             std::is_trivially_destructible_v<T>
class cached_spsc
{
private:
    static constexpr std::size_t mask = sz - 1;
    struct aligned_indexes
    {
        std::atomic<std::size_t> idx_ = 0;
        std::size_t cached_idx_       = 0;
    };

    static constexpr std::size_t padding_size = std::hardware_destructive_interference_size - sizeof(aligned_indexes);

    alignas(std::hardware_destructive_interference_size) aligned_indexes writer_;
    alignas(std::hardware_destructive_interference_size) aligned_indexes reader_;
    uint8_t padding[padding_size]{};
    std::array<T, sz> items_;

public:
    cached_spsc() = default;
    bool try_put(T &&element)
    {
        std::size_t idx = writer_.idx_.load(std::memory_order::relaxed);
        if (idx - writer_.cached_idx_ == sz)
        {
            writer_.cached_idx_ = reader_.idx_.load(std::memory_order_acquire);
        }

        if (idx - writer_.cached_idx_ == sz)
        {
            return false;
        }

        items_[idx & mask] = std::move(element);
        writer_.idx_.store(idx + 1, std::memory_order::release);

        return true;
    }

    /*
     * spin here and busy wait to remove latency. In order to reduce the cost of calling the function, this option is
     * provided as in testing it's noticeably faster than calling a while loop with try_put until the element is
     * inserted.
     */
    void put(T &&element)
    {
        std::size_t idx = writer_.idx_.load(std::memory_order::relaxed);
        while (idx - writer_.cached_idx_ == sz)
        {
            writer_.cached_idx_ = reader_.idx_.load(std::memory_order_acquire);
        }

        std::memcpy(&items_[idx & mask], &element, sizeof(T));
        writer_.idx_.store(idx + 1, std::memory_order::release);
    }

    void put(T &element)
    {
        std::size_t idx = writer_.idx_.load(std::memory_order::relaxed);
        while (idx - writer_.cached_idx_ == sz)
        {
            writer_.cached_idx_ = reader_.idx_.load(std::memory_order_acquire);
        }

        std::memcpy(&items_[idx & mask], &element, sizeof(T));
        writer_.idx_.store(idx + 1, std::memory_order::release);
    }

    std::size_t get_write(T **ptr)
    {
        std::size_t idx = writer_.idx_.load(std::memory_order::relaxed);
        while (idx - writer_.cached_idx_ == sz)
        {
            writer_.cached_idx_ = reader_.idx_.load(std::memory_order_acquire);
        }
        ptr[0] = &items_[idx & mask];
        return idx;
    }

    void commit(std::size_t idx)
    {
        writer_.idx_.store(idx + 1, std::memory_order::release);
    }

    std::optional<T> try_read()
    {
        std::size_t idx = reader_.idx_.load(std::memory_order::relaxed);
        if (idx == reader_.cached_idx_)
        {
            reader_.cached_idx_ = writer_.idx_.load(std::memory_order_acquire);
        }

        if (idx == reader_.cached_idx_)
        {
            return {};
        }

        T element = std::move(items_[idx & mask]);

        reader_.idx_.store(idx + 1, std::memory_order::release);
        return element;
    }

    /*
     * spin here and busy wait to remove latency. In order to reduce the cost of calling the function, this option is
     * provided as in testing it's noticeably faster than calling a while loop externally.
     */
    T read()
    {
        std::size_t idx = reader_.idx_.load(std::memory_order::relaxed);
        while (idx == reader_.cached_idx_)
        {
            reader_.cached_idx_ = writer_.idx_.load(std::memory_order_acquire);
        }

        T result;
        __builtin_memcpy(&result, &items_[idx & mask], sizeof(T));
        reader_.idx_.store(idx + 1, std::memory_order::release);
        return result;
    }
};
} // namespace JC_SPSC
#endif
