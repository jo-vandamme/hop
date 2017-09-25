#include "render/film.h"
#include "math/math.h"
#include "math/vec3.h"

#include <memory>
#include <cstring>
#include <utility>

namespace hop {

Film::Film(uint32 w, uint32 h)
    : m_width(w), m_height(h)
    , m_image(std::make_unique<Pixel[]>(w * h))
{
    std::memset(&m_image[0], 0, sizeof(Pixel) * m_width * m_height);
}

void Film::add_sample(uint32 x, uint32 y, const Vec3r& color)
{
    uint32 idx = y * m_width + x;
    Real n = ++m_image[idx].num_samples;
    Real rcp_n = rcp(n);

    if (n > 2)
        m_image[idx].variance = m_image[idx].variance * ((n - 2.0) * rcp(n - 1.0)) +
                                (sqr(color - m_image[idx].color) * rcp_n);

    m_image[idx].color += (color - m_image[idx].color) * rcp_n;
}

void Film::reset_pixel(uint32 x, uint32 y)
{
    uint32 idx = y * m_width + x;
    m_image[idx].num_samples = 0.0;
    m_image[idx].color = Vec3r(0, 0, 0);
    m_image[idx].variance = Vec3r(0, 0, 0);
}

Vec3r Film::get_variance(uint32 x, uint32 y)
{
    return m_image[y * m_width + x].variance;
}

Vec3r Film::get_standard_deviation(uint32 x, uint32 y)
{
    return sqrt(m_image[y * m_width + x].variance);
}

} // namespace hop
