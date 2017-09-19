#pragma once

#include "hop.h"
#include "types.h"
#include "math/math.h"
#include "math/vec3.h"

#include <ostream>
#include <iomanip>

namespace hop {

template <typename T>
struct Mat4
{
    T m[4][4] = { {1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1} };

    Mat4() { }

    Mat4(T a, T b, T c, T d, T e, T f, T g, T h,
         T i, T j, T k, T l, T x, T n, T o, T p)
    {
        m[0][0] = a; m[0][1] = b; m[0][2] = c; m[0][3] = d;
        m[1][0] = e; m[1][1] = f; m[1][2] = g; m[1][3] = h;
        m[2][0] = i; m[2][1] = j; m[2][2] = k; m[2][3] = l;
        m[3][0] = x; m[3][1] = n; m[3][2] = o; m[3][3] = p;
    }

    const T* operator[](uint8 i) const { return m[i]; }
    T* operator[](uint8 i) { return m[i]; }
};

typedef Mat4<double> Mat4d;
typedef Mat4<float> Mat4f;
typedef Mat4<Real> Mat4r;

template <typename T>
inline Mat4<T> operator-(const Mat4<T>& m)
{
    return Mat4<T>(
            -m[0][0], -m[0][1], -m[0][2], -m[0][3],
            -m[1][0], -m[1][1], -m[1][2], -m[1][3],
            -m[2][0], -m[2][1], -m[2][2], -m[2][3],
            -m[3][0], -m[3][1], -m[3][2], -m[3][3]);
}

template <typename T>
inline Mat4<T> transpose(const Mat4<T>& m)
{
    return Mat4<T>(
            m[0][0], m[1][0], m[2][0], m[3][0],
            m[0][1], m[1][1], m[2][1], m[3][1],
            m[0][2], m[1][2], m[2][2], m[3][2],
            m[0][3], m[1][3], m[2][3], m[3][3]);
}

template <typename T>
inline Mat4<T> operator+(const Mat4<T>& m1, const Mat4<T>& m2)
{
    return Mat4<T>(
            m1[0][0] + m2[0][0], m1[0][1] + m2[0][1], m1[0][2] + m2[0][2], m1[0][3] + m2[0][3],
            m1[1][0] + m2[1][0], m1[1][1] + m2[1][1], m1[1][2] + m2[1][2], m1[1][3] + m2[1][3],
            m1[2][0] + m2[2][0], m1[2][1] + m2[2][1], m1[2][2] + m2[2][2], m1[2][3] + m2[2][3],
            m1[3][0] + m2[3][0], m1[3][1] + m2[3][1], m1[3][2] + m2[3][2], m1[3][3] + m2[3][3]);
}

template <typename T>
inline Mat4<T> operator-(const Mat4<T>& m1, const Mat4<T>& m2)
{
    return Mat4<T>(
            m1[0][0] - m2[0][0], m1[0][1] - m2[0][1], m1[0][2] - m2[0][2], m1[0][3] - m2[0][3],
            m1[1][0] - m2[1][0], m1[1][1] - m2[1][1], m1[1][2] - m2[1][2], m1[1][3] - m2[1][3],
            m1[2][0] - m2[2][0], m1[2][1] - m2[2][1], m1[2][2] - m2[2][2], m1[2][3] - m2[2][3],
            m1[3][0] - m2[3][0], m1[3][1] - m2[3][1], m1[3][2] - m2[3][2], m1[3][3] - m2[3][3]);
}

template <typename T>
inline Mat4<T> operator*(const Mat4<T>& m, const T& v)
{
    return Mat4<T>(
            m[0][0] * v, m[0][1] * v, m[0][2] * v, m[0][3] * v,
            m[1][0] * v, m[1][1] * v, m[1][2] * v, m[1][3] * v,
            m[2][0] * v, m[2][1] * v, m[2][2] * v, m[2][3] * v,
            m[3][0] * v, m[3][1] * v, m[3][2] * v, m[3][3] * v);
}

template <typename T>
inline Mat4<T> operator*(const T& v, const Mat4<T>& m)
{
    return Mat4<T>(
            v * m[0][0], v * m[0][1], v * m[0][2], v * m[0][3],
            v * m[1][0], v * m[1][1], v * m[1][2], v * m[1][3],
            v * m[2][0], v * m[2][1], v * m[2][2], v * m[2][3],
            v * m[3][0], v * m[3][1], v * m[3][2], v * m[3][3]);
}

template <typename T>
inline Mat4<T> operator*(const Mat4<T>& mat1, const Mat4<T>& mat2)
{
    const T* __restrict m1 = &mat1[0][0];
    const T* __restrict m2 = &mat2[0][0];

    return Mat4<T>(
		m2[0] * m1[0]  + m2[4] * m1[1]  + m2[8]  * m1[2]  + m2[12] * m1[3],
		m2[1] * m1[0]  + m2[5] * m1[1]  + m2[9]  * m1[2]  + m2[13] * m1[3],
		m2[2] * m1[0]  + m2[6] * m1[1]  + m2[10] * m1[2]  + m2[14] * m1[3],
		m2[3] * m1[0]  + m2[7] * m1[1]  + m2[11] * m1[2]  + m2[15] * m1[3],
		m2[0] * m1[4]  + m2[4] * m1[5]  + m2[8]  * m1[6]  + m2[12] * m1[7],
		m2[1] * m1[4]  + m2[5] * m1[5]  + m2[9]  * m1[6]  + m2[13] * m1[7],
		m2[2] * m1[4]  + m2[6] * m1[5]  + m2[10] * m1[6]  + m2[14] * m1[7],
		m2[3] * m1[4]  + m2[7] * m1[5]  + m2[11] * m1[6]  + m2[15] * m1[7],
		m2[0] * m1[8]  + m2[4] * m1[9]  + m2[8]  * m1[10] + m2[12] * m1[11],
		m2[1] * m1[8]  + m2[5] * m1[9]  + m2[9]  * m1[10] + m2[13] * m1[11],
		m2[2] * m1[8]  + m2[6] * m1[9]  + m2[10] * m1[10] + m2[14] * m1[11],
		m2[3] * m1[8]  + m2[7] * m1[9]  + m2[11] * m1[10] + m2[15] * m1[11],
		m2[0] * m1[12] + m2[4] * m1[13] + m2[8]  * m1[14] + m2[12] * m1[15],
		m2[1] * m1[12] + m2[5] * m1[13] + m2[9]  * m1[14] + m2[13] * m1[15],
		m2[2] * m1[12] + m2[6] * m1[13] + m2[10] * m1[14] + m2[14] * m1[15],
		m2[3] * m1[12] + m2[7] * m1[13] + m2[11] * m1[14] + m2[15] * m1[15]);
}

template <typename T>
Mat4<T> inverse(const Mat4<T>& mat)
{
    const T* __restrict m = &mat[0][0];

	T det = m[0] * m[5] * m[10] * m[15] - m[0] * m[5] * m[11] * m[14] - m[0] * m[6] * m[9]  * m[15] + m[0] * m[6] * m[11] * m[13]
          + m[0] * m[7] * m[9]  * m[14] - m[0] * m[7] * m[10] * m[13] - m[1] * m[4] * m[10] * m[15] + m[1] * m[4] * m[11] * m[14]
          + m[1] * m[6] * m[8]  * m[15] - m[1] * m[6] * m[11] * m[12] - m[1] * m[7] * m[8]  * m[14] + m[1] * m[7] * m[10] * m[12]
          + m[2] * m[4] * m[9]  * m[15] - m[2] * m[4] * m[11] * m[13] - m[2] * m[5] * m[8]  * m[15] + m[2] * m[5] * m[11] * m[12]
          + m[2] * m[7] * m[8]  * m[13] - m[2] * m[7] * m[9]  * m[12] - m[3] * m[4] * m[9]  * m[14] + m[3] * m[4] * m[10] * m[13]
          + m[3] * m[5] * m[8]  * m[14] - m[3] * m[5] * m[10] * m[12] - m[3] * m[6] * m[8]  * m[13] + m[3] * m[6] * m[9]  * m[12];

    if (almost_equal(det, T(0)))
        return Mat4<T>();

    Mat4<T> ret(
		-m[7] * m[10] * m[13] + m[6] * m[11] * m[13] + m[7] * m[9] * m[14] - m[5] * m[11] * m[14] - m[6] * m[9] * m[15] + m[5] * m[10] * m[15],
		 m[3] * m[10] * m[13] - m[2] * m[11] * m[13] - m[3] * m[9] * m[14] + m[1] * m[11] * m[14] + m[2] * m[9] * m[15] - m[1] * m[10] * m[15],
		-m[3] * m[6]  * m[13] + m[2] * m[7]  * m[13] + m[3] * m[5] * m[14] - m[1] * m[7]  * m[14] - m[2] * m[5] * m[15] + m[1] * m[6]  * m[15],
		 m[3] * m[6]  * m[9]  - m[2] * m[7]  * m[9]  - m[3] * m[5] * m[10] + m[1] * m[7]  * m[10] + m[2] * m[5] * m[11] - m[1] * m[6]  * m[11],
		 m[7] * m[10] * m[12] - m[6] * m[11] * m[12] - m[7] * m[8] * m[14] + m[4] * m[11] * m[14] + m[6] * m[8] * m[15] - m[4] * m[10] * m[15],
		-m[3] * m[10] * m[12] + m[2] * m[11] * m[12] + m[3] * m[8] * m[14] - m[0] * m[11] * m[14] - m[2] * m[8] * m[15] + m[0] * m[10] * m[15],
		 m[3] * m[6]  * m[12] - m[2] * m[7]  * m[12] - m[3] * m[4] * m[14] + m[0] * m[7]  * m[14] + m[2] * m[4] * m[15] - m[0] * m[6]  * m[15],
		-m[3] * m[6]  * m[8]  + m[2] * m[7]  * m[8]  + m[3] * m[4] * m[10] - m[0] * m[7]  * m[10] - m[2] * m[4] * m[11] + m[0] * m[6]  * m[11],
		-m[7] * m[9]  * m[12] + m[5] * m[11] * m[12] + m[7] * m[8] * m[13] - m[4] * m[11] * m[13] - m[5] * m[8] * m[15] + m[4] * m[9]  * m[15],
		 m[3] * m[9]  * m[12] - m[1] * m[11] * m[12] - m[3] * m[8] * m[13] + m[0] * m[11] * m[13] + m[1] * m[8] * m[15] - m[0] * m[9]  * m[15],
		-m[3] * m[5]  * m[12] + m[1] * m[7]  * m[12] + m[3] * m[4] * m[13] - m[0] * m[7]  * m[13] - m[1] * m[4] * m[15] + m[0] * m[5]  * m[15],
		 m[3] * m[5]  * m[8]  - m[1] * m[7]  * m[8]  - m[3] * m[4] * m[9]  + m[0] * m[7]  * m[9]  + m[1] * m[4] * m[11] - m[0] * m[5]  * m[11],
		 m[6] * m[9]  * m[12] - m[5] * m[10] * m[12] - m[6] * m[8] * m[13] + m[4] * m[10] * m[13] + m[5] * m[8] * m[14] - m[4] * m[9]  * m[14],
		-m[2] * m[9]  * m[12] + m[1] * m[10] * m[12] + m[2] * m[8] * m[13] - m[0] * m[10] * m[13] - m[1] * m[8] * m[14] + m[0] * m[9]  * m[14],
		 m[2] * m[5]  * m[12] - m[1] * m[6]  * m[12] - m[2] * m[4] * m[13] + m[0] * m[6]  * m[13] + m[1] * m[4] * m[14] - m[0] * m[5]  * m[14],
		-m[2] * m[5]  * m[8]  + m[1] * m[6]  * m[8]  + m[2] * m[4] * m[9]  - m[0] * m[6]  * m[9]  - m[1] * m[4] * m[10] + m[0] * m[5]  * m[10]);

    return ret * rcp(det);
}

template <typename T>
Vec3<T> mul_point(const Mat4<T>& m, const Vec3<T>& v)
{
    T a = v[0] * m[0][0] + v[1] * m[0][1] + v[2] * m[0][2] + m[0][3];
    T b = v[0] * m[1][0] + v[1] * m[1][1] + v[2] * m[1][2] + m[1][3];
    T c = v[0] * m[2][0] + v[1] * m[2][1] + v[2] * m[2][2] + m[2][3];
    T w = v[0] * m[3][0] + v[1] * m[3][1] + v[2] * m[3][2] + m[3][3];
    if (!almost_equal(w, T(1)) && !almost_equal(w, T(0)))
    {
        T w_inv = rcp(w);
        return Vec3<T>(a * w_inv, b * w_inv, c * w_inv);
    }
    return Vec3<T>(a, b, c);
}

template <typename T>
Vec3<T> mul_vec(const Mat4<T>& m, const Vec3<T>& v)
{
    T a = v[0] * m[0][0] + v[1] * m[0][1] + v[2] * m[0][2];
    T b = v[0] * m[1][0] + v[1] * m[1][1] + v[2] * m[1][2];
    T c = v[0] * m[2][0] + v[1] * m[2][1] + v[2] * m[2][2];
    return Vec3<T>(a, b, c);
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const Mat4<T>& m)
{
    std::ios_base::fmtflags old_flags = os.flags();
    int w = 6;
    os.precision(4);
    os.setf(std::ios_base::fixed);

    os << std::showpos;
    os << '(' << std::setw(w) << m[0][0] <<
          ' ' << std::setw(w) << m[0][1] <<
          ' ' << std::setw(w) << m[0][2] <<
          ' ' << std::setw(w) << m[0][3] << '\n' <<
          ' ' << std::setw(w) << m[1][0] <<
          ' ' << std::setw(w) << m[1][1] <<
          ' ' << std::setw(w) << m[1][2] <<
          ' ' << std::setw(w) << m[1][3] << '\n' <<
          ' ' << std::setw(w) << m[2][0] <<
          ' ' << std::setw(w) << m[2][1] <<
          ' ' << std::setw(w) << m[2][2] <<
          ' ' << std::setw(w) << m[2][3] << '\n' <<
          ' ' << std::setw(w) << m[3][0] <<
          ' ' << std::setw(w) << m[3][1] <<
          ' ' << std::setw(w) << m[3][2] <<
          ' ' << std::setw(w) << m[3][3] << ")\n";

    os.flags(old_flags);
    return os;
}

} // namespace hop
