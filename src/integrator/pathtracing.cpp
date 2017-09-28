#include "hop.h"
#include "types.h"
#include "integrator/pathtracing.h"
#include "math/math.h"
#include "math/vec3.h"
#include "geometry/ray.h"
#include "geometry/hit_info.h"
#include "geometry/surface_interaction.h"
#include "geometry/world.h"
#include "sampler/sampling.h"
#include "spectrum/spectrum.h"

namespace hop {

PathIntegrator::PathIntegrator(std::shared_ptr<World> world)
    : Integrator(world)
{
}

static Real fresnel(Real costheta1, Real costheta2, Real n1, Real n2)
{
    Real rp = (n2 * costheta1 - n1 * costheta2) * rcp(n2 * costheta1 + n1 * costheta2);
    Real rs = (n1 * costheta1 - n2 * costheta2) * rcp(n1 * costheta1 + n2 * costheta2);
    return (sqr(rp) + sqr(rs)) * 0.5;
}

Spectrum PathIntegrator::Li(const Ray& r) const
{
    Spectrum rad(0);
    Spectrum throughput(1);
    uint32 depth = 0;

    Ray ray = r;
    while (1)
    {
        HitInfo hit;
        if (!m_world->intersect(ray, &hit))
        {
            rad += throughput * Spectrum(1); // sample sky
            break;
        }
        SurfaceInteraction isect;
        m_world->get_surface_interaction(hit, &isect);
        Vec3r n = normalize(isect.normal);

        Spectrum brdf;
        Real pdf;

        uint32 mat = 0;
        if (mat == 0)
        {
            // ray, brdf, pdf = random_sample(hit)
            ray.dir = (sample::cosine_sample_hemisphere(random<Real>(), random<Real>()));
            if (dot(ray.dir, n) < 0)
                ray.dir = -ray.dir;
            brdf = Spectrum(0.9, 0.7, 0.4) * (Real)one_over_pi;
            pdf = dot(n, ray.dir) * (Real)one_over_pi;
        }
        else if (mat == 1)
        {
            ray.dir = reflect(ray.dir, n);
            if (dot(ray.dir, n) < 0)
                ray.dir = -ray.dir;
            brdf = Spectrum(0.25, 0.65, 0.85) * rcp(dot(ray.dir, n));
            pdf = 1.0;
        }
        else
        {
            Real n_dot_dir = dot(n, ray.dir);
            Real n1 = n_dot_dir < 0 ? 1.0  : 1.5;
            Real n2 = n_dot_dir < 0 ? 1.5 : 1.0;
            Vec3r refract_dir = refract(ray.dir, n, n1 * rcp(n2));
            Real fresnel_coeff = fresnel(abs(n_dot_dir), abs(dot(refract_dir, n)), n1, n2);
            Real p = random<Real>();
            if (p < fresnel_coeff)
            {
                ray.dir = reflect(ray.dir, n);
                if (n_dot_dir > 0 && dot(ray.dir, n) < 0)
                    ray.dir = -ray.dir;
            }
            else
            {
                ray.dir = refract_dir;
            }
            brdf = Spectrum(0.8, 0.8, 0.8) * rcp(dot(ray.dir, n));
            pdf = 1.0;
        }

        ray.org = isect.position + ray.dir * RAY_EPSILON;
        ray.tmin = RAY_TMIN;
        ray.tmax = RAY_TFAR;

        throughput *= brdf * dot(n, ray.dir) * rcp(pdf);

        // Russian roulette
        ++depth;
        if (depth > 3)
        {
            const Real absorption = 0.2;
            if (random<Real>() > (1.0 - absorption))
                break;
            throughput *= rcp(1.0 - absorption);
        }
    }

    return rad;
}

} // namespace hop
