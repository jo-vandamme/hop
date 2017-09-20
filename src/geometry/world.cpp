#include "hop.h"
#include "geometry/world.h"
#include "geometry/shape.h"
#include "geometry/triangle.h"
#include "geometry/triangle_mesh.h"
#include "geometry/ray.h"
#include "geometry/hit_info.h"
#include "geometry/surface_interaction.h"
#include "geometry/intersect_triangle.h"
#include "math/math.h"
#include "math/bbox.h"
#include "math/vec3.h"
#include "math/transform.h"
#include "accel/bvh_node.h"
#include "accel/bvh_builder.h"
#include "accel/bvh_intersector_two_levels.h"
#include "util/stop_watch.h"
#include "util/log.h"

#include <memory>
#include <vector>
#include <map>
#include <cassert>

namespace hop {

void World::add_shape(ShapeID id)
{
    Shape* shape = ShapeManager::get<Shape>(id);
    if (!shape->is_instance())
        id = ShapeManager::create<ShapeInstance>(id, Transformr());

    ShapeInstance* instance = ShapeManager::get<ShapeInstance>(id);
    m_instance_ids.push_back(id);

    m_dirty = true;

    Log("world") << INFO << "added shape [id: " << instance->get_id()
                 << " name: " << instance->get_name()
                 << " primitives: " << instance->get_num_primitives() << "]";
}

BBoxr World::get_bbox()
{
    if (m_dirty)
    {
        m_bbox = BBoxr();
        for (auto id : m_instance_ids)
            m_bbox.merge(ShapeManager::get<Shape>(id)->get_bbox());
        m_dirty = false;
    }
    return m_bbox;
}

void World::get_surface_interaction(const HitInfo& hit, SurfaceInteraction* info)
{
    Real b0 = 1 - hit.b1 - hit.b2;

#ifdef TRIS_SIMD_ISECT
    info->normal = Vec3r(b0, hit.b1, hit.b2);
    return;
#endif

    info->position = b0     * m_vertices[hit.primitive_id * 3 + 0] +
                     hit.b1 * m_vertices[hit.primitive_id * 3 + 1] +
                     hit.b2 * m_vertices[hit.primitive_id * 3 + 2];

    info->normal = b0     * m_normals[hit.primitive_id * 3 + 0] +
                   hit.b1 * m_normals[hit.primitive_id * 3 + 1] +
                   hit.b2 * m_normals[hit.primitive_id * 3 + 2];

    info->uv = b0     * m_uvs[hit.primitive_id * 3 + 0] +
               hit.b1 * m_uvs[hit.primitive_id * 3 + 1] +
               hit.b2 * m_uvs[hit.primitive_id * 3 + 2];

    info->position = transform_point(m_instance_transforms[hit.shape_id], info->position);
    info->normal = transform_normal(m_instance_transforms[hit.shape_id], info->normal);
}

void World::preprocess()
{
    StopWatch stop_watch;
    stop_watch.start();
    Log("world") << INFO << "preprocessing scene";

    partition_instances();
    partition_meshes();

    stop_watch.stop();
    Log("world") << INFO << "preprocessed scene in " << stop_watch.get_elapsed_time_ms() << " ms";

    uint64 total = 0;
    for (auto id : m_instance_ids)
        total += ShapeManager::get<Shape>(id)->get_num_primitives();

    Log("world") << INFO << m_vertices.size() / 3 << " unique triangles, "
                         << m_instance_ids.size() << " instances, "
                         << total << " instanced triangles";
}

class InstAccessor
{
public:
    static const BBoxr& get_bbox(ShapeInstance* ptr) { return ptr->get_bbox(); }
    static const Vec3r& get_centroid(ShapeInstance* ptr) { return ptr->get_centroid(); }
};

class TriAccessor
{
public:
    static const BBoxr& get_bbox(const Triangle& tri) { return tri.get_bbox(); }
    static const Vec3r& get_centroid(const Triangle& tri) { return tri.get_centroid(); }
};

// Partition mesh instances so that each instance ends up in its own BVH leaf.
void World::partition_instances()
{
    std::vector<ShapeInstance*> instances_vec;
    for (auto id : m_instance_ids)
        instances_vec.push_back(ShapeManager::get<ShapeInstance>(id));

    Log("world") << INFO << "building scene BVH tree (" << instances_vec.size() << " instanced meshes)";

    m_instance_bvh_roots.resize(instances_vec.size());
    m_instance_transforms.resize(instances_vec.size());

    for (size_t i = 0; i < instances_vec.size(); ++i)
        m_instance_transforms[i] = instances_vec[i]->get_transform();

    auto inst_leaf_cb = [&](bvh::Node* leaf, const std::vector<ShapeInstance*>& instances)
    {
        ShapeInstance* inst = instances[0];
        for (size_t i = 0; i < instances_vec.size(); ++i)
        {
            if (instances_vec[i] == inst)
            {
                leaf->set_instance_index(i);
                break;
            }
        }
    };

    m_bvh_nodes = bvh::Builder<ShapeInstance*, InstAccessor,
        bvh::SAHStrategy<ShapeInstance*, InstAccessor>>::build(
            instances_vec, 1, inst_leaf_cb);
}

// Partition each mesh into its own BVH. Update all instances to point
// to this mesh BVH.
void World::partition_meshes()
{
    // Generate a map of meshes to lists of instance indices
    std::map<TriangleMesh*, std::vector<uint32>> mesh_to_instance_map;
    for (size_t i = 0; i < m_instance_ids.size(); ++i)
    {
        ShapeInstance* inst = ShapeManager::get<ShapeInstance>(m_instance_ids[i]);
        if (inst->get_type() == TRIANGLE_MESH)
            mesh_to_instance_map[reinterpret_cast<TriangleMesh*>(inst->get_shape())].push_back(i);
    }

    // Scan all unique meshes and calculate the total number of vertices for preallocation
    uint32 total_vertices = 0;
    for (const auto& kv : mesh_to_instance_map)
        total_vertices += 3 * kv.first->get_triangles().size();

    m_vertices.resize(total_vertices);
    m_normals.resize(total_vertices);
    m_uvs.resize(total_vertices);
    m_materials.resize(total_vertices / 3);

    uint32 vertex_offset = 0;
    uint32 triangle_offset = 0;
#ifdef TRIS_SIMD_ISECT
    uint32 packed_triangle_offset = 0;
#endif

    for (const auto& kv : mesh_to_instance_map)
    {
        TriangleMesh* mesh = kv.first;

        Log("world") << INFO << "building BVH tree for " << mesh->get_name()
                             << " (" << mesh->get_num_primitives() << " triangles, "
                             << kv.second.size() << " instances)";

        uint32 num_bvh2_leaves = 0;

        auto tri_leaf_cb = [&](bvh::Node* leaf, const std::vector<Triangle>& triangles)
        {
#ifdef TRIS_SIMD_ISECT
            leaf->set_primitives(packed_triangle_offset, triangles.size());
#else
            leaf->set_primitives(triangle_offset, triangles.size());
#endif
            ++num_bvh2_leaves;

            // Copy triangles to flat array
            for (auto& tri : triangles)
            {
                m_vertices[vertex_offset + 0] = tri.vertices[0];
                m_vertices[vertex_offset + 1] = tri.vertices[1];
                m_vertices[vertex_offset + 2] = tri.vertices[2];

                m_normals[vertex_offset + 0] = tri.normals[0];
                m_normals[vertex_offset + 1] = tri.normals[1];
                m_normals[vertex_offset + 2] = tri.normals[2];

                m_uvs[vertex_offset + 0] = tri.uvs[0];
                m_uvs[vertex_offset + 1] = tri.uvs[1];
                m_uvs[vertex_offset + 2] = tri.uvs[2];

                m_materials[triangle_offset] = tri.material;

                vertex_offset += 3;
                ++triangle_offset;
            }

#ifdef TRIS_SIMD_ISECT
            for (size_t i = 0; i < triangles.size();)
            {
                uint32 i0 = (i + 0);
                uint32 i1 = (i + 1) >= triangles.size() ? i : i + 1;
                uint32 i2 = (i + 2) >= triangles.size() ? i : i + 2;
                uint32 i3 = (i + 3) >= triangles.size() ? i : i + 3;

                __m256d mask = _mm256_set_pd(0, 0, 0, 0);
                if      (i + 1 >= triangles.size())
                    mask = _mm256_set_pd(0, 1, 1, 1);
                else if (i + 2 >= triangles.size())
                    mask = _mm256_set_pd(0, 0, 1, 1);
                else if (i + 3 >= triangles.size())
                    mask = _mm256_set_pd(0, 0, 0, 1);

                i += 4;

                const Triangle& t0 = triangles[i0];
                const Triangle& t1 = triangles[i1];
                const Triangle& t2 = triangles[i2];
                const Triangle& t3 = triangles[i3];

                const Vec3r t0e1 = t0.vertices[1] - t0.vertices[0];
                const Vec3r t1e1 = t1.vertices[1] - t1.vertices[0];
                const Vec3r t2e1 = t2.vertices[1] - t2.vertices[0];
                const Vec3r t3e1 = t3.vertices[1] - t3.vertices[0];

                const Vec3r t0e2 = t0.vertices[2] - t0.vertices[0];
                const Vec3r t1e2 = t1.vertices[2] - t1.vertices[0];
                const Vec3r t2e2 = t2.vertices[2] - t2.vertices[0];
                const Vec3r t3e2 = t3.vertices[2] - t3.vertices[0];

                const Vec3r t0v0 = t0.vertices[0];
                const Vec3r t1v0 = t1.vertices[0];
                const Vec3r t2v0 = t2.vertices[0];
                const Vec3r t3v0 = t3.vertices[0];

                bvh::PackedTriangles ALIGN(32) packed_tris;
                packed_tris.e1[0] = _mm256_set_pd(t0e1.x, t1e1.x, t2e1.x, t3e1.x);
                packed_tris.e1[1] = _mm256_set_pd(t0e1.y, t1e1.y, t2e1.y, t3e1.y);
                packed_tris.e1[2] = _mm256_set_pd(t0e1.z, t1e1.z, t2e1.z, t3e1.z);
                packed_tris.e2[0] = _mm256_set_pd(t0e2.x, t1e2.x, t2e2.x, t3e2.x);
                packed_tris.e2[1] = _mm256_set_pd(t0e2.y, t1e2.y, t2e2.y, t3e2.y);
                packed_tris.e2[2] = _mm256_set_pd(t0e2.z, t1e2.z, t2e2.z, t3e2.z);
                packed_tris.v0[0] = _mm256_set_pd(t0v0.x, t1v0.x, t2v0.x, t3v0.x);
                packed_tris.v0[1] = _mm256_set_pd(t0v0.y, t1v0.y, t2v0.y, t3v0.y);
                packed_tris.v0[2] = _mm256_set_pd(t0v0.z, t1v0.z, t2v0.z, t3v0.z);
                packed_tris.inactive_mask = mask;

                m_triangles.push_back(packed_tris);
                ++packed_triangle_offset;
            }
#endif
        };

        auto bvh_nodes = bvh::Builder<Triangle, TriAccessor,
             bvh::SAHStrategy<Triangle, TriAccessor>>::build(
                mesh->get_triangles(), MIN_PRIMS_PER_LEAF, tri_leaf_cb);

        // For all instances that point to this mesh, set their bvh_root to this mesh
        const std::vector<uint32>& instances = kv.second;
        int32 offset = (int32)m_bvh_nodes.size();
        for (size_t i = 0; i < instances.size(); ++i)
            m_instance_bvh_roots[instances[i]] = uint32(offset);

        // Update the nodes indices and push them at the end of the bvh node list
        for (size_t i = 0; i < bvh_nodes.size(); ++i)
            bvh_nodes[i].offset_child_nodes(offset);
        m_bvh_nodes.insert(m_bvh_nodes.end(), bvh_nodes.begin(), bvh_nodes.end());
    }
}

class Visitor
{
public:
#ifdef TRIS_SIMD_ISECT
    Visitor(const Vec3r* vertices, const bvh::PackedTriangles* triangles) : m_vertices(vertices), m_triangles(triangles) { }
#else
    Visitor(const Vec3r* vertices) : m_vertices(vertices) { }
#endif

#ifdef TRIS_SIMD_ISECT
    bool intersect(const bvh::Node& node, const bvh::PackedRay& ray, HitInfo* hit) const;
    bool intersect_any(const bvh::Node& node, const bvh::PackedRay& ray, HitInfo* hit) const;
    const bvh::PackedTriangles* m_triangles;
#else
    bool intersect(const bvh::Node& node, const Ray& ray, HitInfo* hit) const;
    bool intersect_any(const bvh::Node& node, const Ray& ray, HitInfo* hit) const;
#endif

