#pragma once

#include "hop.h"
#include "math/vec2.h"

namespace hop {

class CameraSample
{
public:
    Vec2r film_point;
    Vec2r lens_point;
};

} // namespace hop
