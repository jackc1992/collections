#ifndef JC_BTREE_H
#define JC_BTREE_H
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
        size_t count;
        Key keys_[node_count];
        Val values_[node_count];
        Node* children_[node_count];
    };
};
} // namespace jc::collections

#endif
