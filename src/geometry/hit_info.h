#pragma once

#include "types.h"
#include "math/vec3.h"

namespace hop {

class HitInfo
{
public:
    Vec3r ray_dir;
    Real t;
    Real b1;
    Real b2;
    int32 primitive_id;
    int32 shape_id;
};

} // namespace hop
