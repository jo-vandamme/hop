#pragma once

#include "hop.h"
#include "camera/projective_camera.h"
#include "camera/camera_sample.h"
#include "math/transform.h"
#include "math/vec2.h"

namespace hop {

class PerspectiveCamera : public ProjectiveCamera
{
public:
    PerspectiveCamera(const Vec3r& eye, const Vec3r& target, const Vec3r& up, const Vec2u& film_res,
                      Real fovy = 90.0, Real lens_radius = 0.0, Real focal_distance = 0.0,
                      Real near = RAY_TMIN, Real far = RAY_TFAR);

    Real generate_ray(const CameraSample& sample, Ray* ray) override;

private:
    Real m_near;
    Real m_far;
};

} // namespace hop
