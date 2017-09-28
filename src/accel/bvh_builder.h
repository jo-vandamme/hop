#pragma once

#include "hop.h"
#include "types.h"
#include "accel/bvh_node.h"
#include "math/math.h"
#include "math/vec3.h"
#include "math/bbox.h"

#include <vector>
#include <functional>
#include <cassert>

namespace hop { namespace bvh {

template <typename Object, typename Accessor, typename ScoringStrategy>
class Builder
{
public:
    typedef std::function<void(Node*, const std::vector<Object>&)> LeafCreationCallback;

    static std::vector<Node> build(Accessor* accessor, const std::vector<Object>& items, uint32 min_leaf_size, LeafCreationCallback callback);

private:
    Builder()
        : m_callback(nullptr), m_min_leaf_size(0), m_num_partitioned_items(0), m_num_total_items(0)
        , m_num_nodes(0), m_num_leaves(0), m_max_depth(0)
    {
    }

    uint32 partition(const std::vector<Object>& items, int depth, BBoxr& node_bbox);
    uint32 create_leaf(Node* node, const std::vector<Object>& items);

private:

    struct SplitScore
    {
        uint8 axis;
        Real split_point;
        uint32 left_count, right_count;
        Real score;
    };

    static constexpr Real min_side_length = 1e-3;
    static constexpr Real min_split_step = 1e-5;

    std::vector<Node> m_nodes;

    LeafCreationCallback m_callback;
    Accessor* m_accessor;

    uint32 m_min_leaf_size;
    uint32 m_num_partitioned_items;
    uint32 m_num_total_items;
    uint32 m_num_nodes;
    uint32 m_num_leaves;
    uint32 m_max_depth;
};

template <typename Object, typename Accessor, typename ScoringStrategy>
std::vector<Node> Builder<Object, Accessor, ScoringStrategy>::build(Accessor* accessor,
        const std::vector<Object>& items, uint32 min_leaf_size, LeafCreationCallback callback)
{
    Builder builder;
    builder.m_accessor = accessor;
    builder.m_callback = callback;
    builder.m_min_leaf_size = min_leaf_size;
    builder.m_num_total_items = items.size();

    BBoxr bbox;
    builder.partition(items, 0, bbox);

    return std::move(builder.m_nodes);
}

template <typename Object, typename Accessor, typename ScoringStrategy>
uint32 Builder<Object, Accessor, ScoringStrategy>::partition(
        const std::vector<Object>& items, int depth, BBoxr& node_bbox)
{
    if (depth > (int)m_max_depth)
        m_max_depth = depth;

    Node node;

    // Calculate BBoxf for the node
    node_bbox = BBoxr();
    for (auto& item : items)
        node_bbox.merge(m_accessor->get_bbox(item));

    if (items.size() <= m_min_leaf_size)
        return create_leaf(&node, items);

    // Get current node score
    Real best_score = ScoringStrategy::score_partition(m_accessor, items);

    constexpr size_t num_buckets = NUM_SAH_SPLITS;
    static SplitScore score_list[3 * num_buckets];
    size_t num_scores = 0;

    const Vec3r side = node_bbox.pmax - node_bbox.pmin;

    for (uint8 axis = 0; axis < 3; ++axis)
    {
        // Skip axis if bbox dimension is too small
        if (side[axis] < min_side_length)
            continue;

        const Real split_step = side[axis] / Real(num_buckets);
        if (split_step < min_split_step)
            continue;

        // Compute split scores in parallel
#pragma omp parallel for
        for (size_t i = 0; i < num_buckets; ++i)
        {
            Real split_point = node_bbox.pmin[axis] + i * split_step;
            SplitScore score;
            score.axis = axis;
            score.split_point = split_point;
            score.score = ScoringStrategy::score_split(m_accessor, items, axis, split_point, &score.left_count, &score.right_count);
            score_list[num_scores + i] = score;
        }
        num_scores += num_buckets;
    }

    // Process all scores and pick the best split
    SplitScore* best_split = nullptr;
    for (size_t i = 0; i < num_scores; ++i)
    {
        SplitScore* score = &score_list[i];
        if (score->score < best_score)
        {
            best_score = score->score;
            best_split = score;
        }
    }

    // If we can't find a split that improves the current node score create a leaf
#ifdef TRIS_SIMD_ISECT
    if (best_split == nullptr || best_split->left_count < m_min_leaf_size || best_split->right_count < m_min_leaf_size)
#else
    if (best_split == nullptr)
#endif
        return create_leaf(&node, items);

    // Split items list into two sets
    std::vector<Object> left_items, right_items;
    left_items.reserve(best_split->left_count);
    right_items.reserve(best_split->right_count);

    for (auto& item : items)
    {
        Vec3r center = m_accessor->get_centroid(item);
        if (center[best_split->axis] < best_split->split_point)
            left_items.push_back(item);
        else
            right_items.push_back(item);
    }

    // Add node to list
    uint32 node_index = m_nodes.size();
    m_nodes.push_back(node);
    ++m_num_nodes;

    // Partition children and update node indices
    BBoxr left_bbox, right_bbox;
    uint32 left_node_index = partition(left_items, depth + 1, left_bbox);
    uint32 right_node_index = partition(right_items, depth + 1, right_bbox);

    assert(left_node_index == node_index + 1);

    m_nodes[node_index].set_right_child(right_node_index);
    m_nodes[node_index].set_left_bbox(left_bbox);
    m_nodes[node_index].set_right_bbox(right_bbox);
    m_nodes[node_index].set_split_axis(best_split->axis);

    return node_index;
}

template <typename Object, typename Accessor, typename ScoringStrategy>
uint32 Builder<Object, Accessor, ScoringStrategy>::create_leaf(Node* node, const std::vector<Object>& items)
{
    m_callback(node, items);

    // Make sure this is a leaf if the callback ignored it
    if (node->get_type() == 0)
        node->set_type(1);

    // Append node to list
    uint32 node_index = m_nodes.size();
    m_nodes.push_back(*node);

    // Update stats
    ++m_num_leaves;
    m_num_partitioned_items += items.size();

    return node_index;
}

template <typename Object, typename Accessor>
class SAHStrategy
{
public:

