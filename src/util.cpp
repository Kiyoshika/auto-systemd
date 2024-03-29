#include "util.hpp"

std::pair<std::string, std::string> asyd::util::parse_line(const std::string& current_line)
{
    std::string key = "";
    std::string value = "";
    bool parsing_key = true;

    for (const char c : current_line)
    {
        if (c == '=')
        {
            parsing_key = false;
            continue;
        }

        if (parsing_key)
            key += c;
        else
            value += c;
    }

    return std::make_pair(key, value);
}

std::string asyd::util::strip_newline(const std::string& value)
{
    std::string new_value = value;
    if (!new_value.empty() && new_value[new_value.length()-1] == '\n')
        new_value.erase(new_value.length()-1);

    return new_value;
}
