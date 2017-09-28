#include "integrator/debug.h"
#include "geometry/world.h"
#include "geometry/ray.h"
#include "geometry/hit_info.h"
#include "geometry/surface_interaction.h"

namespace hop {

DebugIntegrator::DebugIntegrator(std::shared_ptr<World> world, Type type)
    : Integrator(world), m_type(type)
{
}

Spectrum DebugIntegrator::Li(const Ray& ray) const
{
    HitInfo hit;
    SurfaceInteraction isect;
    if (!m_world->intersect(ray, &hit))
        return Spectrum(0, 0, 0);

    m_world->get_surface_interaction(hit, &isect);

    if (m_type == POSITION)
        return Spectrum(isect.position);
    else if (m_type == NORMALS)
        return Spectrum(isect.normal);
    else if (m_type == UVS)
        return Spectrum(isect.uv.x, isect.uv.y, 0);

    return Spectrum(1, 0, 0);
}

} // namespace hop