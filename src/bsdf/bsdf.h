#pragma once

#include "types.h"
#include "bsdf/bxdf_types.h"
#include "spectrum/spectrum.h"
#include "math/vec2.h"
#include "math/vec3.h"

namespace hop {

class Bxdf;
class SurfaceInteraction;

class Bsdf
{
public:
    Bsdf(const SurfaceInteraction& interaction);

    void add_bxdf(Bxdf* bxdf);

    uint32 num_components(BxdfType type = BXDF_ALL) const;

    Spectrum f(const Vec3f& wo, const Vec3f& wi, BxdfType type = BXDF_ALL) const;

    Spectrum sample_f(const Vec3f& wo, Vec3f* wi, const Vec2f& sample, float* pdf,
            BxdfType type = BXDF_ALL, BxdfType* sample_type = nullptr) const;

    Vec3f world_to_local(const Vec3f& v) const;
    Vec3f local_to_world(const Vec3f& v) const;

private:
    static constexpr uint32 MAX_NUM_BXDFS = 8;

    uint32 m_num_bxdfs = 0;
    Bxdf* m_bxdfs[MAX_NUM_BXDFS];

    // Geometric normal
    const Vec3f ng;

    // Shading normal, tangent and bi-tangent
    const Vec3f ns;
    const Vec3f ts;
    const Vec3f ss;
};

} // namespace hop
