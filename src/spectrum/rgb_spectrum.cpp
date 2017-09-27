#include "spectrum/rgb_spectrum.h"
#include "math/vec3.h"

namespace hop {

RGBSpectrum::RGBSpectrum()
    : m_color(0, 0, 0)
{
}

RGBSpectrum::RGBSpectrum(Real v)
    : m_color(v, v, v)
{
}

RGBSpectrum::RGBSpectrum(const Vec3r& color)
    : m_color(color)
{
}

RGBSpectrum::RGBSpectrum(Real r, Real g, Real b)
    : m_color(r, g, b)
{
}

RGBSpectrum RGBSpectrum::clamp(Real low, Real high) const
{
    return RGBSpectrum(max(Vec3r(low), min(Vec3r(high), m_color)));
}

bool RGBSpectrum::is_black() const
{
    if (m_color.x > 0.0)
        return false;
    if (m_color.y > 0.0)
        return false;
    if (m_color.z > 0.0)
        return false;
    return true;
}

Real RGBSpectrum::get_intensity() const
{
    const Vec3r w(0.212671, 0.715160, 0.072169);
    return dot(m_color, w);
}

RGBSpectrum RGBSpectrum::operator+(const RGBSpectrum& c) const
{
    return RGBSpectrum(m_color + c.m_color);
}

RGBSpectrum RGBSpectrum::operator-(const RGBSpectrum& c) const
{
    return RGBSpectrum(m_color - c.m_color);
}

RGBSpectrum RGBSpectrum::operator*(const RGBSpectrum& c) const
{
    return RGBSpectrum(m_color * c.m_color);
}

RGBSpectrum RGBSpectrum::operator/(const RGBSpectrum& c) const
{
    return RGBSpectrum(m_color / c.m_color);
}

RGBSpectrum RGBSpectrum::operator+(Real v) const
{
    return RGBSpectrum(m_color + v);
}

RGBSpectrum RGBSpectrum::operator-(Real v) const
{
    return RGBSpectrum(m_color - v);
}

RGBSpectrum RGBSpectrum::operator*(Real v) const
{
    return RGBSpectrum(m_color * v);
}

RGBSpectrum RGBSpectrum::operator/(Real v) const
{
    return RGBSpectrum(m_color / v);
}

RGBSpectrum& RGBSpectrum::operator+=(const RGBSpectrum& c)
{
    m_color += c.m_color;
    return *this;
}

RGBSpectrum& RGBSpectrum::operator-=(const RGBSpectrum& c)
{
    m_color -= c.m_color;
    return *this;
}

RGBSpectrum& RGBSpectrum::operator*=(const RGBSpectrum& c)
{
    m_color *= c.m_color;
    return *this;
}

RGBSpectrum& RGBSpectrum::operator/=(const RGBSpectrum& c)
{
    m_color /= c.m_color;
    return *this;
}

RGBSpectrum& RGBSpectrum::operator+=(Real v)
{
    m_color += v;
    return *this;
}

RGBSpectrum& RGBSpectrum::operator-=(Real v)
{
    m_color -= v;
    return *this;
}

RGBSpectrum& RGBSpectrum::operator*=(Real v)
{
    m_color *= v;
    return *this;
}

RGBSpectrum& RGBSpectrum::operator/=(Real v)
{
    m_color /= v;
    return *this;
}

} // namespace hop
