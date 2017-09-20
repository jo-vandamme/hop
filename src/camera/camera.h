#pragma once

#include "hop.h"
#include "math/transform.h"
#include "math/vec3.h"
#include "util/log.h"

#include <memory>

namespace hop {

class Ray;
class CameraSample;

class Camera
{
public:
    Camera(const Transformr& cam2world);
    virtual ~Camera() { Log("camera") << DEBUG << "camera deleted"; }

    Transformr get_transform() const { return m_cam2world; }
    void set_transform(const Transformr& xfm);

    void rotate(const Vec3r& axis, Real angle);
    void translate(const Vec3r& delta);

    virtual Real generate_ray(const CameraSample& sample, Ray* ray) = 0;

protected:
    Transformr m_cam2world;
};

} // namespace hop
