#include "spectrum/rgb_spectrum.h"
#include "math/vec3.h"

namespace hop {

RGBSpectrum::RGBSpectrum()
    : m_color(0.0f, 0.0f, 0.0f)
{
}

RGBSpectrum::RGBSpectrum(float v)
    : m_color(v, v, v)
{
}

RGBSpectrum::RGBSpectrum(const Vec3f& color)
    : m_color(color)
{
}

RGBSpectrum::RGBSpectrum(float r, float g, float b)
    : m_color(r, g, b)
{
}

RGBSpectrum RGBSpectrum::clamp(float low, float high) const
{
    return RGBSpectrum(max(Vec3f(low), min(Vec3f(high), m_color)));
}

bool RGBSpectrum::is_black() const
{
    if (m_color.x > 0.0f)
        return false;
    if (m_color.y > 0.0f)
        return false;
    if (m_color.z > 0.0f)
        return false;
    return true;
}

float RGBSpectrum::get_intensity() const
{
    const Vec3f w(0.212671f, 0.715160f, 0.072169f);
    return dot(m_color, w);
}

uint32 RGBSpectrum::to_uint32() const
{
    Vec3f col = clamp(0.0f, 1.0f).m_color * float(255.0f);
    return (uint8(col.z) << 16) | (uint8(col.y) << 8) | uint8(col.x);
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

RGBSpectrum RGBSpectrum::operator+(float v) const
{
    return RGBSpectrum(m_color + v);
}

RGBSpectrum RGBSpectrum::operator-(float v) const
{
    return RGBSpectrum(m_color - v);
}

RGBSpectrum RGBSpectrum::operator*(float v) const
{
    return RGBSpectrum(m_color * v);
}

RGBSpectrum RGBSpectrum::operator/(float v) const
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

RGBSpectrum& RGBSpectrum::operator+=(float v)
{
    m_color += v;
    return *this;
}

RGBSpectrum& RGBSpectrum::operator-=(float v)
{
    m_color -= v;
    return *this;
}

RGBSpectrum& RGBSpectrum::operator*=(float v)
{
    m_color *= v;
    return *this;
}

RGBSpectrum& RGBSpectrum::operator/=(float v)
{
    m_color /= v;
    return *this;
}

} // namespace hop
