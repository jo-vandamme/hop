#pragma once

#include "types.h"
#include "math/math.h"
#include "math/vec2.h"
#include "math/vec3.h"
#include "geometry/shape_instance.h"

namespace hop {

class Material;

class Interaction
{
public:
    Vec3r position;
    Vec3f normal;
    Vec3f wo;

    Interaction() { }

    Interaction(const Vec3r& p, const Vec3f& n, const Vec3f& wo)
        : position(p), normal(n), wo(wo)
    {
    }
};

class SurfaceInteraction : public Interaction
{
public:
    Vec2f uv;
    const ShapeInstance* shape;
    const Material* material;

    SurfaceInteraction() { }

    SurfaceInteraction(const Vec3r& p, const Vec3f& n, const Vec2f& uv, const Vec3f& wo,
                       const ShapeInstance* shape, const Material* mat)
        : Interaction(p, n, wo)
        , uv(uv), shape(shape), material(mat)
    {
        if (shape && shape->transform_swaps_handedness())
            normal *= -1;
    }
};

} // namespace hop
