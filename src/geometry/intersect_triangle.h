#pragma once

#include "hop.h"
#include "math/math.h"
#include "math/vec3.h"
#include "geometry/ray.h"
#include "geometry/hit_info.h"

namespace hop {

inline bool intersect_triangle(const Vec3r& v0, const Vec3r& e1, const Vec3r& e2, const Ray& ray, HitInfo* hit)
{
    const Vec3r s1 = cross(ray.dir, e2);
    const Real det = dot(s1, e1);
    //if (almost_equal(det, Real(0)))
    if (det > -RAY_EPSILON && det < RAY_EPSILON)
        return false;

    const Real inv_det = rcp(det);

    // Compute first barycentric coordinate
    const Vec3r d = ray.org - v0;
    const Real b1 = dot(d, s1) * inv_det;
    if (b1 < 0.0 - RAY_EPSILON || b1 > 1.0 + RAY_EPSILON)
        return false;

    // Compute second barycentric coordinate
    const Vec3r s2 = cross(d, e1);
    const Real b2 = dot(ray.dir, s2) * inv_det;
    if (b2 < 0.0 - RAY_EPSILON || (b1 + b2) > 1.0 + RAY_EPSILON)
        return false;

    // Compute distance to intersection point
    const Real t = dot(e2, s2) * inv_det;
    if (t < ray.tmin || t > ray.tmax)
        return false;

    hit->t = t;
    hit->b1 = b1;
    hit->b2 = b2;
    ray.tmax = hit->t;
    return true;
}

} // namespace hop
