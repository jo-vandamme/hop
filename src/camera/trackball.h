#pragma once

#include "types.h"
#include "camera/camera.h"
#include "math/transform.h"
#include "math/vec2.h"

#include <memory>

namespace hop {

class Renderer;

class TrackBall
{
public:
    TrackBall(std::shared_ptr<Camera> camera, Renderer* renderer);
    ~TrackBall();

    void update(float time);

    void on_button_down(int button);
    void on_button_up(int button);
    void on_motion(double x, double y);

    void reset();

private:
    std::shared_ptr<Camera> m_camera;
    Renderer* m_renderer;

    Vec3r m_orig_eye, m_orig_target, m_orig_up;
    Vec3r m_eye, m_target, m_up;

    Vec2r m_last_pos;
    Real m_motion_sensitivity;
    Real m_zoom_sensitivity;

    bool m_update_last_pos;
    bool m_dirty;
    bool m_rotating;
    bool m_panning;
    bool m_zooming;
};

} // namespace hop
