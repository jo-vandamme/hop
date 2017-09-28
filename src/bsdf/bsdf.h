#pragma once

#include "geometry/surface_interaction.h"

namespace hop {

class Bxdf;

class Bsdf
{
public:
    Bsdf(const SurfaceInteraction& intersection_info);

    //void add_bxdf(Bxdf* bxdf);

protected:
    const SurfaceInteraction m_intersection_info;
};

} // namespace hop
