#pragma once

#include "integrator/integrator.h"
#include "geometry/ray.h"
#include "sampler/sampling.h"

namespace hop {

class DebugIntegrator : public Integrator
{
public:
    enum Type
    {
        POSITION,
        NORMALS,
        UVS
    };

    DebugIntegrator(std::shared_ptr<World> world, Type type);
    Spectrum get_radiance(const Ray& ray) override;

private:
    Type m_type;
};

} // namespace hop
