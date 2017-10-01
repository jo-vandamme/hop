#include "integrator/debug.h"
#include "geometry/world.h"
#include "geometry/ray.h"
#include "geometry/hit_info.h"
#include "geometry/interaction.h"

namespace hop {

DebugIntegrator::DebugIntegrator(std::shared_ptr<World> world, float ray_eps, Type type)
    : Integrator(world, ray_eps), m_type(type)
{
}

Spectrum DebugIntegrator::Li(const Ray& ray) const
{
    HitInfo hit;
    SurfaceInteraction isect;
    if (!m_world->intersect(ray, &hit))
        return Spectrum(0.0f, 0.0f, 0.0f);

    m_world->get_surface_interaction(hit, &isect);

    if (m_type == POSITION)
        return Spectrum(Vec3f(isect.position));
    else if (m_type == NORMALS)
        return Spectrum(isect.normal);
    else if (m_type == UVS)
        return Spectrum(isect.uv.x, isect.uv.y, 0.0f);

    return Spectrum(1.0f, 0.0f, 0.0f);
}

} // namespace hop
