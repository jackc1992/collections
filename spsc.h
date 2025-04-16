#ifndef JC_SPSC_H
#define JC_SPSC_H

#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <bit>

template<std::size_t sz>
concept is_power_of_two = std::has_single_bit(sz);

template<typename T, std::size_t sz = 512>
    requires is_power_of_two<sz>
class spsc
{
private:
    alignas(64)std::atomic<ssize_t> writer_ = 0;
    alignas(64)std::atomic<ssize_t> reader_ = 0;
    alignas(64)std::array<T, sz> items;

public:
    spsc() = default;
    void force_put(T&& element)
    {
        ssize_t idx = writer_.load(std::memory_order::relaxed);
        items[idx & (sz - 1)] = std::move(element);
        writer_.store(idx + 1, std::memory_order::release);
    }

    bool try_put(T&& element)
    {
        ssize_t idx = writer_.load(std::memory_order::relaxed);
        if (idx - reader_.load(std::memory_order::acquire) == sz)
        {
            return false;
        }
        items[idx & (sz - 1)] = std::move(element);
        writer_.store(idx + 1, std::memory_order::release);
        return true;
    }

    bool try_read(T& element)
    {
        ssize_t idx = reader_.load(std::memory_order::relaxed);
        if (idx == writer_.load(std::memory_order::acquire))
        {
            return false;
        }
        element = std::move(items[idx & (sz - 1)]);
        reader_.store(idx + 1, std::memory_order::release);
        return true;
    }

    void force_read(T& element)
    {
        ssize_t idx = reader_.load(std::memory_order::relaxed);
        element = std::move(items[idx & (sz - 1)]);
        reader_.store(idx + 1, std::memory_order::release);
    }

};
#endif
