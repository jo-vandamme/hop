#include <vector>
#include <string>
#include <sstream>

namespace hop {

std::vector<std::string> split_string(const std::string& s, char sep = ' ');

template <typename T>
std::string to_string(const std::vector<T>& vec, const std::string& sep = "")
{
    std::ostringstream oss;
    for (size_t i = 0; i < vec.size(); ++i)
        oss << vec[i] << sep;
    oss << vec[vec.size() - 1];
    return oss.str();
}

} // namespace hop
