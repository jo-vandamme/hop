#include "hop.h"
#include "integrators/ambient_occlusion.h"
#include "math/math.h"
#include "math/vec3.h"
#include "geometry/ray.h"
#include "geometry/hit_info.h"
#include "geometry/surface_interaction.h"
#include "geometry/world.h"
#include "integrators/sampling.h"

namespace hop {

AmbientOcclusionIntegrator::AmbientOcclusionIntegrator(std::shared_ptr<World> world)
    : Integrator(world)
{
}

Vec3r AmbientOcclusionIntegrator::get_radiance(const Ray& ray)
{
    SurfaceInteraction isect;
    HitInfo hit;
    if (!m_world->intersect(ray, &hit))
        return Vec3r(0, 0, 0);

    m_world->get_surface_interaction(hit, &isect);
    Vec3r n = normalize(isect.normal);

    Vec3r occlusion;
    Real occlusion_amount = 1.0;
    const Real occlusion_step = 1.0 / (Real)NUM_AO_RAYS;
    for (int i = 0; i < NUM_AO_RAYS; ++i)
    {
        Vec3r random_dir = sample::uniform_sample_hemisphere(random<Real>(), random<Real>());
        if (dot(random_dir, n) < 0)
            random_dir = -random_dir;

        Ray occlusion_ray;
        occlusion_ray.org = isect.position + random_dir * RAY_EPSILON;
        occlusion_ray.dir = random_dir;
        occlusion_ray.tmin = RAY_TMIN;
        occlusion_ray.tmax = RAY_TFAR;
        HitInfo occlusion_hit;
        if (m_world->intersect_any(occlusion_ray, &occlusion_hit))
            occlusion_amount -= occlusion_step;
    }
    occlusion = occlusion_amount * Vec3r(1, 1, 1);

    return occlusion;
}

} // namespace hop
