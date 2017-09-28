#include "camera/camera.h"
#include "math/vec3.h"
#include "math/transform.h"

namespace hop {

Camera::Camera(const Vec3r& eye, const Vec3r& target, const Vec3r& up)
    : m_eye(eye), m_target(target), m_up(up)
{
    update_transform();
}

void Camera::set_eye(const Vec3r& eye)
{
    m_eye = eye;
}

void Camera::set_target(const Vec3r& target)
{
    m_target = target;
}

void Camera::set_up(const Vec3r& up)
{
    m_up = up;
}

void Camera::update_transform()
{
    m_cam2world = make_lookat(m_eye, m_target, m_up);
}

} // namespace hop
