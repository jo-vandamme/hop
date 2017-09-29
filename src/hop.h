#pragma once

// The shading pipeline uses floats
// This define determines if the geometry pipelines
// should use floats or doubles
#define REAL_IS_DOUBLE

#ifdef REAL_IS_DOUBLE
    #define BBOX_SIMD_ISECT
#endif

#define RAND_HARDWARE

#define MIN_PRIMS_PER_LEAF 8
#define NUM_SAH_SPLITS 20
#define BVH_TRAV_COST Real(0.25)
#define NUM_AO_RAYS 5

#define RAY_EPSILON Real(1e-14)
#define RAY_TMIN    Real(1e-6)
#define RAY_TFAR    Real(1e14)

#define TILES_SPIRAL

#define AO_BACKGROUND Spectrum(0.0f, 0.0f, 0.0f)

#define likely(x) __builtin_expect(!!(x),1)
#define unlikely(x) __builtin_expect(!!(x),0)

#define ALIGN(x) __attribute__((aligned((x))))

#define NOOPTIMIZE __attribute__((optimize("O0")))
