#ifndef JC_BTREE_H
#define JC_BTREE_H
#include <bits/memory_resource.h>
#include <jc_collections/util.h>

namespace jc::collections
{
/**
 *@brief traditional btree
 */
template <typename Key, typename Val, typename Allocator, size_t node_count>
class b_tree
{
    struct Node
    {
        size_t count = 0;
        Key keys_[node_count]{0};
        Val values_[node_count]{0};
        Node *children_[node_count + 1]{};
    };

public:
    explicit b_tree(Allocator &allocator) : root_(nullptr), allocator_(allocator)
    {
        root_ = std::allocator_traits<Allocator>::allocate(allocator_, 1);
        std::allocator_traits<Allocator>::construct(allocator_, root_);
    }

    template<typename ...Args>
    bool emplace(Args &&... args)
    {

    }

private:
    Node *root_;
    [[no_unique_address]] Allocator allocator_;
};
} // namespace jc::collections

#endif
