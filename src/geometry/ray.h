#pragma once

#include "types.h"
#include "math/math.h"
#include "math/vec3.h"

#define RAY_TFAR Real(1e14)

namespace hop {

class Ray
{
public:
    Ray(const Vec3r& o = Vec3r(), const Vec3r& d = Vec3r(),
        Real tmin = neg_inf, Real tmax = pos_inf)
        : org(o), tmin(tmin), dir(d), tmax(tmax)
    {
    }

    Vec3r org;
    Real tmin;
    Vec3r dir;
    mutable Real tmax;
};

} // namespace hop
