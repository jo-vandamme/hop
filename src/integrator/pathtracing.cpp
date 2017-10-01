#include "hop.h"
#include "types.h"
#include "integrator/pathtracing.h"
#include "math/math.h"
#include "math/vec3.h"
#include "geometry/ray.h"
#include "geometry/hit_info.h"
#include "geometry/interaction.h"
#include "geometry/world.h"
#include "sampler/sampling.h"
#include "spectrum/spectrum.h"

namespace hop {

PathIntegrator::PathIntegrator(std::shared_ptr<World> world, float ray_eps)
    : Integrator(world, ray_eps)
{
}

static float fresnel(float costheta1, float costheta2, float n1, float n2)
{
    float rp = (n2 * costheta1 - n1 * costheta2) * rcp(n2 * costheta1 + n1 * costheta2);
    float rs = (n1 * costheta1 - n2 * costheta2) * rcp(n1 * costheta1 + n2 * costheta2);
    return (sqr(rp) + sqr(rs)) * 0.5f;
}

Spectrum PathIntegrator::Li(const Ray& r) const
{
    Spectrum rad(0.0f);
    Spectrum throughput(1.0f);
    uint32 depth = 0;

    Ray ray = r;
    while (1)
    {
        HitInfo hit;
        if (!m_world->intersect(ray, &hit))
        {
            rad += throughput * Spectrum(1.0f); // sample sky
            break;
        }
        SurfaceInteraction isect;
        m_world->get_surface_interaction(hit, &isect);
        Vec3f n = isect.normal;

        Spectrum brdf;
        float pdf;

        uint32 mat = 0;
        Vec3f ray_dir = Vec3f(ray.dir);
        if (mat == 0)
        {
            ray_dir = sample::cosine_sample_hemisphere(random<float>(), random<float>());
            if (dot(ray_dir, n) < 0.0f)
                ray_dir = -ray_dir;
            brdf = Spectrum(0.9f, 0.7f, 0.4f) * (float)one_over_pi;
            pdf = dot(n, ray_dir) * (float)one_over_pi;
        }
        else if (mat == 1)
        {
            ray_dir = reflect(ray_dir, n);
            if (dot(ray_dir, n) < 0.0f)
                ray_dir = -ray_dir;
            brdf = Spectrum(0.25f, 0.65f, 0.85f) * rcp(dot(ray_dir, n));
            pdf = 1.0f;
        }
        else
        {
            float n_dot_dir = dot(n, ray_dir);
            float n1 = n_dot_dir < 0.0f ? 1.0f : 1.5f;
            float n2 = n_dot_dir < 0.0f ? 1.5f : 1.0f;
            Vec3f refract_dir = refract(ray_dir, n, n1 * rcp(n2));
            float fresnel_coeff = fresnel(abs(n_dot_dir), abs(dot(refract_dir, n)), n1, n2);
            if (random<float>() < fresnel_coeff)
            {
                ray_dir = reflect(ray_dir, n);
                if (n_dot_dir > 0.0f && dot(ray_dir, n) < 0.0f)
                    ray_dir = -ray_dir;
            }
            else
            {
                ray_dir = refract_dir;
            }
            brdf = Spectrum(0.8f, 0.8f, 1.0f) * rcp(dot(ray_dir, n));
            pdf = 1.0f;
        }

        ray.dir = normalize(Vec3r(ray_dir));
        ray.org = isect.position;
        ray.tmin = m_ray_epsilon;
        ray.tmax = RAY_TFAR;

        throughput *= brdf * dot(n, ray_dir) * rcp(pdf);

        // Russian roulette
        ++depth;
        if (depth > 3)
        {
            const float absorption = 0.2f;
            if (random<float>() > (1.0f - absorption))
                break;
            throughput *= rcp(1.0f - absorption);
        }
    }

    return rad;
}

} // namespace hop
