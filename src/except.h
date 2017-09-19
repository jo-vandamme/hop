#pragma once

#include <stdexcept>
#include <string>

namespace hop {

class Error : public std::runtime_error
{
public:
    Error(const std::string& m) : std::runtime_error(m) { }
};

} // namespace hop
