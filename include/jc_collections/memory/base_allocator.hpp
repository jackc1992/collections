#ifndef JC_BASE_ALLOCATOR_H
#define JC_BASE_ALLOCATOR_H

#include <cstddef>
#include <memory>
#include <memory_resource>
#include <sys/mman.h>
namespace jc::memory
{
/**
 * @brief A simple arena allocator using mmap.
 *
 * This allocator reserves a contiguous block of virtual memory using mmap
 * and dispenses it sequentially using bump allocation. Deallocations are no-ops.
 * The entire memory block is released when the allocator is destroyed.
 * It is designed to be used as a std::pmr::memory_resource.
 *
 * Note: This implementation is NOT thread-safe. Access from multiple threads
 * requires external synchronization.
 */
class base_allocator final : public std::pmr::memory_resource
{
public:
    /**
     * @brief Constructs the allocator and reserves memory.
     * @param capacity The total number of bytes to reserve using mmap.
     * It does not throw on unsuccessful initiation, instead call the `successful_init()` function
     */
    explicit base_allocator(const size_t capacity) noexcept : capacity_(capacity)
    {
        current_ptr_ = mmap(nullptr, capacity_, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        base_ptr     = current_ptr_;

        if (current_ptr_ == MAP_FAILED)
        {
            base_ptr     = nullptr;
            current_ptr_ = nullptr;
            capacity_    = 0;
        }
    }
    /**
     * @brief Deallocates all memory associated here and releases mapped memory
     */
    ~base_allocator() noexcept override
    {
        if (base_ptr != nullptr)
        {
            munmap(base_ptr, capacity_);
        }
    }

    // Resource management: one instantiation of this object directly owns memory,
    // as a result copying and moving can lead to double-free or overwriting values
    base_allocator(const base_allocator &)            = delete;
    base_allocator &operator=(const base_allocator &) = delete;
    base_allocator(base_allocator &&)                 = delete;
    base_allocator &operator=(base_allocator &&)      = delete;

    /**
     * @brief Checks if the allocator was successfully initialized (if not throwing).
     * @return true if mmap succeeded, false otherwise.
     */
    bool successful_init() const noexcept
    {
        return base_ptr != nullptr;
    }

    /**
     * @brief Gets the capacity in bytes of the allocator.
     * @return the capacity in bytes.
     */
    std::size_t capacity() const noexcept
    {
        return capacity_;
    }

    /**
     * @brief Gets the number of bytes used by this allocator
     * @return number of used bytes.
     */
    std::size_t used() const noexcept
    {
        return static_cast<std::size_t>(current_ptr_ - base_ptr);
    }

    /**
     * @brief Gets the number of bytes remaining in this allocator
     * @return number of remaining bytes
     */
    std::size_t remaining() const noexcept
    {
        return capacity() - used();
    }

private:
    /**
     * @brief Allocates memory with specified size and alignment.
     *
     * Implements the virtual allocation function from std::pmr::memory_resource.
     * Performs bump allocation with alignment support.
     *
     * @param bytes The number of bytes to allocate.
     * @param alignment The required alignment for the allocation.
     * @return Pointer to the allocated memory, or nullptr if allocation fails.
     */
    void *do_allocate(const std::size_t bytes, const std::size_t alignment) noexcept override
    {
        if (bytes == 0)
        {
            return current_ptr_;
        }
        if (capacity_ == 0)
        {
            return nullptr;
        }
        const std::size_t current_ptr_used =
            static_cast<std::byte *>(current_ptr_) - static_cast<std::byte *>(base_ptr);
        std::size_t remaining = capacity_ > current_ptr_used ? capacity_ - current_ptr_used : 0;
        const auto ptr        = std::align(alignment, bytes, current_ptr_, remaining);
        if (ptr)
        {
            current_ptr_ = static_cast<std::byte *>(current_ptr_) + bytes;
        }
        return ptr;
    }

    /**
     * @brief Deallocates memory (no-op for arena).
     *
     * Implements the virtual deallocation function from std::pmr::memory_resource.
     * Memory is only freed when the entire arena allocator is freed
     *
     * @param p Pointer to the memory to deallocate (ignored).
     * @param bytes Size of the memory block (ignored).
     * @param alignment Alignment of the memory block (ignored).
     */
    void do_deallocate(void *p, std::size_t bytes, std::size_t alignment) noexcept override
    {
        (void)p;
        (void)bytes;
        (void)alignment;
    }

    /**
     * @brief Compares this allocator with another memory_resource.
     *
     * Implements the virtual comparison function from std::pmr::memory_resource.
     * Two base_allocators are considered equal only if they are the same object,
     * as they manage distinct memory regions.
     *
     * @param other The other memory_resource to compare against.
     * @return true if this allocator is the same object as other, false otherwise.
     */
    bool do_is_equal(const memory_resource &other) const noexcept override
    {
        return this == &other;
    }

    void *current_ptr_    = nullptr;
    void *base_ptr        = nullptr;
    std::size_t capacity_ = 0;
};
} // namespace jc::memory

#endif
