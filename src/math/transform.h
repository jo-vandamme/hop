#pragma once

#include "hop.h"
#include "math/vec3.h"
#include "math/mat4.h"
#include "math/bbox.h"

#include <ostream>

namespace hop {

template <typename T>
class Transform
{
public:
    Mat4<T> m;
    Mat4<T> inv;

    Transform() { }
    explicit Transform(const Mat4<T>& m) : m(m), inv(inverse(m)) { }
    Transform(const Mat4<T>& m, const Mat4<T>& inv) : m(m), inv(inv) { }
    Transform(const Transform<T>& t) : m(t.m), inv(t.inv) { }

    Mat4<T> get_mat4() const { return m; }

    Transform<T> operator*(const Transform<T>& t) const
    {
        return Transform(m * t.m, t.inv * inv);
    }

    Transform<T> operator/(const Transform<T>& t) const
    {
        return Transform(m * t.inv, t.m * inv);
    }

};

typedef Transform<float> Transformf;
typedef Transform<double> Transformd;
typedef Transform<Real> Transformr;

template <typename T>
inline const Transform<T> inverse(const Transform<T>& t)
{
    return Transform<T>(t.inv, t.m);
}

template <typename T>
inline Vec3<T> transform_point(const Transform<T>& t, const Vec3<T>& p)
{
    return mul_point(t.m, p);
}

template <typename T>
inline Vec3<T> transform_vector(const Transform<T>& t, const Vec3<T>& v)
{
    return mul_vec(t.m, v);
}

template <typename T>
inline Vec3<T> transform_normal(const Transform<T>& t, const Vec3<T>& n)
{
    return mul_vec(transpose(t.inv), n);
}

template <typename T>
inline BBox<T> transform_bbox(const Transform<T>& t, const BBox<T>& b)
{
    BBox<T> res(transform_point(t, b.pmin));
    res = merge(res, transform_point(t, Vec3<T>(b.pmax.x, b.pmin.y, b.pmin.z)));
    res = merge(res, transform_point(t, Vec3<T>(b.pmin.x, b.pmax.y, b.pmin.z)));
    res = merge(res, transform_point(t, Vec3<T>(b.pmin.x, b.pmin.y, b.pmax.z)));
    res = merge(res, transform_point(t, Vec3<T>(b.pmin.x, b.pmax.y, b.pmax.z)));
    res = merge(res, transform_point(t, Vec3<T>(b.pmax.x, b.pmax.y, b.pmin.z)));
    res = merge(res, transform_point(t, Vec3<T>(b.pmax.x, b.pmin.y, b.pmax.z)));
    res = merge(res, transform_point(t, Vec3<T>(b.pmax.x, b.pmax.y, b.pmax.z)));
    return res;
}

template <typename T>
Transform<T> make_translation(const Vec3<T>& delta)
{
    Mat4<T> m(1, 0, 0, delta.x,
              0, 1, 0, delta.y,
              0, 0, 1, delta.z,
              0, 0, 0, 1);

    Mat4<T> i(1, 0, 0, -delta.x,
              0, 1, 0, -delta.y,
              0, 0, 1, -delta.z,
              0, 0, 0, 1);

    return Transform<T>(m, i);
}

template <typename T>
Transform<T> make_scale(const Vec3<T>& s)
{
    Mat4<T> m(s.x, 0, 0, 0,
              0, s.y, 0, 0,
              0, 0, s.z, 0,
              0, 0, 0, 1);

    Mat4<T> i(rcp(s.x), 0, 0, 0,
              0, rcp(s.y), 0, 0,
              0, 0, rcp(s.z), 0,
              0, 0, 0, 1);

    return Transform<T>(m, i);
}

template <typename T>
Transform<T> make_rotation_x(const T angle)
{
    const T s = sin(radians(angle));
    const T c = cos(radians(angle));

    Mat4<T> m(1, 0, 0, 0,
              0, c, -s, 0,
              0, s, c, 0,
              0, 0, 0, 1);

    return Transform<T>(m, transpose(m));
}

template <typename T>
Transform<T> make_rotation_y(const T angle)
{
    const T s = sin(radians(angle));
    const T c = cos(radians(angle));

    Mat4<T> m(c, 0, s, 0,
              0, 1, 0, 0,
             -s, 0, c, 0,
              0, 0, 0, 1);

    return Transform<T>(m, transpose(m));
}

template <typename T>
Transform<T> make_rotation_z(const T angle)
{
    const T s = sin(radians(angle));
    const T c = cos(radians(angle));

    Mat4<T> m(c, -s, 0, 0,
              s, c, 0, 0,
              0, 0, 1, 0,
              0, 0, 0, 1);

    return Transform<T>(m, transpose(m));
}

template <typename T>
Transform<T> make_rotation(const Vec3<T>& axis, const T angle)
{
    const Vec3<T> a = normalize(axis);
    const T s = sin(radians(angle));
    const T c = cos(radians(angle));
    Mat4<T> m;

    m[0][0] = a.x * a.x + (T(1) - a.x * a.x) * c;
	m[0][1] = a.x * a.y * (T(1) - c) - a.z * s;
	m[0][2] = a.x * a.z * (T(1) - c) + a.y * s;
	m[0][3] = T(0);

	m[1][0] = a.x * a.y * (T(1) - c) + a.z * s;
	m[1][1] = a.y * a.y + (T(1) - a.y * a.y) * c;
	m[1][2] = a.y * a.z * (T(1) - c) - a.x * s;
	m[1][3] = T(0);

	m[2][0] = a.x * a.z * (T(1) - c) - a.y * s;
	m[2][1] = a.y * a.z * (T(1) - c) + a.x * s;
	m[2][2] = a.z * a.z + (T(1) - a.z * a.z) * c;
	m[2][3] = T(0);

	m[3][0] = T(0);
	m[3][1] = T(0);
	m[3][2] = T(0);
	m[3][3] = T(1);

    return Transform<T>(m, transpose(m));
}

template <typename T>
Transform<T> make_perspective(const T fovy, const T n, const T f)
{
    T inv_den = rcp(f - n);
    T s = rcp(tan(deg2rad(fovy) * 0.5));

    Mat4<T> persp(s, 0, 0,             0,
                  0, s, 0,             0,
                  0, 0, -f * inv_den, -f * n * inv_den,
                  0, 0, -1,            0);

    return Transform<T>(persp);
}

template <typename T>
Transform<T> make_lookat(const Vec3<T>& eye, const Vec3<T>& target, const Vec3<T>& up)
{
    Vec3<T> f = normalize(eye - target);
    Vec3<T> l = normalize(cross(normalize(up), f));
    Vec3<T> u = cross(f, l);

    Mat4<T> lookat(l.x, u.x, f.x, eye.x,
                   l.y, u.y, f.y, eye.y,
                   l.z, u.z, f.z, eye.z,
                   0,   0,   0,   1);

    return Transform<T>(lookat);
}

template <typename T>
Vec3<T> get_col(const Transform<T>& m, int col)
{
    Vec3<T> out;
    out[0] = m.m[0][col];
    out[1] = m.m[1][col];
    out[2] = m.m[2][col];
    return out;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const Transform<T>& t)
{
    os << "[" << t.m << t.inv << "]";
    return os;
}

} // namespace hop
