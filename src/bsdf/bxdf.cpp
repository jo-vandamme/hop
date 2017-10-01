#include "bsdf/bxdf.h"
#include "bsdf/bxdf_types.h"
#include "spectrum/spectrum.h"
#include "math/vec2.h"
#include "math/vec3.h"

namespace hop {

Bxdf::Bxdf(BxdfType type)
    : m_type(type)
{
}

Spectrum Bxdf::sample_f(const Vec3f& wo, Vec3f* wi, const Vec2f& sample, float* pdf,
        BxdfType* sample_type) const
{
    return Spectrum(0.0f);
}

} // namespace hop
