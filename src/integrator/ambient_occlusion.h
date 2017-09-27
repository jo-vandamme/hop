#pragma once

#include "integrator/integrator.h"
#include "geometry/ray.h"
#include "spectrum/spectrum.h"

namespace hop {

class AmbientOcclusionIntegrator : public Integrator
{
public:
    AmbientOcclusionIntegrator(std::shared_ptr<World> world);
    Spectrum get_radiance(const Ray& ray) override;
};

} // namespace hop
