#pragma once

#include "integrator/integrator.h"

namespace hop {

class AmbientOcclusionIntegrator : public Integrator
{
public:
    AmbientOcclusionIntegrator(std::shared_ptr<World> world, float ray_eps);
    Spectrum Li(const Ray& ray) const override;
};

} // namespace hop
