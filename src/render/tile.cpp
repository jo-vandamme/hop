#include "render/tile.h"
#include "math/math.h"

#include <vector>

namespace hop {

std::vector<Tile> make_tiles_spiral(int fw, int fh, int tw, int th)
{
    std::vector<Tile> tiles;

    // This function gets a tile coordinate and calculates the
    // corresponding tile, if the tile is non-empty, it pushed into the list
    auto make_tile = [&](int i, int j)
    {
        int x = ((float)i - 0.5f) * tw + (float)fw / 2.0f;
        int y = ((float)j - 0.5f) * th + (float)fh / 2.0f;

        if (x + tw >= 0 && x < fw && y + th >= 0 && y < fh)
        {
            Tile tile;
            tile.x = max(0, x);
            tile.y = max(0, y);
            tile.w = min(fw - x, tw);
            tile.h = min(fh - y, th);
            tile.n = 0;

            tiles.push_back(tile);
        }
    };

    int X = fw / tw + 2;
    int Y = fh / th + 2;
    int x, y, dx, dy;
    x = y = dx = 0;
    dy = -1;

    int t = max(X, Y);
    int max_iters = t * t;

    for (int i = 0; i < max_iters; ++i)
    {
        make_tile(x, y);

        if ((x == y) || ((x < 0) && (x == -y)) || ((x > 0) && (x == 1-y)))
        {
            t = dx;
            dx = -dy;
            dy = t;
        }
        x += dx;
        y += dy;
    }

    return tiles;
}

std::vector<Tile> make_tiles_linear(int fw, int fh, int tw, int th)
{
    std::vector<Tile> tiles;

    for (uint32 y = 0; y < (uint32)fh; y += th)
    {
        Tile tile;
        tile.y = y;
        tile.h = min(fh - y, (uint32)th);
        tile.n = 0;

        for (uint32 x = 0; x < (uint32)fw; x += tw)
        {
            tile.x = x;
            tile.w = min(fw - x, (uint32)tw);
            tiles.push_back(tile);
        }
    }
    return tiles;
}

} // namespace hop
