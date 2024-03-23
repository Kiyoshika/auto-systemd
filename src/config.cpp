#include "config.hpp"

using namespace asyd;

bool Config::from_file(const std::string& filepath)
{
    std::ifstream config(filepath);
    if (!config.is_open())
        return false;

    std::string current_line;
    while (std::getline(config, current_line))
    {
        auto [key, value] = this->parse_line(current_line);
        if (key == "config_type")
        {
            if (value == "server")
                this->config_type = ConfigType::SERVER;
            else
                this->config_type = ConfigType::JOB;
        }
        else
        {
            key_action_fptr key_action = this->key_action[key];
            if (!key_action)
            {
                config.close();
                return false;
            }
            (this->*key_action)(value);
        }
    }

    config.close();
    return true;
}

bool Config::to_file(const std::string& filepath) const
{
    std::ofstream config(filepath);
    if (!config.is_open())
        return false;

    if (this->config_type == ConfigType::SERVER)
        config << "config_type=server\n";
    else
        config << "config_type=job\n";

    config << "project_description=" << this->project_description << "\n";
    config << "service_username=" << this->service_username << "\n";
    config << "server_hostname=" << this->server_hostname << "\n";
    config << "public_key_path=" << this->public_key_path << "\n";
    config << "working_directory=" << this->working_directory << "\n";
    config << "entry_point=" << this->entry_point << "\n";
    config << "port=" << this->port << "\n";
    config << "schedule=" << this->schedule;
    config.close();
    return true;
}

std::pair<std::string, std::string> Config::parse_line(const std::string& current_line) const
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
