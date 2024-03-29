#include "systemd.hpp"
#include "config.hpp"

using namespace asyd;

void Systemd::from_config(const Config& config)
{
    this->description = config.get_project_description();
    this->working_directory = config.get_server_home_directory() 
        + "/.asyd/" 
        + config.get_project_name();
    this->entry_point = config.get_server_bash_directory() 
        + " -c '" 
        + this->working_directory
        + "/" 
        + config.get_entry_point()
        + "'\n";
    this->is_sudo = config.get_service_username() == "sudo";
}

bool Systemd::from_file(const std::string& filepath)
{
    std::ifstream sysfile(filepath);
    if (!sysfile.is_open())
        return false;

    std::string current_line;
    while (std::getline(sysfile, current_line))
    {
        if (current_line.find('=') != std::string::npos)
        {
            auto [key, value] = asyd::util::parse_line(current_line);
            if (this->key_action.find(key) != this->key_action.end())
            {
                key_action_fptr key_action = this->key_action[key];
                (this->*key_action)(value);
            }
        }
    }

    sysfile.close();
    return true;
}

bool Systemd::to_file(const std::string& filepath)
{
    std::ofstream sysfile(filepath);
    if (!sysfile.is_open())
        return false;

    sysfile << "[Unit]\n";
    sysfile << "Description=" << this->description << "\n";

    sysfile << "\n[Service]\n";
    sysfile << "Type=simple\n";
    sysfile << "Restart=always\n";
    sysfile << "RestartSec=1\n";
    sysfile << "WorkingDirectory=" << this->working_directory << "\n";
    sysfile << "ExecStart=" << this->entry_point << "\n";

    sysfile << "\n[Install]\n";
    if (this->is_sudo)
        sysfile << "WantedBy=multi-user.target\n";
    else
        sysfile << "WantedBy=default.target\n";
    
    sysfile.close();
    return true;
}
