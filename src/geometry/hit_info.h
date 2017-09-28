#pragma once

#include "types.h"

namespace hop {

class HitInfo
{
public:
    Real t;
    Real b1;
    Real b2;
    int32 primitive_id;
    int32 shape_id;
};

} // namespace hop