    // Score a BVH split based on the surface area heuristic. The SAH calculates
    // the split score using the formulat:
    // left count * left BBoxf area + right count * right BBoxf area.
    // SAH avoids splits that generate empty partitions by assigning the worst
    // possible score when it encounters such cases.
    static Real score_split(Accessor* accessor, const std::vector<Object>& items, uint8 axis, Real split_point, uint32* left_count, uint32* right_count)
    {
        BBoxr left_bbox, right_bbox;

        *left_count = 0;
        *right_count = 0;

        for (auto& object : items)
        {
            const Vec3r center = accessor->get_centroid(object);
            const BBoxr& bbox = accessor->get_bbox(object);

            if (center[axis] < split_point)
            {
                ++*left_count;
                left_bbox.merge(bbox);
            }
            else
            {
                ++*right_count;
                right_bbox.merge(bbox);
            }
        }

        if (*left_count == 0 || *right_count == 0)
            return pos_inf;

        const Vec3r left_side = left_bbox.pmax - left_bbox.pmin;
        const Vec3r right_side = right_bbox.pmax - right_bbox.pmin;

        Real score = (Real)*left_count * (left_side.x * left_side.y + left_side.x * left_side.z + left_side.y * left_side.z) +
                     (Real)*right_count * (right_side.x * right_side.y + right_side.x * right_side.z + right_side.y * right_side.z);

        score *= BVH_TRAV_COST;

        return score;
    }

    // Calculate score for a partitioned list using formula:
    // count * BBoxf area
    // If the list is empty, then this method returns the worst possible score.
    static Real score_partition(Accessor* accessor, const std::vector<Object>& items)
    {
        if (items.empty())
            return pos_inf;

        BBoxr bbox;
        for (auto& object : items)
            bbox.merge(accessor->get_bbox(object));

        const Vec3r side = bbox.pmax - bbox.pmin;

        return (Real)items.size() * (side.x * side.y + side.x * side.z + side.y * side.z);
    }
};

} } // namespace hop::bvh
