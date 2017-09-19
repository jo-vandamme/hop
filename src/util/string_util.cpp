#include "util/string_util.h"

#include <vector>
#include <string>

namespace hop {

std::vector<std::string> split_string(const std::string& s, char sep)
{
    std::vector<std::string> tokens;
    std::string token;
    const char* c = s.c_str();

    while (1)
    {
        if (*c == sep || *c == '\0')
        {
            tokens.push_back(token);
            token.clear();
            if (*c == '\0')
                break;
        }
        else
        {
            token += *c;
        }
        ++c;
    }
    return tokens;
}

} // namespace hop
