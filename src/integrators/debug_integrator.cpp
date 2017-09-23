#include "integrators/debug_integrator.h"
#include "geometry/world.h"
#include "geometry/ray.h"
#include "geometry/hit_info.h"
#include "geometry/surface_interaction.h"

namespace hop {

DebugIntegrator::DebugIntegrator(std::shared_ptr<World> world, Type type)
    : Integrator(world), m_type(type)
{
}

Vec3r DebugIntegrator::get_radiance(const Ray& ray)
{
    HitInfo hit;
    SurfaceInteraction isect;
    if (!m_world->intersect(ray, &hit))
        return Vec3r(0, 0, 0);

    m_world->get_surface_interaction(hit, &isect);

    if (m_type == POSITION)
        return isect.position;
    else if (m_type == NORMALS)
        return isect.normal;
    else if (m_type == UVS)
        return Vec3r(isect.uv.x, isect.uv.y, 0);

    return Vec3r(1, 0, 0);
}

} // namespace hop
