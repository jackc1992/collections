#ifndef ABSTRACT_ALLOC_H
#define ABSTRACT_ALLOC_H
#include <memory>

namespace jc::memory
{

/**
 * @brief For internal use to abstract over a std::allocator or a pmr::memory_resource
 */
template <typename Allocator, typename T>
class abstract_allocator
{
public:
    explicit abstract_allocator(Allocator *underlying) noexcept : underlying_allocator_(underlying)
    {
    }

    [[nodiscard]] T *allocate(size_t n)
    {
        if constexpr (is_memory_resource())
        {
            constexpr size_t bytes = n * sizeof(T);
            constexpr size_t alignment = alignof(T);
            return static_cast<T*>(underlying_allocator_.allocate(bytes, alignment));
        }
        else // todo: investigate concept that identifies type of allocator
        {
            using traits = std::allocator_traits<Allocator>;
            return traits::allocate(underlying_allocator_, n);
        }
    }

    void deallocate(T *ptr, size_t n)
    {
        if constexpr (is_memory_resource())
        {
            constexpr size_t length = n * sizeof(T);
            constexpr size_t alignment = alignof(T);
            underlying_allocator_.deallocate(static_cast<void *>(ptr), length, alignment);
        } else // todo: investigate concept that identifies type of allocator
        {
            std::allocator_traits<Allocator>::deallocate(underlying_allocator_, ptr, n);
        }
    }

private:
    Allocator &underlying_allocator_;
    static constexpr bool is_memory_resource()
    {
        return std::is_base_of_v<std::pmr::memory_resource, std::remove_cvref_t<Allocator>>;
    }
};

} // namespace jc::memory

#endif
