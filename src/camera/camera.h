#pragma once

#include "types.h"
#include "math/vec3.h"
#include "math/transform.h"
#include "util/log.h"

#include <memory>

namespace hop {

class Ray;
class CameraSample;

class Camera
{
public:
    Camera(const Vec3r& eye, const Vec3r& target, const Vec3r& up);
    virtual ~Camera() { Log("camera") << DEBUG << "camera deleted"; }

    virtual Real generate_ray(const CameraSample& sample, Ray* ray) = 0;

    Transformr get_transform() const { return m_cam2world; }

    Vec3r get_eye() const { return m_eye; }
    Vec3r get_target() const { return m_target; }
    Vec3r get_up() const { return m_up; }

    void set_eye(const Vec3r& eye);
    void set_target(const Vec3r& target);
    void set_up(const Vec3r& up);
    void update_transform();

protected:
    Transformr m_cam2world;
    Vec3r m_eye;
    Vec3r m_target;
    Vec3r m_up;
};

} // namespace hop
