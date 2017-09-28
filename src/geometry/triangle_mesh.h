#pragma once

#include "types.h"
#include "geometry/shape.h"
#include "math/bbox.h"
#include "math/vec2.h"
#include "math/vec3.h"
#include "material/material.h"

#include <string>
#include <vector>
#include <memory>

namespace hop {

class Triangle
{
public:
    Vec3r vertices[3];
    Vec3r normals[3];
    Vec2r uvs[3];
    MaterialID material_id;

    Triangle() : material_id(0) { }

    BBoxr get_bbox() const { return BBoxr(vertices[0], vertices[1], vertices[2]); }
    Vec3r get_centroid() const { return get_bbox().get_centroid(); }
};

class TriangleMesh : public Shape
{
public:
    TriangleMesh(const std::string& name,
                 std::vector<Triangle>& triangles);

    const std::string& get_name() const override { return m_name; }
    ShapeType get_type() const override { return TRIANGLE_MESH; }
    uint64 get_num_primitives() const override { return m_num_primitives; }
    bool is_instance() const override { return false; }

    const BBoxr& get_bbox() const override { return m_bbox; }
    const Vec3r& get_centroid() const override { return m_centroid; }

    const std::vector<Triangle>& get_triangles() const { return m_triangles; }
    const std::vector<BBoxr>& get_triangles_bboxes() const { return m_bboxes; }

    void clear_triangles();
    void clear_bboxes();

private:
    std::string m_name;
    std::vector<Triangle> m_triangles;
    std::vector<BBoxr> m_bboxes;
    BBoxr m_bbox;
    Vec3r m_centroid;
    uint64 m_num_primitives;
};

typedef std::shared_ptr<TriangleMesh> TriangleMeshPtr;

} // namespace hop
