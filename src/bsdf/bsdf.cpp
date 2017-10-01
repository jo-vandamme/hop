#include "bsdf/bsdf.h"
#include "bsdf/bxdf.h"
#include "bsdf/bxdf_types.h"
#include "spectrum/spectrum.h"
#include "geometry/interaction.h"
#include "math/vec2.h"
#include "math/vec3.h"


namespace hop {

Bsdf::Bsdf(const SurfaceInteraction& interaction)
    : ng(interaction.normal)
    , ns(interaction.shading_normal)
    , ts(normalize(interaction.shading_dpdu))
    , ss(cross(ns, ts))
{
}

void Bsdf::add_bxdf(Bxdf* bxdf)
{
    if (m_num_bxdfs == MAX_NUM_BXDFS || bxdf == nullptr)
        return;
    m_bxdfs[m_num_bxdfs++] = bxdf;
}

uint32 Bsdf::num_components(BxdfType type) const
{
    uint32 count = 0;
    for (uint32 i = 0; i < m_num_bxdfs; ++i)
        if (m_bxdfs[i]->matches_flags(type))
            ++count;
    return count;
}

Spectrum Bsdf::f(const Vec3f& wo, const Vec3f& wi, BxdfType type) const
{
    return Spectrum(0);
}

Spectrum Bsdf::sample_f(const Vec3f& wo, Vec3f* wi, const Vec2f& sample, float* pdf,
        BxdfType type, BxdfType* sample_type) const
{
    return Spectrum(0);
}

Vec3f Bsdf::world_to_local(const Vec3f& v) const
{
    return Vec3f(dot(v, ss), dot(v, ts), dot(v, ns));
}

Vec3f Bsdf::local_to_world(const Vec3f& v) const
{
    return Vec3f(v.x * ss.x + v.x * ts.y + v.z * ns.z,
                 v.y * ss.x + v.y * ts.y + v.z * ns.z,
                 v.z * ss.x + v.z * ts.y + v.z * ns.z);
}

} // namespace hop
