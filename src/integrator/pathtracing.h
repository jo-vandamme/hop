#pragma once

#include "integrator/integrator.h"
#include "geometry/ray.h"
#include "spectrum/spectrum.h"

namespace hop {

class PathIntegrator : public Integrator
{
public:
    PathIntegrator(std::shared_ptr<World> world);
    Spectrum Li(const Ray& ray) const override;
};

} // namespace hop
