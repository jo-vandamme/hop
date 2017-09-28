#include "bsdf/bsdf.h"
#include "geometry/surface_interaction.h"

namespace hop {

Bsdf::Bsdf(const SurfaceInteraction& intersection_info)
    : m_intersection_info(intersection_info)
{
}

} // namespace hop
