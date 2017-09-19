#include "accel/bvh_intersector_two_levels.h"

#ifdef TRIS_SIMD_ISECT

#include <limits>
#include <immintrin.h>

namespace hop { namespace bvh {

#define or8f _mm256_or_pd
#define mul _mm256_mul_pd
#define add _mm256_add_pd
#define sub _mm256_sub_pd
#define fmsub(a, b, c) sub(mul((a), (b)), (c))
#define fmadd(a, b, c) add(mul((a), (b)), (c))
#define cmp _mm256_cmp_pd
#define div _mm256_div_pd

void avx_multi_cross(__m256d result[3], const __m256d a[3], const __m256d b[3])
{
    result[0] = fmsub(a[1], b[2], mul(b[1], a[2]));
    result[1] = fmsub(a[2], b[0], mul(b[2], a[0]));
    result[2] = fmsub(a[0], b[1], mul(b[0], a[1]));
}

__m256d avx_multi_dot(const __m256d a[3], const __m256d b[3])
{
    return fmadd(a[2], b[2], fmadd(a[1], b[1], mul(a[0], b[0])));
}

void avx_multi_sub(__m256d result[3], const __m256d a[3], const __m256d b[3])
{
    result[0] = _mm256_sub_pd(a[0], b[0]);
    result[1] = _mm256_sub_pd(a[1], b[1]);
    result[2] = _mm256_sub_pd(a[2], b[2]);
}

bool intersect_triangles_simd(const PackedRay& ray, const PackedTriangles& tris, PackedHitInfo& hit)
{
    static const __m256d one_m256 = _mm256_set1_pd(1.0);
    static const __m256d minus_one_m256 = _mm256_set1_pd(-1.0);
    static const __m256d pos_eps_m256 = _mm256_set1_pd(1e-6);
    static const __m256d neg_eps_m256 = _mm256_set1_pd(-1e-6);
    static const __m256d zero_m256 = _mm256_set1_pd(0.0);

    // check if ray is parallel
    __m256d s1[3];
    avx_multi_cross(s1, ray.dir, tris.e2);
    __m256d det = avx_multi_dot(tris.e1, s1);

    __m256d inv_det = div(one_m256, det);

    // first barycentric coordinate
    __m256d d[3];
    avx_multi_sub(d, ray.org, tris.v0);
    __m256d b1 = mul(inv_det, avx_multi_dot(d, s1));

    // second barycentric coordinate
    __m256d s2[3];
    avx_multi_cross(s2, d, tris.e1);
    __m256d b2 = mul(inv_det, avx_multi_dot(ray.dir, s2));

    // distance to intersection
    __m256d t = mul(inv_det, avx_multi_dot(tris.e2, s2));

    // check if determinant is close to 0 (ray parallel)
    __m256d failed = _mm256_and_pd(
        cmp(det, neg_eps_m256, _CMP_GT_OQ),
        cmp(det, pos_eps_m256, _CMP_LT_OQ)
    );

    // b1 < 0
    failed = or8f(failed, cmp(b1, zero_m256, _CMP_LT_OQ));
    // b2 < 0
    failed = or8f(failed, cmp(b2, zero_m256, _CMP_LT_OQ));
    // b1 + b2 > 1
    failed = or8f(failed, cmp(_mm256_add_pd(b1, b2), one_m256, _CMP_GT_OQ));
    // t < tmin
    failed = or8f(failed, cmp(t, ray.tmin, _CMP_LT_OQ));
    // t > tmax
    failed = or8f(failed, cmp(t, ray.tmax, _CMP_GT_OQ));
    // mask inactive triangles
    failed = or8f(failed, tris.inactive_mask);

    // set failed slots to -1
    __m256d hit_t = _mm256_blendv_pd(t, minus_one_m256, failed);

    int mask = _mm256_movemask_pd(hit_t);
    if (mask != 0xf)
    {
        // There is at least one intersection
        hit.idx = -1;

        double* hit_ptr = (double*)&hit_t;
        double* b1_ptr = (double*)&b1;
        double* b2_ptr = (double*)&b2;
        for (int i = 0; i < 4; ++i)
        {
            if (hit_ptr[i] >= 0.0 && hit_ptr[i] < hit.t)
            {
                hit.t = hit_ptr[i];
                hit.b1 = b1_ptr[i];
                hit.b2 = b2_ptr[i];
                hit.idx = i;
            }
        }
        return hit.idx != -1;
    }
    return false;
}

} } // namespace hop::bvh

#endif
