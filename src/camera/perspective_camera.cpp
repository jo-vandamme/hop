#include "camera/perspective_camera.h"
#include "camera/camera_sample.h"
#include "geometry/ray.h"
#include "math/transform.h"
#include "math/math.h"
#include "math/vec2.h"
#include "math/vec3.h"

namespace hop {

Vec2r concentric_sample_disk(const Vec2r& u)
{
    Vec2r u_offset = Real(2) * u - Vec2r(1, 1);

    if (almost_equal(u_offset.x, Real(0)) && almost_equal(u_offset.y, Real(0)))
        return Vec2r(0, 0);

    Real theta, r;
    if (abs(u_offset.x) > abs(u_offset.y))
    {
        r = u_offset.x;
        theta = (Real)pi * 0.25 * u_offset.y * rcp(u_offset.x);
    }
    else
    {
        r = u_offset.y;
        theta = (Real)pi * 0.5 - (Real)pi * 0.25 * u_offset.x * rcp(u_offset.y);
    }
    return r * Vec2r(cos(theta), sin(theta));
}

PerspectiveCamera::PerspectiveCamera(
        const Transformr& cam2world, const Vec2u& film_res,
        Real fovy, Real lens_radius, Real focal_distance, Real near, Real far)
    : ProjectiveCamera(
            cam2world, make_perspective(fovy, near, far),
            film_res, lens_radius, focal_distance)
    , m_near(near), m_far(far)
{
}

Real PerspectiveCamera::generate_ray(const CameraSample& sample, Ray* ray)
{
    Vec3r film_point = Vec3r(sample.film_point.x, sample.film_point.y, 0);
    Vec3r cam_point = transform_point(inverse(m_cam2raster), film_point);

    Vec3r ray_org = Vec3r(0, 0, 0);
    Vec3r ray_dir = normalize(cam_point);

    if (m_lens_radius > 0)
    {
        Vec2r lens_point = m_lens_radius * concentric_sample_disk(sample.lens_point);
        Real ft = -m_focal_distance * rcp(ray_dir.z);
        Vec3r focus_point = ray_org + ray_dir * ft;
        ray_org = Vec3r(lens_point.x, lens_point.y, 0);
        ray_dir = normalize(focus_point - ray_org);
    }

    ray->org = transform_point(m_cam2world, ray_org);
    ray->dir = transform_vector(m_cam2world, ray_dir);
    ray->tmin = m_near;
    ray->tmax = m_far;

    return 1.0;
}

} // namespace hop
