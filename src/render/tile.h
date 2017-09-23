#pragma once

#include "types.h"

#include <vector>

namespace hop {

struct Tile
{
    uint32 x, y, w, h, n;
};

std::vector<Tile> make_tiles_linear(int frame_width, int frame_height, int tile_width, int tile_height);
std::vector<Tile> make_tiles_spiral(int frame_width, int frame_height, int tile_width, int tile_height);

} // namespace hop
