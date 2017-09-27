#pragma once

#include "hop.h"
#include "spectrum/spectrum.h"

#include <memory>

namespace hop {

class Film
{
public:
    struct Pixel
    {
        Spectrum color;
        Real variance;
        Real num_samples;
    };

    Film(uint32 w, uint32 h);

    void add_sample(uint32 x, uint32 y, const Spectrum& color);
    void reset_pixel(uint32 x, uint32 y);
    Real get_variance(uint32 x, uint32 y);
    Real get_standard_deviation(uint32 x, uint32 y);

    Pixel* get_pixels() { return m_image.get(); }

private:
    uint32 m_width;
    uint32 m_height;
    std::unique_ptr<Pixel[]> m_image;
};

} // namespace hop
