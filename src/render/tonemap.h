#pragma once

#include "math/math.h"
#include "math/vec3.h"
#include "spectrum/spectrum.h"

namespace hop {

enum class ToneMapType
{
    LINEAR,
    GAMMA,
    REINHARD,
    FILMIC
};

ToneMapType tonemap_from_string(const char* str);

inline Vec3f tonemap(ToneMapType type, const Spectrum& color)
{
    static float inv_gamma = rcp(2.2f);
    Vec3r col_ = color.get_color();
    Vec3f col = Vec3f(col_.x, col_.y, col_.z);

    switch (type)
    {
        case ToneMapType::GAMMA:
            return Vec3f(pow(col.x, inv_gamma),
                         pow(col.y, inv_gamma),
                         pow(col.z, inv_gamma));

        case ToneMapType::REINHARD:
            return Vec3f(pow(col.x * rcp(1.0f + col.x), inv_gamma),
                         pow(col.y * rcp(1.0f + col.y), inv_gamma),
                         pow(col.z * rcp(1.0f + col.z), inv_gamma));

        // Jim Jejl and Richard Burgess-Dawson formula
        case ToneMapType::FILMIC: {
            Vec3f x = max(Vec3f(0.0f), col - 0.004f);
            return (x * (6.2f * x + 0.5f)) * rcp(x * (6.2f * x + 1.7f) + 0.06f);
        }

        case ToneMapType::LINEAR:
        default:
            return Vec3f(col.x, col.y, col.z);
    }
}

} // namespace hop
