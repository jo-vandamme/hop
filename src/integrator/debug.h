#pragma once

#include "integrator/integrator.h"

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

    DebugIntegrator(std::shared_ptr<World> world, float ray_eps, Type type);
    Spectrum Li(const Ray& ray) const override;

private:
    Type m_type;
};

} // namespace hop
