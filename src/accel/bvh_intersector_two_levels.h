#pragma once

#include "hop.h"
#include "types.h"
#include "accel/bvh_node.h"
#include "geometry/ray.h"
#include "geometry/hit_info.h"
#include "math/math.h"
#include "math/transform.h"
#include "util/log.h"

#include <limits>
#include <immintrin.h>

namespace hop { namespace bvh {

template <typename Visitor>
bool intersect_two_levels(const Node* nodes, const Transformr* inv_transforms, const uint32* bvh_roots,
                          const Ray& r, HitInfo* hit, Visitor& visitor)
{
    constexpr uint32 BVH_MAX_STACK_SIZE = 32;

    Ray ray(r);

    const Vec3r orig_ray_org = ray.org;
    const Vec3r orig_ray_dir = ray.dir;
    Vec3r inv_dir = rcp(ray.dir);
    Vec3i dir_is_neg = { inv_dir.x < 0, inv_dir.y < 0, inv_dir.z < 0 };

    uint32 node_stack[BVH_MAX_STACK_SIZE];
    uint32 node_idx = 0;
    const bvh::Node* node_ptr;
    bool got_hit = false;
    int stack_index = 0;
    uint32 instance_idx = 0;
    int mesh_bvh_stack_start_index = -1;

#ifdef BBOX_SIMD_ISECT
    // Load the ray into SSE registers.
    __m128d org_x = _mm_set1_pd(ray.org.x);
    __m128d org_y = _mm_set1_pd(ray.org.y);
    __m128d org_z = _mm_set1_pd(ray.org.z);
    __m128d rcp_dir_x = _mm_set1_pd(inv_dir.x);
    __m128d rcp_dir_y = _mm_set1_pd(inv_dir.y);
    __m128d rcp_dir_z = _mm_set1_pd(inv_dir.z);
    __m128d ray_tmin = _mm_set1_pd(ray.tmin);
#endif

    while (stack_index > -1)
    {
        node_ptr = &nodes[node_idx];

        if (likely(node_ptr->is_interior()))
        {
#ifdef BBOX_SIMD_ISECT
            const __m128d xl1 = _mm_mul_pd(rcp_dir_x, _mm_sub_pd(_mm_load_pd(node_ptr->bbox_data + 0 + 2 * (    dir_is_neg.x)), org_x));
            const __m128d xl2 = _mm_mul_pd(rcp_dir_x, _mm_sub_pd(_mm_load_pd(node_ptr->bbox_data + 0 + 2 * (1 - dir_is_neg.x)), org_x));
            const __m128d yl1 = _mm_mul_pd(rcp_dir_y, _mm_sub_pd(_mm_load_pd(node_ptr->bbox_data + 4 + 2 * (    dir_is_neg.y)), org_y));
            const __m128d yl2 = _mm_mul_pd(rcp_dir_y, _mm_sub_pd(_mm_load_pd(node_ptr->bbox_data + 4 + 2 * (1 - dir_is_neg.y)), org_y));
            const __m128d zl1 = _mm_mul_pd(rcp_dir_z, _mm_sub_pd(_mm_load_pd(node_ptr->bbox_data + 8 + 2 * (    dir_is_neg.z)), org_z));
            const __m128d zl2 = _mm_mul_pd(rcp_dir_z, _mm_sub_pd(_mm_load_pd(node_ptr->bbox_data + 8 + 2 * (1 - dir_is_neg.z)), org_z));

            const __m128d ray_tmax = _mm_set1_pd(ray.tmax);
            const __m128d tmin = _mm_max_pd(zl1, _mm_max_pd(yl1, _mm_max_pd(xl1, ray_tmin)));
            const __m128d tmax = _mm_min_pd(zl2, _mm_min_pd(yl2, _mm_min_pd(xl2, ray_tmax)));

            const int hits =
                _mm_movemask_pd(
                    _mm_or_pd(
                        _mm_cmpgt_pd(tmin, tmax),
                        _mm_or_pd(
                            _mm_cmplt_pd(tmax, ray_tmin),
                            _mm_cmpge_pd(tmin, ray_tmax)))) ^ 3;

            const size_t hit_left = hits & 1;
            const size_t hit_right = hits >> 1;
#else
            const BBoxr& left_bbox = node_ptr->get_left_bbox();
            const BBoxr& right_bbox = node_ptr->get_right_bbox();
            Vec3r tminl = (left_bbox.pmin - ray.org) * inv_dir;
            Vec3r tminr = (right_bbox.pmin - ray.org) * inv_dir;
            Vec3r tmaxl = (left_bbox.pmax - ray.org) * inv_dir;
            Vec3r tmaxr = (right_bbox.pmax - ray.org) * inv_dir;
            Vec3r rminl = min(tminl, tmaxl);
            Vec3r rminr = min(tminr, tmaxr);
            Vec3r rmaxl = max(tminl, tmaxl);
            Vec3r rmaxr = max(tminr, tmaxr);

            Real farl = min(rmaxl);
            Real farr = min(rmaxr);
            Real nearl = max(rminl);
            Real nearr = max(rminr);

            Real left_hit_dist = (farl < ray.tmin || nearl > farl || nearl >= ray.tmax) ? (Real)pos_inf : nearl;
            Real right_hit_dist = (farr < ray.tmin || nearr > farr || nearr >= ray.tmax) ? (Real)pos_inf : nearr;

            size_t hit_left = left_hit_dist < (Real)pos_inf ? 1 : 0;
            size_t hit_right = right_hit_dist < (Real)pos_inf ? 1 : 0;
#endif

            if (hit_left && hit_right)
            {
                if (dir_is_neg[node_ptr->get_split_axis()])
                {
                    node_stack[stack_index++] = node_idx + 1;
                    node_idx = node_ptr->get_right_child();
                }
                else
                {
                    node_stack[stack_index++] = node_ptr->get_right_child();
                    node_idx = node_idx + 1;
                }
                continue;
            }
            else if (hit_left || hit_right)
            {
                node_idx = hit_left ? node_idx + 1 : node_ptr->get_right_child();
                continue;
            }
        }
        else
        {
            // This is a top level BVH leaf
            if (node_ptr->get_num_primitives() == 0)
            {
                // Push bottom level bvh root to the stack
                instance_idx = node_ptr->get_instance_index();
                mesh_bvh_stack_start_index = stack_index;
                node_stack[stack_index++] = bvh_roots[instance_idx];

                // TODO use sse for the transformation
                // Transform the ray
                const Transformr& xfm = inv_transforms[instance_idx];
                ray.org = transform_point(xfm, ray.org);
                ray.dir = transform_vector(xfm, ray.dir);
                inv_dir = rcp(ray.dir);
                dir_is_neg[0] = inv_dir.x < 0;
                dir_is_neg[1] = inv_dir.y < 0;
                dir_is_neg[2] = inv_dir.z < 0;

#ifdef BBOX_SIMD_ISECT
                // Load the ray into SSE registers.
                org_x = _mm_set1_pd(ray.org.x);
                org_y = _mm_set1_pd(ray.org.y);
                org_z = _mm_set1_pd(ray.org.z);
                rcp_dir_x = _mm_set1_pd(inv_dir.x);
                rcp_dir_y = _mm_set1_pd(inv_dir.y);
                rcp_dir_z = _mm_set1_pd(inv_dir.z);
                ray_tmin = _mm_set1_pd(ray.tmin);
#endif
            }
            // This is a bottom level BVH leaf
            else if (visitor.intersect(*node_ptr, ray, hit))
            {
                got_hit = true;
                hit->shape_id = instance_idx;
                ray.tmax = hit->t;
            }
        }

        // If we exited from a bottom bvh tree, we need to restore the ray
        if (stack_index == mesh_bvh_stack_start_index)
        {
            ray.org = orig_ray_org;
            ray.dir = orig_ray_dir;
            inv_dir = rcp(ray.dir);
            dir_is_neg[0] = inv_dir.x < 0;
            dir_is_neg[1] = inv_dir.y < 0;
            dir_is_neg[2] = inv_dir.z < 0;
            mesh_bvh_stack_start_index = -1;

#ifdef BBOX_SIMD_ISECT
            // Load the ray into SSE registers.
            org_x = _mm_set1_pd(ray.org.x);
            org_y = _mm_set1_pd(ray.org.y);
            org_z = _mm_set1_pd(ray.org.z);
            rcp_dir_x = _mm_set1_pd(inv_dir.x);
            rcp_dir_y = _mm_set1_pd(inv_dir.y);
            rcp_dir_z = _mm_set1_pd(inv_dir.z);
            ray_tmin = _mm_set1_pd(ray.tmin);
#endif
        }

        // Pop the next node off the stack
        if (--stack_index >= 0)
            node_idx = node_stack[stack_index];
    }
    r.tmax = ray.tmax;

    return got_hit;
}

template <typename Visitor>
bool intersect_any_two_levels(const Node* nodes, const Transformr* inv_transforms, const uint32* bvh_roots,
                              const Ray& r, HitInfo* hit, Visitor& visitor)
{
    constexpr uint32 BVH_MAX_STACK_SIZE = 32;

    Ray ray(r);

    const Vec3r orig_ray_org = ray.org;
    const Vec3r orig_ray_dir = ray.dir;
    Vec3r inv_dir = rcp(ray.dir);
    Vec3i dir_is_neg = { inv_dir.x < 0, inv_dir.y < 0, inv_dir.z < 0 };

    uint32 node_stack[BVH_MAX_STACK_SIZE];
    uint32 node_idx = 0;
    const bvh::Node* node_ptr;
    int stack_index = 0;
    uint32 instance_idx = 0;
    int mesh_bvh_stack_start_index = -1;

#ifdef BBOX_SIMD_ISECT
    // Load the ray into SSE registers.
    __m128d org_x = _mm_set1_pd(ray.org.x);
    __m128d org_y = _mm_set1_pd(ray.org.y);
    __m128d org_z = _mm_set1_pd(ray.org.z);
    __m128d rcp_dir_x = _mm_set1_pd(inv_dir.x);
    __m128d rcp_dir_y = _mm_set1_pd(inv_dir.y);
    __m128d rcp_dir_z = _mm_set1_pd(inv_dir.z);
    __m128d ray_tmin = _mm_set1_pd(ray.tmin);
#endif

    while (stack_index > -1)
    {
        node_ptr = &nodes[node_idx];

        if (likely(node_ptr->is_interior()))
        {
#ifdef BBOX_SIMD_ISECT
            const __m128d xl1 = _mm_mul_pd(rcp_dir_x, _mm_sub_pd(_mm_load_pd(node_ptr->bbox_data + 0 + 2 * (    dir_is_neg.x)), org_x));
            const __m128d xl2 = _mm_mul_pd(rcp_dir_x, _mm_sub_pd(_mm_load_pd(node_ptr->bbox_data + 0 + 2 * (1 - dir_is_neg.x)), org_x));
            const __m128d yl1 = _mm_mul_pd(rcp_dir_y, _mm_sub_pd(_mm_load_pd(node_ptr->bbox_data + 4 + 2 * (    dir_is_neg.y)), org_y));
            const __m128d yl2 = _mm_mul_pd(rcp_dir_y, _mm_sub_pd(_mm_load_pd(node_ptr->bbox_data + 4 + 2 * (1 - dir_is_neg.y)), org_y));
            const __m128d zl1 = _mm_mul_pd(rcp_dir_z, _mm_sub_pd(_mm_load_pd(node_ptr->bbox_data + 8 + 2 * (    dir_is_neg.z)), org_z));
            const __m128d zl2 = _mm_mul_pd(rcp_dir_z, _mm_sub_pd(_mm_load_pd(node_ptr->bbox_data + 8 + 2 * (1 - dir_is_neg.z)), org_z));

            const __m128d ray_tmax = _mm_set1_pd(ray.tmax);
            const __m128d tmin = _mm_max_pd(zl1, _mm_max_pd(yl1, _mm_max_pd(xl1, ray_tmin)));
            const __m128d tmax = _mm_min_pd(zl2, _mm_min_pd(yl2, _mm_min_pd(xl2, ray_tmax)));

            const int hits =
                _mm_movemask_pd(
                    _mm_or_pd(
                        _mm_cmpgt_pd(tmin, tmax),
                        _mm_or_pd(
                            _mm_cmplt_pd(tmax, ray_tmin),
                            _mm_cmpge_pd(tmin, ray_tmax)))) ^ 3;

            const size_t hit_left = hits & 1;
            const size_t hit_right = hits >> 1;
#else
            const BBoxr& left_bbox = node_ptr->get_left_bbox();
            const BBoxr& right_bbox = node_ptr->get_right_bbox();
            Vec3r tminl = (left_bbox.pmin - ray.org) * inv_dir;
            Vec3r tminr = (right_bbox.pmin - ray.org) * inv_dir;
            Vec3r tmaxl = (left_bbox.pmax - ray.org) * inv_dir;
            Vec3r tmaxr = (right_bbox.pmax - ray.org) * inv_dir;
            Vec3r rminl = min(tminl, tmaxl);
            Vec3r rminr = min(tminr, tmaxr);
            Vec3r rmaxl = max(tminl, tmaxl);
            Vec3r rmaxr = max(tminr, tmaxr);

            Real farl = min(rmaxl);
            Real farr = min(rmaxr);
            Real nearl = max(rminl);
            Real nearr = max(rminr);

            Real left_hit_dist = (farl < ray.tmin || nearl > farl || nearl >= ray.tmax) ? (Real)pos_inf : nearl;
            Real right_hit_dist = (farr < ray.tmin || nearr > farr || nearr >= ray.tmax) ? (Real)pos_inf : nearr;

            size_t hit_left = left_hit_dist < (Real)pos_inf ? 1 : 0;
            size_t hit_right = right_hit_dist < (Real)pos_inf ? 1 : 0;
#endif

            if (hit_left && hit_right)
            {
                if (dir_is_neg[node_ptr->get_split_axis()])
                {
                    node_stack[stack_index++] = node_idx + 1;
                    node_idx = node_ptr->get_right_child();
                }
                else
                {
                    node_stack[stack_index++] = node_ptr->get_right_child();
                    node_idx = node_idx + 1;
                }
                continue;
            }
            else if (hit_left || hit_right)
            {
                node_idx = hit_left ? node_idx + 1 : node_ptr->get_right_child();
                continue;
            }
        }
        else
        {
            // This is a top level BVH leaf
            if (node_ptr->get_num_primitives() == 0)
            {
                // Push bottom level bvh root to the stack
                instance_idx = node_ptr->get_instance_index();
                mesh_bvh_stack_start_index = stack_index;
                node_stack[stack_index++] = bvh_roots[instance_idx];

                // TODO use sse for the transformation
                // Transform the ray
                const Transformr& xfm = inv_transforms[instance_idx];
                ray.org = transform_point(xfm, ray.org);
                ray.dir = transform_vector(xfm, ray.dir);
                inv_dir = rcp(ray.dir);
                dir_is_neg[0] = inv_dir.x < 0;
                dir_is_neg[1] = inv_dir.y < 0;
                dir_is_neg[2] = inv_dir.z < 0;

#ifdef BBOX_SIMD_ISECT
                // Load the ray into SSE registers.
                org_x = _mm_set1_pd(ray.org.x);
                org_y = _mm_set1_pd(ray.org.y);
                org_z = _mm_set1_pd(ray.org.z);
                rcp_dir_x = _mm_set1_pd(inv_dir.x);
                rcp_dir_y = _mm_set1_pd(inv_dir.y);
                rcp_dir_z = _mm_set1_pd(inv_dir.z);
                ray_tmin = _mm_set1_pd(ray.tmin);
#endif
            }
            // This is a bottom level BVH leaf
            else if (visitor.intersect_any(*node_ptr, ray, hit))
            {
                hit->shape_id = instance_idx;
                ray.tmax = hit->t;
                return true;
            }
        }

        // If we exited from a bottom bvh tree, we need to restore the ray
        if (stack_index == mesh_bvh_stack_start_index)
        {
            ray.org = orig_ray_org;
            ray.dir = orig_ray_dir;
            inv_dir = rcp(ray.dir);
            dir_is_neg[0] = inv_dir.x < 0;
            dir_is_neg[1] = inv_dir.y < 0;
            dir_is_neg[2] = inv_dir.z < 0;
            mesh_bvh_stack_start_index = -1;

#ifdef BBOX_SIMD_ISECT
            // Load the ray into SSE registers.
            org_x = _mm_set1_pd(ray.org.x);
            org_y = _mm_set1_pd(ray.org.y);
            org_z = _mm_set1_pd(ray.org.z);
            rcp_dir_x = _mm_set1_pd(inv_dir.x);
            rcp_dir_y = _mm_set1_pd(inv_dir.y);
            rcp_dir_z = _mm_set1_pd(inv_dir.z);
            ray_tmin = _mm_set1_pd(ray.tmin);
#endif
        }

        // Pop the next node off the stack
        if (--stack_index >= 0)
            node_idx = node_stack[stack_index];
    }
    r.tmax = ray.tmax;

    return false;
}

} } // namespace hop::bvh
