#pragma once

#include <vector>
#include <string>
#include <array>
#include <cstdio>

namespace asyd
{
class Command
{
public:
    Command() {}

    // Adds new token to command and whether or not to include
    // a space after.
    Command& add(const std::string& token, bool add_space = true);

    // Adds a double quote in the command buffer.
    Command& addQuote();

    // Executes the command and clears the command buffer.
    // Returns false if the status code of the command returns
    // anything but 0.
    // Fetch the output with command.get_output()
    bool execute();

    // Returns the output from the executed command and clears buffer.
    const std::string& get_output() const;
private:
    std::vector<std::string> command_tokens;
    std::string command_output;

    std::string to_string();
};
}; // namespace asyd
