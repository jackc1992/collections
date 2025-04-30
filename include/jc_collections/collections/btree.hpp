#ifndef JC_BTREE_H
#define JC_BTREE_H
#include <bits/memory_resource.h>
#include <jc_collections/util.h>
#include <optional>

namespace jc::collections
{
/**
 *@brief traditional btree
 */
template <typename Key, typename Val, typename Allocator, size_t node_count>
class b_tree
{
    struct pair
    {
        Key key;
        Val val;
    };
    struct Node
    {
        u32 key_count = 0;
        u32 child_count = 0;
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

    bool emplace(Key &&key, Val &&val)
    {
        return insert_node(key, val, root_);
    }

private:
    std::optional<pair> insert_node(Key &&key, Val &&val, Node *node)
    {
        if (node->key_count == node_count)
        {

        }

        return {};
    }

    Node *root_;
    [[no_unique_address]] Allocator allocator_;
};
} // namespace jc::collections

#endif
