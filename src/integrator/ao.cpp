#include "hop.h"
#include "types.h"
#include "integrator/ao.h"
#include "math/math.h"
#include "math/vec3.h"
#include "geometry/ray.h"
#include "geometry/hit_info.h"
#include "geometry/interaction.h"
#include "geometry/world.h"
#include "sampler/sampling.h"

namespace hop {

AmbientOcclusionIntegrator::AmbientOcclusionIntegrator(std::shared_ptr<World> world)
    : Integrator(world)
{
}

Spectrum AmbientOcclusionIntegrator::Li(const Ray& ray) const
{
    SurfaceInteraction isect;
    HitInfo hit;
    if (!m_world->intersect(ray, &hit))
        return AO_BACKGROUND;

    m_world->get_surface_interaction(hit, &isect);
    Vec3f n = isect.normal;

    float occlusion_amount = 1.0f;
    const float occlusion_step = 1.0f / (float)NUM_AO_RAYS;

    for (int i = 0; i < NUM_AO_RAYS; ++i)
    {
        Vec3f random_dir = sample::uniform_sample_hemisphere(random<float>(), random<float>());
        if (dot(random_dir, n) < 0.0f)
            random_dir = -random_dir;

        Ray occlusion_ray;
        occlusion_ray.dir = normalize(Vec3r(random_dir));
        occlusion_ray.org = Vec3r(isect.position) + occlusion_ray.dir * RAY_EPSILON;
        occlusion_ray.tmin = RAY_TMIN;
        occlusion_ray.tmax = RAY_TFAR;
        HitInfo occlusion_hit;
        if (m_world->intersect_any(occlusion_ray, &occlusion_hit))
            occlusion_amount -= occlusion_step;
    }
    return Spectrum(1.0f) * occlusion_amount;
}

} // namespace hop
