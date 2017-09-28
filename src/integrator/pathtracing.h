#pragma once

#include "integrator/integrator.h"

namespace hop {

class PathIntegrator : public Integrator
{
public:
    PathIntegrator(std::shared_ptr<World> world);
    Spectrum Li(const Ray& ray) const override;
};

} // namespace hop
