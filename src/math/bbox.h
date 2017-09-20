#pragma once

#include "hop.h"
#include "math/math.h"
#include "math/vec3.h"

#include <ostream>

namespace hop {

template <typename T>
class BBox
{
public:
    Vec3<T> pmin, pmax;

    BBox()
    {
        pmin = Vec3<T>(pos_inf, pos_inf, pos_inf);
        pmax = Vec3<T>(neg_inf, neg_inf, neg_inf);
    }

    BBox(const Vec3<T>& p) : pmin(p), pmax(p) { }

    BBox(const Vec3<T>& p1, const Vec3<T>& p2)
    {
        pmin = min(p1, p2);
        pmax = max(p1, p2);
    }

    BBox(const Vec3<T>& p1, const Vec3<T>& p2, const Vec3<T>& p3)
    {
        pmin = min(p1, min(p2, p3));
        pmax = max(p1, max(p2, p3));
    }

    inline BBox<T>& merge(const BBox<T>& o)
    {
        pmin = min(pmin, o.pmin);
        pmax = max(pmax, o.pmax);
        return *this;
    }

    inline BBox<T>& merge(const Vec3<T>& p)
    {
        pmin = min(pmin, p);
        pmax = max(pmax, p);
        return *this;
    }

    inline Vec3<T> get_centroid() const
    {
        return (pmin + pmax) * T(0.5);
    }

    inline bool empty() const
    {
        if (pmin.x > pmax.x) return true;
        if (pmin.y > pmax.y) return true;
        if (pmin.z > pmax.z) return true;
        return false;
    }
};

typedef BBox<double> BBoxd;
typedef BBox<float> BBoxf;
typedef BBox<Real> BBoxr;

template <typename T>
inline BBox<T> merge(const BBox<T>& b1, const BBox<T>& b2)
{
    return BBox<T>(min(b1.pmin, b2.pmin), max(b1.pmax, b2.pmax));
}

template <typename T>
inline BBox<T> merge(const BBox<T>& b, const Vec3<T>& p)
{
    return BBox<T>(min(b.pmin, p), max(b.pmax, p));
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const BBox<T>& v)
{
    os << "[" << v.pmin << ", " << v.pmax << "]";
    return os;
}

} // namespace hop
