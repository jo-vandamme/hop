#pragma once

#include "geometry/world.h"
#include "spectrum/spectrum.h"

#include <memory>

namespace hop {

class Ray;

class Integrator
{
public:
    Integrator(std::shared_ptr<World> world, float ray_epsilon);

    // Sample the incident radiance along a ray.
    virtual Spectrum Li(const Ray& ray) const = 0;

    void set_world(std::shared_ptr<World> world);

protected:
    std::shared_ptr<World> m_world;
    float m_ray_epsilon;
};

} // namespace hop
