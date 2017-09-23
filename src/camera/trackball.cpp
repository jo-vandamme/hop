#include "camera/trackball.h"
#include "camera/camera.h"
#include "render/renderer.h"
#include "util/log.h"
#include "math/math.h"
#include "math/vec3.h"
#include "math/transform.h"

#include <memory>

namespace hop {

TrackBall::TrackBall(std::shared_ptr<Camera> camera, Renderer* renderer)
    : m_camera(camera), m_renderer(renderer)
    , m_motion_sensitivity(0.01), m_zoom_sensitivity(0.1)
    , m_update_last_pos(false), m_dirty(false)
    , m_rotating(false), m_panning(false), m_zooming(false)
{
    m_orig_eye    = m_camera->get_eye();
    m_orig_target = m_camera->get_target();
    m_orig_up     = m_camera->get_up();
}

TrackBall::~TrackBall()
{
}

void TrackBall::update(float /*time*/)
{
    if (m_dirty)
    {
        m_camera->set_eye(m_eye);
        m_camera->set_target(m_target);
        m_camera->set_up(m_up);
        m_camera->update_transform();

        if (m_renderer)
            m_renderer->reset();
    }
    m_dirty = false;
}

void TrackBall::reset()
{
    m_eye = m_orig_eye;
    m_target = m_orig_target;
    m_up = m_orig_up;
    m_dirty = true;
}

void TrackBall::on_button_down(int button)
{
    m_rotating = m_panning = m_zooming = false;
    if (button == GLFW_MOUSE_BUTTON_LEFT)
        m_rotating = true;
    else if (button == GLFW_MOUSE_BUTTON_RIGHT)
        m_zooming = true;
    else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
        m_panning = true;

    m_eye    = m_camera->get_eye();
    m_target = m_camera->get_target();
    m_up     = m_camera->get_up();

    m_update_last_pos = true;
}

void TrackBall::on_button_up(int /*button*/)
{
    m_rotating = m_panning = m_zooming = false;
}

void TrackBall::on_motion(double x, double y)
{
    if (m_update_last_pos)
        m_last_pos = Vec2r(x, y);
    m_update_last_pos = false;

    if (m_rotating && (x != m_last_pos.x || y != m_last_pos.y))
    {
        Vec3r right = normalize(cross(m_eye - m_target, m_up));
        Vec3r diff = normalize(right) * (x - m_last_pos.x) + normalize(m_up) * (y - m_last_pos.y);
        m_eye = m_eye + diff * m_motion_sensitivity;
        m_dirty = true;
    }
    else if (m_panning && (x != m_last_pos.x || y != m_last_pos.y))
    {
        Vec3r right = normalize(cross(m_eye - m_target, m_up));
        Vec3r diff = normalize(right) * (x - m_last_pos.x) + normalize(m_up) * (y - m_last_pos.y);
        m_eye    = m_eye    + diff * m_motion_sensitivity;
        m_target = m_target + diff * m_motion_sensitivity;
        m_dirty = true;
    }
    else if (m_zooming && (x != m_last_pos.x || y != m_last_pos.y))
    {
        Vec2r diff = Vec2r(x, y) - m_last_pos;
        Real zoom_sign = (Real)sign(max_abs_component(diff));
        m_eye = m_eye + normalize(m_eye - m_target) * zoom_sign * m_zoom_sensitivity;
        m_dirty = true;
    }
}

} // namespace hop
