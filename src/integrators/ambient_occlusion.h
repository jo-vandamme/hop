#pragma once

#include "integrators/integrator.h"
#include "geometry/ray.h"

namespace hop {

class AmbientOcclusionIntegrator : public Integrator
{
public:
    AmbientOcclusionIntegrator(std::shared_ptr<World> world);

    Vec3r get_radiance(const Ray& ray) override;
};

} // namespace hop
