#pragma once

#include "hop.h"
#include "types.h"
#include "math/vec2.h"
#include "math/vec3.h"

namespace hop {

class Triangle
{
public:
    Vec3r vertices[3];
    Vec3r normals[3];
    Vec2r uvs[3];
    uint32 material;

    BBoxr bbox;
    Vec3r centroid;

    Triangle() : material(0) { }

    const BBoxr& get_bbox() const { return bbox; }
    const Vec3r& get_centroid() const { return centroid; }
};

} // namespace hop
