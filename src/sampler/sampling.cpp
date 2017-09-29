#include "sampler/sampling.h"
#include "math/math.h"
#include "math/vec3.h"

namespace hop { namespace sample {

Vec3f uniform_sample_hemisphere(float u1, float u2)
{
    const float r = sqrt(1.0f - u1 * u1);
    const float phi = 2.0f * (float)pi * u2;
    return Vec3f(cos(phi) * r, sin(phi) * r, u1);
}

Vec3f cosine_sample_hemisphere(float u1, float u2)
{
    const float r = sqrt(u1);
    const float theta = 2.0f * (float)pi * u2;
    const float x = r * cos(theta);
    const float y = r * sin(theta);
    return Vec3f(x, y, sqrt(max(0.0f, 1.0f - u1)));
}

} } // namespace hop::sample
