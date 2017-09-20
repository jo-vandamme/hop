#include "math/math.h"

namespace hop {

int log2(int n)
{
    return __builtin_ffs(n) - 1;
}

} // namespace log

