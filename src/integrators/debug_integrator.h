#pragma once

#include "integrators/integrator.h"
#include "geometry/ray.h"

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
    Vec3r get_radiance(const Ray& ray) override;

private:
    Type m_type;
};

} // namespace hop
