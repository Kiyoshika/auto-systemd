#include "config.hpp"
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

using namespace asyd;

ConfigType Config::get_type() const
{
    return this->config_type;
}

bool Config::from_file(const std::string& filepath)
{
    std::ifstream config(filepath);
    if (!config.is_open())
        return false;

    std::string current_line;
    while (std::getline(config, current_line))
    {
        auto [key, value] = asyd::util::parse_line(current_line);
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

    config << "project_name=" << this->project_name << "\n";
    config << "project_description=" << this->project_description << "\n";
    config << "service_username=" << this->service_username << "\n";
    config << "server_hostname=" << this->server_hostname << "\n";
    config << "working_directory=" << this->working_directory << "\n";
    config << "entry_point=" << this->entry_point << "\n";
    if (this->get_type() == ConfigType::JOB)
        config << "schedule=" << this->schedule << "\n";
    config << "server_home_directory=" << this->server_home_directory << "\n";
    config << "server_bash_directory=" << this->server_bash_directory << "\n";
    config.close();
    return true;
}

// TODO: also return false if the exit status on command is non-zero
bool execute_command(const char* cmd, std::string& output) {
    std::array<char, 256> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe)
        return false;
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr)
        result += buffer.data();

    output = result;
    return true;
}

bool Config::fetch_server_info()
{
    std::string output = "";
    std::string get_server_home_command = "ssh " + this->server_hostname + " \"pwd ~\"";
    if (!execute_command(get_server_home_command.c_str(), output))
        return false;
    this->set_server_home_directory(output);

    // this prints out a result like:
    // bash: /usr/bin/bash /some/other/thing/maybe
    // we we want to extract the "first" result after the "bash: " text
    std::string get_server_bash_command = "ssh " + this->server_hostname + " \"whereis bash\"";
    std::string server_bash_directory = "";
    if (!execute_command(get_server_bash_command.c_str(), output))
        return false;

    std::string bash_directory = "";
    // the +6 is to move after the "bash: " text
    for (std::size_t i = 6; i < output.length(); ++i)
    {
        if (output[i] == ' ')
            break;

        bash_directory += output[i];
    }

    this->set_server_bash_directory(bash_directory);
    return true;
}

bool Config::setup_server(const std::string& config_directory)
{
    std::string command_output = "";

    // make directory for project on server (~/.asyd/)
    std::string create_project_dir = "ssh " + this->server_hostname + " \"mkdir -p ~/.asyd/\"";
    if (!execute_command(create_project_dir.c_str(), command_output))
        return false;

    std::string copy_project = "scp -rp " + this->working_directory + " " + this->server_hostname + ":~/.asyd/" + this->project_name;
    if (!execute_command(copy_project.c_str(), command_output))
        return false;

    // make the entry point script executable
    std::string make_exe = "ssh " + this->server_hostname + " \"chmod +x ~/.asyd/" + this->project_name + "/" + this->entry_point + "\"";
    if (!execute_command(make_exe.c_str(), command_output))
        return false;

    if (this->service_username == "sudo")
    {
        // copy systemd config
        std::string copy_systemd = "scp " + config_directory + "/" + this->project_name + ".service " + this->server_hostname + ":/etc/systemd/system/" + this->project_name + ".service";
        if (!execute_command(copy_systemd.c_str(), command_output))
            return false;

        // reload systemd daemon
        std::string reload = "ssh " + this->server_hostname + "\"systemctl daemon-reload\"";
        if (!execute_command(reload.c_str(), command_output))
            return false;

        // enable the service
        std::string enable_service = "ssh " + this->server_hostname + " \"systemctl enable " + this->project_name + ".service\"";
        if (!execute_command(enable_service.c_str(), command_output))
            return false;

        // start the service
        std::string start_service = "ssh " + this->server_hostname + " \"systemctl start " + this->project_name + ".service\"";
        if (!execute_command(start_service.c_str(), command_output))
            return false;
    }
    else
    {
        // copy systemd config
        std::string copy_systemd = "scp " + config_directory + "/" + this->project_name + ".service " + this->server_hostname + ":~/.config/systemd/user/" + this->project_name + ".service";
        if (!execute_command(copy_systemd.c_str(), command_output))
            return false;

        // reload systemd daemon
        std::string reload = "ssh " + this->server_hostname + " \"systemctl --user daemon-reload\"";
        if (!execute_command(reload.c_str(), command_output))
            return false;

        // enable the service
        std::string enable_service = "ssh " + this->server_hostname + " \"systemctl --user enable " + this->project_name + ".service\"";
        if (!execute_command(enable_service.c_str(), command_output))
            return false;

        // start the service
        std::string start_service = "ssh " + this->server_hostname + " \"systemctl --user start " + this->project_name + ".service\"";
        if (!execute_command(start_service.c_str(), command_output))
            return false;
    }

    return true;
}
