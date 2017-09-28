#pragma once

#include "types.h"
#include "math/vec3.h"

namespace hop {

class RGBSpectrum
{
public:
    RGBSpectrum();
    RGBSpectrum(Real v);
    RGBSpectrum(const Vec3r& color);
    RGBSpectrum(Real r, Real g, Real b);

    RGBSpectrum clamp(Real low = 0.0, Real high = 1.0) const;
    bool is_black() const;
    Real get_intensity() const;

    Vec3r get_color() const { return m_color; }

    uint32 to_uint32() const;

    RGBSpectrum operator+(const RGBSpectrum& c) const;
    RGBSpectrum operator-(const RGBSpectrum& c) const;
    RGBSpectrum operator*(const RGBSpectrum& c) const;
    RGBSpectrum operator/(const RGBSpectrum& c) const;

    RGBSpectrum operator+(Real v) const;
    RGBSpectrum operator-(Real v) const;
    RGBSpectrum operator*(Real v) const;
    RGBSpectrum operator/(Real v) const;

    RGBSpectrum& operator+=(const RGBSpectrum& c);
    RGBSpectrum& operator-=(const RGBSpectrum& c);
    RGBSpectrum& operator*=(const RGBSpectrum& c);
    RGBSpectrum& operator/=(const RGBSpectrum& c);

    RGBSpectrum& operator+=(Real v);
    RGBSpectrum& operator-=(Real v);
    RGBSpectrum& operator*=(Real v);
    RGBSpectrum& operator/=(Real v);

private:
    Vec3r m_color;
};

} // namespace hop
