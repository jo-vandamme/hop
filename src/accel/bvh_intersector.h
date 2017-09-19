#pragma once

#include "accel/bvh_node.h"
#include "geometry/ray.h"
#include "geometry/hit_info.h"

namespace hop { namespace bvh {

template <typename Visitor>
bool intersect(const Node* nodes, const Ray& ray, HitInfo* hit, Visitor& visitor)
{
    constexpr uint32 BVH_MAX_STACK_SIZE = 32;

    uint32 node_stack[BVH_MAX_STACK_SIZE];
    uint32 cur_node_idx = 0;
    const Node* cur_node;

    bool got_hit = false;
    int stack_index = 0;
    int want_left, want_right;

    Vec3f inv_dir = rcp(ray.dir);
    int dir_is_neg[3] = { inv_dir.x < 0, inv_dir.y < 0, inv_dir.z < 0 };

    while (stack_index > -1)
    {
        cur_node = &nodes[cur_node_idx];

        if (cur_node->is_leaf())
        {
            got_hit = visitor.visit(*cur_node, ray, hit);

            want_left = want_right = 0;
        }
        else
        {
            // TODO test both boxes at the same time
            const BBoxf& left_bbox = cur_node->get_left_bbox();
            const BBoxf& right_bbox = cur_node->get_right_bbox();

            Vec3f tmin = (left_bbox.pmin - ray.org) * inv_dir;
            Vec3f tmax = (left_bbox.pmax - ray.org) * inv_dir;
            Vec3f rmin = min(tmin, tmax);
            Vec3f rmax = max(tmin, tmax);
            float min_max = min(rmax);
            float max_min = max(rmin);
            float left_hit_dist = (min_max < ray.tmin || max_min > min_max) ?
                (float)pos_inf : (max_min >= ray.tmax ? (float)pos_inf : max_min);

            tmin = (right_bbox.pmin - ray.org) * inv_dir;
            tmax = (right_bbox.pmax - ray.org) * inv_dir;
            rmin = min(tmin, tmax);
            rmax = max(tmin, tmax);
            min_max = min(rmax);
            max_min = max(rmin);
            float right_hit_dist = (min_max < ray.tmin || max_min > min_max) ?
                (float)pos_inf : (max_min >= ray.tmax ? (float)pos_inf : max_min);

            want_left = left_hit_dist < (float)pos_inf ? 1 : 0;
            want_right = right_hit_dist < (float)pos_inf ? 1 : 0;
        }

        if (want_left && want_right)
        {
            if (dir_is_neg[cur_node->get_split_axis()])
            {
                node_stack[stack_index++] = cur_node_idx + 1;
                cur_node_idx = cur_node->get_right_child();
            }
            else
            {
                node_stack[stack_index++] = cur_node->get_right_child();
                cur_node_idx = cur_node_idx + 1;
            }
        }
        else if (want_left || want_right)
        {
            cur_node_idx = want_left ? cur_node_idx + 1 : cur_node->get_right_child();
        }
        else if (--stack_index >= 0)
        {
            cur_node_idx = node_stack[stack_index];
        }
    }
    return got_hit;
}

} } // namespace hop::bvh
