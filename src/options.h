#pragma once

#include "types.h"
#include "math/vec2.h"
#include "render/tonemap.h"

namespace hop {

class Options
{
public:
    Vec2u frame_size;
    Vec2u tile_size;
    uint32 spp;
    uint32 preview_spp;
    uint32 adaptive_spp;
    uint32 firefly_spp;
    Real adaptive_threshold;
    Real adaptive_exponent;
    Real firefly_threshold;
    ToneMapType tonemap;
    bool preview;
};

} // namespace hop
