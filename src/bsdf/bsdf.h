#pragma once

namespace hop {

class Bxdf;
class SurfaceInteraction;

class Bsdf
{
public:
    Bsdf(const SurfaceInteraction& interaction);

    //void add_bxdf(Bxdf* bxdf);
};

} // namespace hop
