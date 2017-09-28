#include "material/material.h"
#include "bsdf/bsdf.h"
#include "geometry/surface_interaction.h"

#include <string>

namespace hop {

Material::Material(const std::string& name)
    : m_name(name)
{
}

Bsdf* Material::get_bsdf(const SurfaceInteraction& isect) const
{
    (void)isect;
    return nullptr;
}

} // namespace hop
