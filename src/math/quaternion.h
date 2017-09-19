#pragma once

#include "hop.h"
#include "math/math.h"
#include "math/vec3.h"
#include "math/mat4.h"

#include <ostream>

namespace hop {

template <typename T>
class Quaternion
{
public:
    Vec3<T> v;
    T w;

    Quaternion() : v(0), w(1) { }
    Quaternion(const Quaternion<T>& q) : v(q.v), w(q.w) { }
    Quaternion(const Vec3<T>& v, const T w) : v(v), w(w) { }
};

typedef Quaternion<float> Quaternionf;
typedef Quaternion<double> Quaterniond;
typedef Quaternion<Real> Quaternionr;

template <typename T>
inline Quaternion<T> operator+(const Quaternion<T>& q1, const Quaternion<T>& q2)
{
    return Quaternion<T>(q1.v + q2.v, q1.w + q2.w);
}

template <typename T>
inline Quaternion<T> operator-(const Quaternion<T>& q1, const Quaternion<T>& q2)
{
    return Quaternion<T>(q1.v - q2.v, q1.w - q2.w);
}

template <typename T>
inline Quaternion<T> operator*(const T v, const Quaternion<T>& q)
{
    return Quaternion<T>(v * q.v, v * q.w);
}

template <typename T>
inline Quaternion<T> operator*(const Quaternion<T>& q1, const Quaternion<T>& q2)
{
    return Quaternion<T>(
            q1.w * q2.v + q2.w * q1.v + cross(q1.v, q2.v),
            q1.w * q2.w - dot(q1.v, q2.v));
}

template <typename T>
inline T dot(const Quaternion<T>& q1, const Quaternion<T>& q2)
{
    return q1.w * q2.w + dot(q1.v, q2.v);
}

template <typename T>
inline Quaternion<T> normalize(const Quaternion<T>& q)
{
    return (T(1) * rcp(sqrt(dot(q, q)))) * q;
}

template <typename T>
inline T length(const Quaternion<T>& q)
{
    return sqrt(dot(q, q));
}

template <typename T>
inline Quaternion<T> inverse(const Quaternion<T>& q)
{
    float scaler = T(1) * rcp(dot(q, q));
    return Quaternion<T>(T(-1) * scaler * q.v, scaler * q.w);
}

template <typename T>
inline Vec3<T> rotate_vector(const Quaternion<T>& q, const Vec3<T>& v)
{
    Vec3<T> c = cross(q.v, v);
    return v + c * (T(2) * q.w) + cross(T(2) * q.v, c);
}

template <typename T>
inline Quaternion<T> quat_from_axis_angle(const Vec3<T>& axis, const T angle)
{
    T s = sin(angle * T(0.5));
    T c = cos(angle * T(0.5));

    return Quaternion<T>(axis * s, c);
}

template <typename T>
Mat4<T> to_matrix(const Quaternion<T>& q)
{
    const T xx = q.v.x * q.v.x;
    const T yy = q.v.y * q.v.y;
    const T zz = q.v.z * q.v.z;
    const T xy = q.v.x * q.v.y;
    const T xz = q.v.x * q.v.z;
    const T yz = q.v.y * q.v.z;
    const T xw = q.v.x * q.w;
    const T yw = q.v.y * q.w;
    const T zw = q.v.z * q.w;

    Mat4<T> m;
    m[0][0] = T(1) - T(2) * (yy + zz);
    m[1][0] = T(2) * (xy - zw);
    m[2][0] = T(2) * (xz + yw);
    m[0][1] = T(2) * (xy + zw);
    m[1][1] = T(1) - T(2) * (xx + zz);
    m[2][1] = T(2) * (yz - xw);
    m[0][2] = T(2) * (xz - yw);
    m[1][2] = T(2) * (yz + xw);
    m[2][2] = T(1) - T(2) * (xx + yy);

    m[0][3] = m[1][3] = m[2][3] = T(0);
    m[3][0] = m[3][1] = m[3][2] = T(0);
    m[3][3] = T(1);

    return m;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const Quaternion<T>& q)
{
    os << "[" << q.v << " " << q.w << "]";
    return os;
}

} // namespace hop
