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

    info->position = b0     * m_vertices[hit.primitive_id * 3 + 0] +
                     hit.b1 * m_vertices[hit.primitive_id * 3 + 1] +
                     hit.b2 * m_vertices[hit.primitive_id * 3 + 2];

    info->normal = b0     * m_normals[hit.primitive_id * 3 + 0] +
                   hit.b1 * m_normals[hit.primitive_id * 3 + 1] +
                   hit.b2 * m_normals[hit.primitive_id * 3 + 2];

    info->uv = b0     * m_uvs[hit.primitive_id * 3 + 0] +
               hit.b1 * m_uvs[hit.primitive_id * 3 + 1] +
               hit.b2 * m_uvs[hit.primitive_id * 3 + 2];

    info->t = hit.t;
    info->shape_id = hit.shape_id;
    info->material_id = m_materials[hit.primitive_id];

    const Transformr xfm = inverse(m_instance_inv_xfm[hit.shape_id]);
    info->position = transform_point(xfm, info->position);
    info->normal = transform_normal(xfm, info->normal);
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

// Partition mesh instances so that each instance ends up in its own BVH leaf.
void World::partition_instances()
{
    std::vector<ShapeInstance*> instances_vec;
    for (auto id : m_instance_ids)
        instances_vec.push_back(ShapeManager::get<ShapeInstance>(id));

    Log("world") << INFO << "building scene BVH tree (" << instances_vec.size() << " instanced meshes)";

    m_instance_bvh_roots.resize(instances_vec.size());
    m_instance_inv_xfm.resize(instances_vec.size());

    for (size_t i = 0; i < instances_vec.size(); ++i)
        m_instance_inv_xfm[i] = inverse(instances_vec[i]->get_transform());

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

    class InstAccessor
    {
    public:
        const BBoxr& get_bbox(ShapeInstance* ptr) const { return ptr->get_bbox(); }
        const Vec3r& get_centroid(ShapeInstance* ptr) const { return ptr->get_centroid(); }
    };

    InstAccessor accessor;

    m_bvh_nodes = bvh::Builder<ShapeInstance*, InstAccessor,
        bvh::SAHStrategy<ShapeInstance*, InstAccessor>>::build(
            &accessor, instances_vec, 1, inst_leaf_cb);
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

    for (const auto& kv : mesh_to_instance_map)
    {
        TriangleMesh* mesh = kv.first;

        Log("world") << INFO << "building BVH tree for " << mesh->get_name()
                             << " (" << mesh->get_num_primitives() << " triangles, "
                             << kv.second.size() << " instances)";

        uint32 num_bvh2_leaves = 0;

        auto tri_leaf_cb = [&](bvh::Node* leaf, const std::vector<size_t>& tri_indices)
            //const std::vector<Triangle>& triangles)
        {
            leaf->set_primitives(triangle_offset, tri_indices.size());
            ++num_bvh2_leaves;

            // Copy triangles to flat array
            for (auto i : tri_indices)
            {
                const Triangle& tri = mesh->get_triangles()[i];

                m_vertices[vertex_offset + 0] = tri.vertices[0];
                m_vertices[vertex_offset + 1] = tri.vertices[1];
                m_vertices[vertex_offset + 2] = tri.vertices[2];

                m_normals[vertex_offset + 0] = tri.normals[0];
                m_normals[vertex_offset + 1] = tri.normals[1];
                m_normals[vertex_offset + 2] = tri.normals[2];

                m_uvs[vertex_offset + 0] = tri.uvs[0];
                m_uvs[vertex_offset + 1] = tri.uvs[1];
                m_uvs[vertex_offset + 2] = tri.uvs[2];

                m_materials[triangle_offset] = tri.material_id;

                vertex_offset += 3;
                ++triangle_offset;
            }
        };

        class TriAccessor
        {
        public:
            TriAccessor(const TriangleMesh* mesh) : bboxes(mesh->get_triangles_bboxes()) { }

            const BBoxr& get_bbox(size_t i) const { return bboxes[i]; }
            Vec3r get_centroid(size_t i) const { return bboxes[i].get_centroid(); }

            const std::vector<BBoxr>& bboxes;
        };

        TriAccessor accessor(mesh);
        std::vector<size_t> tri_indices;
        size_t num_tris = mesh->get_triangles().size();
        for (size_t i = 0; i < num_tris; ++i)
            tri_indices.push_back(i);

        auto bvh_nodes = bvh::Builder<size_t, TriAccessor,
             bvh::SAHStrategy<size_t, TriAccessor>>::build(
                &accessor, tri_indices, MIN_PRIMS_PER_LEAF, tri_leaf_cb);

        mesh->clear_bboxes();
        mesh->clear_triangles();

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
    Visitor(const Vec3r* vertices) : m_vertices(vertices) { }

    bool intersect(const bvh::Node& node, const Ray& ray, HitInfo* hit) const;
    bool intersect_any(const bvh::Node& node, const Ray& ray, HitInfo* hit) const;

    const Vec3r* m_vertices;
};

inline bool Visitor::intersect(const bvh::Node& node, const Ray& ray, HitInfo* hit) const
{
    bool got_hit = false;
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
}

inline bool Visitor::intersect_any(const bvh::Node& node, const Ray& ray, HitInfo* hit) const
{
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
}

bool World::intersect(const Ray& r, HitInfo* hit) const
{
    Visitor visitor(&m_vertices[0]);
    return bvh::intersect_two_levels(&m_bvh_nodes[0], &m_instance_inv_xfm[0], &m_instance_bvh_roots[0], r, hit, visitor);
}

bool World::intersect_any(const Ray& r, HitInfo* hit) const
{
    Visitor visitor(&m_vertices[0]);
    return bvh::intersect_any_two_levels(&m_bvh_nodes[0], &m_instance_inv_xfm[0], &m_instance_bvh_roots[0], r, hit, visitor);
}

} // namespace hop
