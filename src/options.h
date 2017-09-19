#pragma once

#include "types.h"
#include "math/vec2.h"

namespace hop {

class Options
{
public:
    Vec2u frame_size;
    Vec2u tile_size;
    uint32 tile_spp;
};

} // namespace hop
