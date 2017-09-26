#include "hop.h"
#include "integrators/path_integrator.h"
#include "math/math.h"
#include "math/vec3.h"
#include "geometry/ray.h"
#include "geometry/hit_info.h"
#include "geometry/surface_interaction.h"
#include "geometry/world.h"
#include "integrators/sampling.h"

namespace hop {

PathIntegrator::PathIntegrator(std::shared_ptr<World> world)
    : Integrator(world)
{
}

Vec3r PathIntegrator::get_radiance(const Ray& r)
{
    Vec3r rad(0, 0, 0);
    Vec3r throughput(1, 1, 1);
    uint32 depth = 0;

    Ray ray = r;
    while (1)
    {
        HitInfo hit;
        if (!m_world->intersect(ray, &hit))
        {
            rad += throughput * Vec3r(1.0, 1.0, 1.0); // sample sky
            break;
        }
        SurfaceInteraction isect;
        m_world->get_surface_interaction(hit, &isect);
        Vec3r n = normalize(isect.normal);

        Vec3r brdf;
        Real pdf;

        uint32 mat = 2;
        if (mat == 0)
        {
            // ray, brdf, pdf = random_sample(hit)
            ray.dir = (sample::cosine_sample_hemisphere(random<Real>(), random<Real>()));
            if (dot(ray.dir, n) < 0)
                ray.dir = -ray.dir;
            brdf = Vec3r(0.25, 0.65, 0.85) * (Real)one_over_pi;
            pdf = dot(n, ray.dir) * (Real)one_over_pi;
        }
        else if (mat == 1)
        {
            ray.dir = reflect(ray.dir, n);
            if (dot(ray.dir, n) < 0)
                ray.dir = -ray.dir;
            brdf = Vec3r(0.25, 0.65, 0.85) * rcp(dot(ray.dir, n));
            pdf = 1.0;
        }
        else
        {
            Real dot_n_dir = dot(n, ray.dir);
            Real n1_over_n2 = dot_n_dir < 0 ? 1.0 * rcp(1.33) : 1.33 * rcp(1.0);
            ray.dir = refract(ray.dir, n, n1_over_n2);
            //brdf = Vec3r(0.25, 0.65, 0.85) * rcp(dot(ray.dir, n));
            brdf = Vec3r(0.80, 0.80, 1.00) * rcp(dot(ray.dir, n));
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
