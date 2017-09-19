#pragma once

#include "hop.h"
#include "types.h"
#include "math/math.h"
#include <ostream>

namespace hop {

template <typename T>
class Vec3
{
public:
    T x, y, z;

    Vec3() : x(T(0)), y(T(0)), z(T(0)) { }
    Vec3(const T& v) : x(v), y(v), z(v) { }
    Vec3(const T& x, const T& y, const T& z) : x(x), y(y), z(z) { }

    const T& operator[](uint8 i) const { return (&x)[i]; }
    T& operator[](uint8 i) { return (&x)[i]; }

    inline T length2() const
    {
        return madd(x, x, madd(y, y, z * z));
    }

    inline T length() const
    {
        return sqrt(length2());
    }

    inline Vec3<T>& normalize()
    {
        T len2 = length2();
        if (len2 > 0)
        {
            T inv_len = rsqrt(len2);
            x *= inv_len; y *= inv_len;; z *= inv_len;
        }
        return *this;
    }
};

typedef Vec3<double> Vec3d;
typedef Vec3<float> Vec3f;
typedef Vec3<Real> Vec3r;
typedef Vec3<int> Vec3i;

template <typename T>
inline Vec3<T> operator-(const Vec3<T>& v)
{
    return Vec3<T>(-v.x, -v.y, -v.z);
}

template <typename T>
inline Vec3<T> operator-(const Vec3<T>& a, const Vec3<T>& b)
{
    return Vec3<T>(a.x - b.x, a.y - b.y, a.z - b.z);
}

template <typename T>
inline Vec3<T> operator+(const Vec3<T>& a, const Vec3<T>& b)
{
    return Vec3<T>(a.x + b.x, a.y + b.y, a.z + b.z);
}

template <typename T>
inline Vec3<T> operator*(const Vec3<T>& a, const Vec3<T>& b)
{
    return Vec3<T>(a.x * b.x, a.y * b.y, a.z * b.z);
}

template <typename T>
inline Vec3<T> operator*(const Vec3<T>& v, const T f)
{
    return Vec3<T>(v.x * f, v.y * f, v.z * f);
}

template <typename T>
inline Vec3<T> operator*(const T f, const Vec3<T>& v)
{
    return Vec3<T>(f * v.x, f * v.y, f * v.z);
}

template <typename T>
inline Vec3<T> min(const Vec3<T>& a, const Vec3<T>& b)
{
    return Vec3<T>(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z));
}

template <typename T>
inline Vec3<T> max(const Vec3<T>& a, const Vec3<T>& b)
{
    return Vec3<T>(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z));
}

template <typename T>
inline Vec3<T> rcp(const Vec3<T>& v)
{
    return Vec3<T>(rcp(v.x), rcp(v.y), rcp(v.z));
}

template <typename T>
inline Vec3<T> sqrt(const Vec3<T>& a)
{
    return Vec3<T>(sqrt(a.x), sqrt(a.y), sqrt(a.z));
}

template <typename T>
inline Vec3<T> rsqrt(const Vec3<T>& a)
{
    return Vec3<T>(rsqrt(a.x), rsqrt(a.y), rsqrt(a.z));
}

template <typename T>
inline Vec3<T> cross(const Vec3<T>& a, const Vec3<T>& b)
{
    return Vec3<T>(msub(a.y, b.z, a.z * b.y), msub(a.z, b.x, a.x * b.z), msub(a.x, b.y, a.y * b.x));
}

template <typename T>
inline T dot(const Vec3<T>& a, const Vec3<T>& b)
{
    return madd(a.x, b.x, madd(a.y, b.y, a.z * b.z));
}

template <typename T>
inline T length2(const Vec3<T>& v)
{
    return madd(v.x, v.x, madd(v.y, v.y, v.z * v.z));
}

template <typename T>
inline T length(const Vec3<T>& v)
{
    return sqrt(length2(v));
}

template <typename T>
inline Vec3<T> normalize(const Vec3<T>& v)
{
    Vec3<T> res(v);
    T len2 = length2(v);
    if (len2 > 0)
    {
        T inv_len = rsqrt(len2);
        res.x *= inv_len; res.y *= inv_len; res.z *= inv_len;
    }
    return res;
}

template <typename T>
inline T max(const Vec3<T>& v)
{
    return max(max(v.x, v.y), v.z);
}

template <typename T>
inline T min(const Vec3<T>& v)
{
    return min(min(v.x, v.y), v.z);
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const Vec3<T>& v)
{
    os << "(" << v.x << " " << v.y << " " << v.z << ")";
    return os;
}

} // namespace hop
