#pragma once

#include "types.h"
#include "math/vec3.h"

namespace hop {

class RGBSpectrum
{
public:
    RGBSpectrum();
    RGBSpectrum(float v);
    RGBSpectrum(const Vec3f& color);
    RGBSpectrum(float r, float g, float b);

    RGBSpectrum clamp(float low = 0.0, float high = 1.0) const;
    bool is_black() const;
    float get_intensity() const;

    Vec3f get_color() const { return m_color; }

    uint32 to_uint32() const;

    RGBSpectrum operator+(const RGBSpectrum& c) const;
    RGBSpectrum operator-(const RGBSpectrum& c) const;
    RGBSpectrum operator*(const RGBSpectrum& c) const;
    RGBSpectrum operator/(const RGBSpectrum& c) const;

    RGBSpectrum operator+(float v) const;
    RGBSpectrum operator-(float v) const;
    RGBSpectrum operator*(float v) const;
    RGBSpectrum operator/(float v) const;

    RGBSpectrum& operator+=(const RGBSpectrum& c);
    RGBSpectrum& operator-=(const RGBSpectrum& c);
    RGBSpectrum& operator*=(const RGBSpectrum& c);
    RGBSpectrum& operator/=(const RGBSpectrum& c);

    RGBSpectrum& operator+=(float v);
    RGBSpectrum& operator-=(float v);
    RGBSpectrum& operator*=(float v);
    RGBSpectrum& operator/=(float v);

private:
    Vec3f m_color;
};

} // namespace hop
