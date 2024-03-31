#include "config.hpp"
#include "server.hpp"

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

using namespace asyd;

bool Config::from_file(const std::string& filepath)
{
    std::ifstream config(filepath);
    if (!config.is_open())
        return false;

    std::string current_line;
    while (std::getline(config, current_line))
    {
        auto [key, value] = asyd::util::parse_line(current_line);
        key_action_fptr key_action = this->key_action[key];
        if (!key_action)
        {
            config.close();
            return false;
        }
        (this->*key_action)(value);
    }

    config.close();
    return true;
}

bool Config::to_file(const std::string& filepath) const
{
    std::ofstream config(filepath);
    if (!config.is_open())
        return false;

    config << "project_name=" << this->project_name << "\n";
    config << "project_description=" << this->project_description << "\n";
    config << "service_username=" << this->service_username << "\n";
    config << "server_hostname=" << this->server_hostname << "\n";
    config << "working_directory=" << this->working_directory << "\n";
    config << "entry_point=" << this->entry_point << "\n";
    config << "schedule=" << this->schedule << "\n";
    config << "server_home_directory=" << this->server_home_directory << "\n";
    config << "server_bash_directory=" << this->server_bash_directory << "\n";
    config.close();
    return true;
}

bool Config::setup_server(const std::string& config_directory)
{
    Server server(this->server_hostname);

    server.set_is_root(this->service_username == "sudo");
    std::string server_project_dir = server.get_home() + "/.asyd/" + this->project_name;

    if (!server.create_directory("~/.asyd/"))
        return false;

    if (!server.copy_from_local(this->working_directory, server_project_dir))
        return false;

    if (!server.chmod("+x", server_project_dir + "/" + this->entry_point))
        return false;

    std::string service_name = this->project_name + ".service";

    if (!server.copy_systemd_file(config_directory, service_name))
        return false;

    if (!server.reload_service())
        return false;

    if (!server.enable_service(service_name))
        return false;

    if (!server.start_service(service_name))
        return false;
    
    return true;
}
