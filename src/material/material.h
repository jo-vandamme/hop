#pragma once

#include "types.h"

#include <string>

namespace hop {

class Bsdf;
class SurfaceInteraction;

class Material
{
public:
    Material(const std::string& name);

    virtual Bsdf* get_bsdf(const SurfaceInteraction& isect) const;

private:
    std::string m_name;
};

} // namespace hop
