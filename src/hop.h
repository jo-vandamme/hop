#pragma once

#define RAND_HARDWARE

#undef DEBUG_BBOX
#undef DEBUG_TRIS

#define BBOX_SIMD_ISECT
#undef TRIS_SIMD_ISECT

#define MIN_PRIMS_PER_LEAF 8
#define NUM_SAH_SPLITS 30
#define BVH_TRAV_COST 0.25

#define likely(x)       __builtin_expect(!!(x),1)
#define unlikely(x)     __builtin_expect(!!(x),0)

#define ALIGN(x) __attribute__((aligned((x))))

typedef double Real;
