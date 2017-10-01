#pragma once

#include "types.h"
#include "accel/bvh_node.h"
#include "math/bbox.h"
#include "math/vec2.h"
#include "math/vec3.h"
#include "math/transform.h"
#include "util/log.h"

#include <memory>
#include <vector>

namespace hop {

class Ray;
class HitInfo;
class SurfaceInteraction;
class ShapeInstance;
class Material;

class World
{
public:
    ~World() { Log("world") << DEBUG << "world deleted"; }
    void add_shape(ShapeID shape_id);

    void preprocess();

    bool intersect(const Ray& r, HitInfo* hit) const;
    bool intersect_any(const Ray& r, HitInfo* hit) const;

    void get_surface_interaction(const HitInfo& hit, SurfaceInteraction* info);

    BBoxr get_bbox();

    // This will trigger a BBox recalculation when needed
    void set_dirty() { m_dirty = true; }

    bool empty() const { return m_instance_ptrs.empty(); }

private:
    void partition_instances();
    void partition_meshes();

private:
    std::vector<ShapeInstance*> m_instance_ptrs;
    std::vector<Material*> m_materials;
    std::vector<bvh::Node> m_bvh_nodes;
    std::vector<Transformr> m_instance_inv_xfm;
    std::vector<uint32> m_instance_bvh_roots;
    std::vector<Vec3f> m_vertices;
    std::vector<Vec3f> m_normals;
    std::vector<Vec2f> m_uvs;
    bool m_dirty;
    BBoxr m_bbox;
};

} // namespace hop
