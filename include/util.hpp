#pragma once

#include <utility>
#include <string>

namespace asyd
{
namespace util
{
    std::pair<std::string, std::string> parse_line(const std::string& current_line);

    std::string strip_newline(const std::string& value);
}; // namespace util
}; // namespace asyd
