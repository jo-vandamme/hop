#pragma once

#include "types.h"
#include "math/vec3.h"

namespace hop { namespace sample {

Vec3r uniform_sample_hemisphere(Real u1, Real u2);
Vec3r cosine_sample_hemisphere(Real u1, Real u2);

} } // namespace hop::sample
