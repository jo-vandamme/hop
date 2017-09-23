#pragma once

#include "integrators/integrator.h"
#include "geometry/ray.h"

namespace hop {

class PathIntegrator : public Integrator
{
public:
    PathIntegrator(std::shared_ptr<World> world);
    Vec3r get_radiance(const Ray& ray) override;
};

} // namespace hop
