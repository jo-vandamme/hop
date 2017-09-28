#pragma once

#include "types.h"
#include "camera/camera.h"
#include "math/transform.h"
#include "math/vec2.h"

namespace hop {

class ProjectiveCamera : public Camera
{
public:
    ProjectiveCamera(const Vec3r& eye, const Vec3r& target, const Vec3r& up,
                     const Transformr& cam2screen,
                     const Vec2u& film_res, Real lens_radius, Real focal_dist);

    void set_focal_distance(Real d);

protected:
    Transformr m_cam2screen;
    Transformr m_cam2raster;

    Real m_lens_radius;
    Real m_focal_distance;
};

} // namespace hop
