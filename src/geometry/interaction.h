#pragma once

#include "types.h"
#include "math/math.h"
#include "math/vec2.h"
#include "math/vec3.h"
#include "geometry/shape_instance.h"

namespace hop {

class Material;

class SurfaceInteraction
{
public:
    Vec3r position;
    Vec3f normal;
    Vec3f dpdu, dpdv;
    Vec3f dndu, dndv;
    Vec3f shading_normal;
    Vec3f shading_dpdu, shading_dpdv;
    Vec3f shading_dndu, shading_dndv;
    Vec3f wo;
    Vec2f uv;

    const ShapeInstance* shape;
    const Material* material;
};

} // namespace hop
