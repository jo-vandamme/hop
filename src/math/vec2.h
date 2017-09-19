#pragma once

#include "hop.h"
#include "types.h"
#include "math/math.h"
#include <ostream>

namespace hop {

template <typename T>
class Vec2
{
public:
    T x, y;

    Vec2() : x(T(0)), y(T(0)) { }
    Vec2(const T& v) : x(v), y(v) { }
    Vec2(const T& x, const T& y) : x(x), y(y) { }

    const T& operator[](uint8 i) const { return (&x)[i]; }
    T& operator[](uint8 i) { return (&x)[i]; }

    inline T length2() const
    {
        return madd(x, x, y * y);
    }

    inline T length() const
    {
        return sqrt(length2());
    }

    inline Vec2<T>& normalize()
    {
        T len2 = length2();
        if (len2 > 0)
        {
            T inv_len = rsqrt(len2);
            x *= inv_len; y *= inv_len;
        }
        return *this;
    }
};

typedef Vec2<double> Vec2d;
typedef Vec2<float> Vec2f;
typedef Vec2<Real> Vec2r;
typedef Vec2<uint32> Vec2u;

template <typename T>
inline Vec2<T> operator-(const Vec2<T>& v)
{
    return Vec2<T>(-v.x, -v.y);
}

template <typename T>
inline Vec2<T> operator-(const Vec2<T>& a, const Vec2<T>& b)
{
    return Vec2<T>(a.x - b.x, a.y - b.y);
}

template <typename T>
inline Vec2<T> operator+(const Vec2<T>& a, const Vec2<T>& b)
{
    return Vec2<T>(a.x + b.x, a.y + b.y);
}

template <typename T>
inline Vec2<T> operator*(const Vec2<T>& a, const Vec2<T>& b)
{
    return Vec2<T>(a.x * b.x, a.y * b.y);
}

template <typename T>
inline Vec2<T> operator*(const Vec2<T>& v, const T f)
{
    return Vec2<T>(v.x * f, v.y * f);
}

template <typename T>
inline Vec2<T> operator*(const T f, const Vec2<T>& v)
{
    return Vec2<T>(f * v.x, f * v.y);
}

template <typename T>
inline Vec2<T> min(const Vec2<T>& a, const Vec2<T>& b)
{
    return Vec2<T>(min(a.x, b.x), min(a.y, b.y));
}

template <typename T>
inline Vec2<T> max(const Vec2<T>& a, const Vec2<T>& b)
{
    return Vec2<T>(max(a.x, b.x), max(a.y, b.y));
}

template <typename T>
inline Vec2<T> sqrt(const Vec2<T>& a)
{
    return Vec2<T>(sqrt(a.x), sqrt(a.y));
}

template <typename T>
inline Vec2<T> rsqrt(const Vec2<T>& a)
{
    return Vec2<T>(rsqrt(a.x), rsqrt(a.y));
}

template <typename T>
inline T dot(const Vec2<T>& a, const Vec2<T>& b)
{
    return madd(a.x, b.x, a.y * b.y);
}

template <typename T>
inline T length2(const Vec2<T>& v)
{
    return madd(v.x, v.x, v.y * v.y);
}

template <typename T>
inline T length(const Vec2<T>& v)
{
    return sqrt(length2(v));
}

template <typename T>
inline Vec2<T> normalize(const Vec2<T>& v)
{
    Vec2<T> res(v);
    T len2 = length2(v);
    if (len2 > 0)
    {
        T inv_len = rsqrt(len2);
        res.x *= inv_len; res.y *= inv_len;
    }
    return res;
}

template <typename T>
inline T max_component(const Vec2<T>& v)
{
    T out = v.x;
    if (v.y > v.x) out = v.y;
    return out;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const Vec2<T>& v)
{
    os << "(" << v.x << " " << v.y << ")";
    return os;
}

} // namespace hop
