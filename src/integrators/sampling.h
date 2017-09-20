#pragma once

#include "math/math.h"
#include "math/vec3.h"

namespace hop { namespace sample {

Vec3r uniform_sample_hemisphere(Real u1, Real u2)
{
    const Real r = sqrt(1.0 - u1 * u1);
    const Real phi = 2 * (Real)pi * u2;
    return Vec3r(cos(phi) * r, sin(phi) * r, u1);
}

Vec3r cosine_sample_hemisphere(Real u1, Real u2)
{
    const Real r = sqrt(u1);
    const Real theta = 2.0 * (Real)pi * u2;
    const Real x = r * cos(theta);
    const Real y = r * sin(theta);
    return Vec3r(x, y, sqrt(max(0.0, 1.0 - u1)));
}

} } // namespace hop::sample
