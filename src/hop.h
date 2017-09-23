#pragma once

#define RAND_HARDWARE

#undef DEBUG_BBOX
#undef DEBUG_TRIS

#define BBOX_SIMD_ISECT
#undef TRIS_SIMD_ISECT

#define MIN_PRIMS_PER_LEAF 8
#define NUM_SAH_SPLITS 20
#define BVH_TRAV_COST 0.25
#define NUM_AO_RAYS 5
#define RAY_EPSILON 1e-14
#define RAY_TMIN 1e-3
#define RAY_TFAR 1e14

#define TILES_SPIRAL

#define likely(x) __builtin_expect(!!(x),1)
#define unlikely(x) __builtin_expect(!!(x),0)

#define ALIGN(x) __attribute__((aligned((x))))

#define NOOPTIMIZE __attribute__((optimize("O0")))

typedef double Real;
