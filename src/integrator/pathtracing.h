#pragma once

#include "integrator/integrator.h"

namespace hop {

class PathIntegrator : public Integrator
{
public:
    PathIntegrator(std::shared_ptr<World> world, float ray_eps);
    Spectrum Li(const Ray& ray) const override;
};

} // namespace hop
