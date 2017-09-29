#pragma once

#include "math/vec3.h"

namespace hop { namespace sample {

Vec3f uniform_sample_hemisphere(float u1, float u2);
Vec3f cosine_sample_hemisphere(float u1, float u2);

} } // namespace hop::sample
