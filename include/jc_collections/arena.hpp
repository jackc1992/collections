#ifndef JC_ARENA_H
#define JC_ARENA_H
#include <jc_collections/util.h>
#include <memory>
#include <sys/mman.h>

/*
 * This is intended to be used to bulk allocate a variety of types. It is intentionally NOT thread safe, please
 * only use it in a single threaded scenario.
 */
class jc_arena : public std::pmr::memory_resource
{
public:
    explicit jc_arena(const size_t capacity) noexcept
    {
        bytes_ = mmap(nullptr, capacity, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        base_  = bytes_;
        len_   = capacity;

        if (bytes_ == MAP_FAILED)
        {
            base_  = nullptr;
            bytes_ = nullptr;
            len_   = 0;
        }
    }

    ~jc_arena() noexcept override
    {
        if (base_ != nullptr)
        {
            munmap(base_, len_);
        }
    }

    bool successful_init() const noexcept
    {
        return base_ != nullptr;
    }

private:
    void *bytes_;
    void *base_;
    std::size_t len_;

    void *do_allocate(const std::size_t bytes, const std::size_t alignment) noexcept override
    {
        if (len_ == 0)
        {
            return nullptr;
        }
        const std::size_t bytes_used = static_cast<u8 *>(bytes_) - static_cast<u8 *>(base_);
        std::size_t remaining        = len_ > bytes_used ? len_ - bytes_used : 0;
        const auto ptr               = std::align(alignment, bytes, bytes_, remaining);
        if (ptr)
        {
            bytes_ = static_cast<u8 *>(bytes_) + bytes;
        }
        return ptr;
    }

    void do_deallocate(void *p, std::size_t bytes, std::size_t alignment) noexcept override
    {
        // no op.
        // todo: consider popping off the top if this is the last thing allocated
    }

    bool do_is_equal(const memory_resource &other) const noexcept override
    {
        return this == &other;
    }
};
#endif
