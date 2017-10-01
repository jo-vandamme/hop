#pragma once

#include "bsdf/bxdf_types.h"
#include "spectrum/spectrum.h"
#include "math/vec2.h"
#include "math/vec3.h"

namespace hop {

class Bxdf
{
public:
    Bxdf(BxdfType type);

    virtual Spectrum f(const Vec3f& wo, const Vec3f& wi, BxdfType type = BXDF_ALL) const = 0;

    virtual Spectrum sample_f(const Vec3f& wo, Vec3f* wi, const Vec2f& sample, float* pdf,
                              BxdfType* sample_type = nullptr) const;

    bool matches_flags(BxdfType type) const
    {
        return (m_type & type) == m_type;
    }

private:
    BxdfType m_type;
};

} // namespace hop
