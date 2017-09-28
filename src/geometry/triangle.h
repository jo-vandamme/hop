#pragma once

#include "types.h"
#include "math/vec2.h"
#include "math/vec3.h"
#include "material/material.h"

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

} // namespace hop
