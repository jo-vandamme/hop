#include "render/tonemap.h"

#include <string>
#include <algorithm>

namespace hop {

std::string to_lower(const char* str)
{
    std::string out(str);
    std::transform(out.begin(), out.end(), out.begin(), ::tolower);
    return out;
}

ToneMapType tonemap_from_string(const char* s)
{
    const std::string str = to_lower(s);

    if (str == "linear")
        return ToneMapType::LINEAR;
    else if (str == "gamma")
        return ToneMapType::GAMMA;
    else if (str == "reinhard")
        return ToneMapType::REINHARD;
    else if (str == "filmic")
        return ToneMapType::FILMIC;
    return ToneMapType::GAMMA;
}

} // namespace hop
