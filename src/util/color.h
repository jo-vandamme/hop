#pragma once

#include "types.h"
#include "math/math.h"
#include "math/vec3.h"

namespace hop {

uint32 vec3_to_uint32(Vec3f col)
{
    uint32 color = (uint8(255.0f * clamp(col.x, 0.0f, 1.0f)) << 0) |
                   (uint8(255.0f * clamp(col.y, 0.0f, 1.0f)) << 8) |
                   (uint8(255.0f * clamp(col.z, 0.0f, 1.0f)) << 16);
    return color;
}

} // namespace hop
