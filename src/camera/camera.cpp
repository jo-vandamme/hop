#include "hop.h"
#include "camera/camera.h"
#include "math/math.h"
#include "math/vec3.h"
#include "math/transform.h"

namespace hop {

Camera::Camera(const Transformr& cam2world)
    : m_cam2world(cam2world)
{
}

void Camera::set_transform(const Transformr& xfm)
{
    m_cam2world = xfm;
}

void Camera::rotate(const Vec3r& axis, Real angle)
{
    if (almost_equal(angle, Real(0)))
        return;

    m_cam2world = make_rotation(axis, angle) * m_cam2world;
}

void Camera::translate(const Vec3r& delta)
{
    m_cam2world = make_translation(delta) * m_cam2world;
}

} // namespace hop
