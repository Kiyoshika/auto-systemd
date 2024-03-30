#include "command.hpp"
#include "util.hpp"

using namespace asyd;

Command& Command::add(const std::string& token, bool add_space)
{
    if (add_space)
        this->command_tokens.push_back(token + " ");
    else
        this->command_tokens.push_back(token);
    return *this;
}

Command& Command::addQuote()
{
    this->command_tokens.push_back("\"");
    return *this;
}

std::string Command::to_string()
{
    std::string command = "";
    for (const std::string& token : this->command_tokens)
        command += token;

    this->command_tokens.clear();
    return command;
}

bool Command::execute()
{
    std::string command_str = this->to_string();
    const char* cmd = command_str.c_str();

    std::array<char, 2048> buffer;
    std::string result;

    auto pipe = popen(cmd, "r");

    if (!pipe)
        return false;

    while (!feof(pipe))
    {
        if (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
            result += buffer.data();
    }

    auto return_code = pclose(pipe);

    this->command_output = asyd::util::strip_newline(result);

    return return_code == 0;
}

const std::string& Command::get_output() const
{
    return this->command_output;
}
