#pragma once

#include "types.h"
#include "math/vec2.h"
#include "render/tonemap.h"

namespace hop {

class RenderOptions
{
public:
    Vec2u frame_size;
    Vec2u tile_size;
    uint32 spp;
    uint32 preview_spp;
    uint32 adaptive_spp;
    uint32 firefly_spp;
    float adaptive_threshold;
    float adaptive_exponent;
    float firefly_threshold;
    ToneMapType tonemap;
    bool preview;
    float ray_epsilon;

    RenderOptions()
        : frame_size(512, 512)
        , tile_size(16, 16)
        , spp(10), preview_spp(1), adaptive_spp(0), firefly_spp(0)
        , adaptive_threshold(1.0), adaptive_exponent(1.0), firefly_threshold(1.0)
        , tonemap(ToneMapType::GAMMA)
        , preview(true)
        , ray_epsilon(1e-4f)
    {
    }
};

} // namespace hop