    const Vec3r* m_vertices;
};

#ifdef TRIS_SIMD_ISECT
inline bool Visitor::intersect(const bvh::Node& node, const bvh::PackedRay& pray, HitInfo* hit) const
#else
inline bool Visitor::intersect(const bvh::Node& node, const Ray& ray, HitInfo* hit) const
#endif
{
    bool got_hit = false;

#ifdef TRIS_SIMD_ISECT
    int32 num_primitives = int32(node.get_num_primitives());
    uint32 packed_tri_idx = node.get_primitives_offset();

    while (num_primitives > 0)
    {
        num_primitives -= 4;
        bvh::PackedHitInfo phit;
        if (intersect_triangles_simd(pray, m_triangles[packed_tri_idx], phit))
        {
            got_hit = true;
            hit->b1 = phit.b1;
            hit->b2 = phit.b2;
            hit->t = phit.t;
            hit->primitive_id = packed_tri_idx + phit.idx;
        }
        ++packed_tri_idx;
    }
    return got_hit;

#else
    uint32 num_primitives = node.get_num_primitives();
    uint32 tri_idx = node.get_primitives_offset();

    uint32 vert_index = tri_idx * 3;
    for (; vert_index < (tri_idx + num_primitives) * 3; vert_index += 3)
    {
        const Vec3r& v0 = m_vertices[vert_index + 0];
        const Vec3r& v1 = m_vertices[vert_index + 1];
        const Vec3r& v2 = m_vertices[vert_index + 2];
        const Vec3r e1 = v1 - v0;
        const Vec3r e2 = v2 - v0;
        if (intersect_triangle(v0, e1, e2, ray, hit))
        {
            got_hit = true;
            hit->primitive_id = vert_index / 3;
        }
    }
    return got_hit;
#endif
}

#ifdef TRIS_SIMD_ISECT
inline bool Visitor::intersect_any(const bvh::Node& node, const bvh::PackedRay& pray, HitInfo* hit) const
#else
inline bool Visitor::intersect_any(const bvh::Node& node, const Ray& ray, HitInfo* hit) const
#endif
{
#ifdef TRIS_SIMD_ISECT
    int32 num_primitives = int32(node.get_num_primitives());
    uint32 packed_tri_idx = node.get_primitives_offset();

    while (num_primitives > 0)
    {
        num_primitives -= 4;
        bvh::PackedHitInfo phit;
        if (intersect_triangles_simd(pray, m_triangles[packed_tri_idx], phit))
        {
            hit->t = phit.t;
            hit->primitive_id = packed_tri_idx + phit.idx;
            return true;
        }
        ++packed_tri_idx;
    }
    return false;

#else
    uint32 num_primitives = node.get_num_primitives();
    uint32 tri_idx = node.get_primitives_offset();

    uint32 vert_index = tri_idx * 3;
    for (; vert_index < (tri_idx + num_primitives) * 3; vert_index += 3)
    {
        const Vec3r& v0 = m_vertices[vert_index + 0];
        const Vec3r& v1 = m_vertices[vert_index + 1];
        const Vec3r& v2 = m_vertices[vert_index + 2];
        const Vec3r e1 = v1 - v0;
        const Vec3r e2 = v2 - v0;
        if (intersect_triangle(v0, e1, e2, ray, hit))
        {
            hit->primitive_id = vert_index / 3;
            return true;
        }
    }
    return false;
#endif
}

bool World::intersect(const Ray& r, HitInfo* hit) const
{
#ifdef TRIS_SIMD_ISECT
    Visitor visitor(&m_vertices[0], &m_triangles[0]);
#else
    Visitor visitor(&m_vertices[0]);
#endif
    return bvh::intersect_two_levels(&m_bvh_nodes[0], &m_instance_transforms[0], &m_instance_bvh_roots[0], r, hit, visitor);
}

bool World::intersect_any(const Ray& r, HitInfo* hit) const
{
#ifdef TRIS_SIMD_ISECT
    Visitor visitor(&m_vertices[0], &m_triangles[0]);
#else
    Visitor visitor(&m_vertices[0]);
#endif
    return bvh::intersect_any_two_levels(&m_bvh_nodes[0], &m_instance_transforms[0], &m_instance_bvh_roots[0], r, hit, visitor);
}

} // namespace hop
