#ifndef JC_SPSC_H
#define JC_SPSC_H

#include <array>
#include <atomic>
#include <cstddef>
#include <bit>
#include <optional>
#include <type_traits>

using std::size_t;

namespace SPSC {

    template<size_t sz>
    concept is_power_of_two = std::has_single_bit(sz);

    template<typename T, size_t sz = 512> requires
    is_power_of_two<sz> &&
    std::is_move_constructible_v<T> &&
    std::is_move_assignable_v<T> &&
    std::is_trivially_destructible_v<T>
    class spsc
    {
        private:
        alignas(64)std::atomic<size_t> writer_ = 0;
        alignas(64)std::atomic<size_t> reader_ = 0;
        alignas(64)std::array<T, sz> items;

        public:
        spsc() = default;

        spsc(const spsc&) = delete;
        spsc& operator=(const spsc&) = delete;
        spsc(spsc&&) = delete;
        spsc& operator=(spsc&&) = delete;

        ~spsc() = default;

        bool put(T&& element)
        {
            size_t idx = writer_.load(std::memory_order::relaxed);
            if (idx - reader_.load(std::memory_order::acquire) == sz)
            {
                return false;
            }
            items[idx & (sz - 1)] = std::move(element);
            writer_.store(idx + 1, std::memory_order::release);
            return true;
        }

        std::optional<T> read()
        {
            size_t idx = reader_.load(std::memory_order::relaxed);
            if (idx == writer_.load(std::memory_order::acquire))
            {
                return {};
            }
            T element = std::move(items[idx & (sz - 1)]);
            reader_.store(idx + 1, std::memory_order::release);
            return std::make_optional(element);
        }
    };
}
#endif
