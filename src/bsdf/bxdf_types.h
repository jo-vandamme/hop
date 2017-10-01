#pragma once

namespace hop {

enum BxdfType
{
    BXDF_REFLECTION   = 1 << 0,
    BXDF_TRANSMISSION = 1 << 1,
    BXDF_DIFFUSE      = 1 << 2,
    BXDF_GLOSSY       = 1 << 3,
    BXDF_SPECULAR     = 1 << 4,
    BXDF_ALL = BXDF_REFLECTION | BXDF_TRANSMISSION | BXDF_DIFFUSE | BXDF_GLOSSY | BXDF_SPECULAR
};

} // namespace hop
