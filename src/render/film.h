#pragma once

#include "hop.h"
#include "math/vec3.h"

#include <memory>

namespace hop {

class Film
{
public:
    struct Pixel
    {
        Vec3r color;
        Real num_samples;
    };

    Film(uint32 w, uint32 h);

    void add_sample(uint32 x, uint32 y, const Vec3r& color);
    void reset_pixel(uint32 x, uint32 y);

    Pixel* get_pixels() { return &m_image[0]; }

private:
    uint32 m_width;
    uint32 m_height;
    std::unique_ptr<Pixel[]> m_image;
};

} // namespace hop
