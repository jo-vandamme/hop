#include "camera/projective_camera.h"
#include "math/transform.h"
#include "math/vec2.h"
#include "math/vec3.h"

namespace hop {

ProjectiveCamera::ProjectiveCamera(
        const Transformr& cam2world, const Transformr& cam2screen,
        const Vec2u& film_res, Real lens_radius, Real focal_dist)
    : Camera(cam2world), m_cam2screen(cam2screen)
    , m_lens_radius(lens_radius), m_focal_distance(focal_dist)
{
    Real res = film_res.x < film_res.y ? (Real)film_res.x : (Real)film_res.y;
    m_cam2raster = make_translation(Vec3r(film_res.x * 0.5, film_res.y * 0.5, 0.0)) *
                   make_scale(Vec3r(res, res, 1)) *
                   m_cam2screen;
}

} // namespace hop
