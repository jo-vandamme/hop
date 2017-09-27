#pragma once

#include "math/vec3.h"
#include "geometry/ray.h"
#include "geometry/world.h"
#include "spectrum/spectrum.h"

#include <memory>

namespace hop {

class Integrator
{
public:
    Integrator(std::shared_ptr<World> world);

    virtual Spectrum get_radiance(const Ray& ray) = 0;

    void set_world(std::shared_ptr<World> world);

protected:
    std::shared_ptr<World> m_world;
};

} // namespace hop
