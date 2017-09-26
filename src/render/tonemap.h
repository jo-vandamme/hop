#pragma once

#include "math/math.h"
#include "math/vec3.h"

namespace hop {

enum class ToneMapType
{
    LINEAR,
    GAMMA,
    REINHARD,
    FILMIC
};

ToneMapType tonemap_from_string(const char* str);

inline Vec3f tonemap(ToneMapType type, const Vec3r& color)
{
    static float inv_gamma = rcp(2.2f);
    switch (type)
    {
        case ToneMapType::GAMMA:
            return Vec3f(pow(float(color.x), inv_gamma),
                         pow(float(color.y), inv_gamma),
                         pow(float(color.z), inv_gamma));

        case ToneMapType::REINHARD:
            return Vec3f(pow(float(color.x) * rcp(1.0f + float(color.x)), inv_gamma),
                         pow(float(color.y) * rcp(1.0f + float(color.y)), inv_gamma),
                         pow(float(color.z) * rcp(1.0f + float(color.z)), inv_gamma));

        case ToneMapType::FILMIC: {
            Vec3f c = Vec3f(float(color.x), float(color.y), float(color.z));
            Vec3f x = max(Vec3f(0.0f), c - 0.004f);
            return (x * (6.2f * x + 0.5f)) * rcp(x * (6.2f * x + 1.7f) + 0.06f);
        }

        case ToneMapType::LINEAR:
        default:
            return Vec3f(float(color.x), float(color.y), float(color.z));
    }
}

} // namespace hop
