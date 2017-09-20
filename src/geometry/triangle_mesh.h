#pragma once

#include "types.h"
#include "geometry/shape.h"
#include "geometry/triangle.h"
#include "geometry/hit_info.h"
#include "geometry/surface_interaction.h"
#include "math/bbox.h"

#include <string>
#include <vector>
#include <memory>

namespace hop {

class TriangleMesh : public Shape
{
public:
    TriangleMesh(const std::string& name,
                 std::vector<Triangle>& triangles);

    const std::string& get_name() const override { return m_name; }
    ShapeType get_type() const override { return TRIANGLE_MESH; }
    uint32 get_num_primitives() const override { return m_triangles.size(); }
    bool is_instance() const override { return false; }

    const BBoxr& get_bbox() const override { return m_bbox; }
    const Vec3r& get_centroid() const override { return m_centroid; }

    void get_surface_interaction(const HitInfo& hit, SurfaceInteraction* info) override;

    const std::vector<Triangle>& get_triangles() const { return m_triangles; }

private:
    std::string m_name;
    std::vector<Triangle> m_triangles;
    BBoxr m_bbox;
    Vec3r m_centroid;
};

typedef std::shared_ptr<TriangleMesh> TriangleMeshPtr;

} // namespace hop
