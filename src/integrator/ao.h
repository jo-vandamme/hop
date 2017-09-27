#pragma once

#include "integrator/integrator.h"
#include "geometry/ray.h"
#include "spectrum/spectrum.h"

namespace hop {

class AmbientOcclusionIntegrator : public Integrator
{
public:
    AmbientOcclusionIntegrator(std::shared_ptr<World> world);
    Spectrum Li(const Ray& ray) const override;
};

} // namespace hop
