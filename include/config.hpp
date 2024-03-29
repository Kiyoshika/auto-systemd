#pragma once

#include <string>
#include <fstream>
#include <utility>
#include <functional>
#include <unordered_map>
#include <filesystem>

#include "util.hpp"
#include "systemd.hpp"

namespace asyd
{
enum ConfigType
{
    SERVER,
    JOB
}; // enum ConfigType

class Config 
{
public:
    typedef void (Config::*key_action_fptr)(const std::string&);

    Config(enum asyd::ConfigType config_type)
    {
        this->config_type = config_type;
        this->key_action["project_name"] = &Config::set_project_name;
        this->key_action["project_description"] = &Config::set_project_description;
        this->key_action["service_username"] = &Config::set_service_username;
        this->key_action["server_hostname"] = &Config::set_server_hostname;
        this->key_action["working_directory"] = &Config::set_working_directory;
        this->key_action["entry_point"] = &Config::set_entry_point;
        this->key_action["schedule"] = &Config::set_schedule;
        this->key_action["server_home_directory"] = &Config::set_server_home_directory;
        this->key_action["server_bash_directory"] = &Config::set_server_bash_directory;
    }

    ConfigType get_type() const;

    // Reads config settings from file.
    // Returns true on success, false otherwise.
    bool from_file(const std::string& filepath);

    // Writes config settings to file.
    // Returns true on succcess, false otherwise.
    bool to_file(const std::string& filepath) const;

    // fetches the home directory and bash directory to
    // setup systemd service properly (some distributions can vary)
    bool fetch_server_info();

    // Copies the files from the local working directory to
    // ~/.asyd/project_name on the remote server
    // Copies the systemd service config to: /etc/systemd/system
    // Starts the service
    bool setup_server(const std::string& config_directory);

    void set_project_description(const std::string& project_description)
    {
        this->project_description = asyd::util::strip_newline(project_description);
    }

    void set_service_username(const std::string& service_username)
    {
        this->service_username = asyd::util::strip_newline(service_username);
    }

    void set_server_hostname(const std::string& server_hostname)
    {
        this->server_hostname = asyd::util::strip_newline(server_hostname);
    }

    void set_working_directory(const std::string& working_directory)
    {
        this->working_directory = asyd::util::strip_newline(working_directory);
    }

    void set_entry_point(const std::string& entry_point)
    {
        this->entry_point = asyd::util::strip_newline(entry_point);
    }

    void set_schedule(const std::string& schedule)
    {
        this->schedule = asyd::util::strip_newline(schedule);
    }

    void set_project_name(const std::string& project_name)
    {
        this->project_name = asyd::util::strip_newline(project_name);
    }

    void set_server_home_directory(const std::string& server_home_directory)
    {
        this->server_home_directory = asyd::util::strip_newline(server_home_directory);
    }

    void set_server_bash_directory(const std::string& server_bash_directory)
    {
        this->server_bash_directory = asyd::util::strip_newline(server_bash_directory);
    }

    const std::string& get_project_description() const
    {
        return this->project_description;
    }

    const std::string& get_service_username() const
    {
        return this->service_username;
    }

    const std::string& get_server_hostname() const
    {
        return this->server_hostname;
    }

    const std::string& get_working_directory() const
    {
        return this->working_directory;
    }

    const std::string& get_entry_point() const
    {
        return this->entry_point;
    }

    const std::string& get_schedule() const
    {
        return this->schedule;
    }

    const std::string& get_project_name() const
    {
        return this->project_name;
    }

    const std::string& get_server_home_directory() const
    {
        return this->server_home_directory;
    }

    const std::string& get_server_bash_directory() const
    {
        return this->server_bash_directory;
    }

private:
    std::string project_name;
    enum asyd::ConfigType config_type;

    // server-specific settings that need to be fetched
    std::string server_home_directory;
    std::string server_bash_directory;

    std::string project_description;    // -d
    std::string service_username;       // -u
    std::string server_hostname;        // -h
    std::string working_directory;      // -D
    std::string entry_point;            // -e
    std::string schedule;               // -s

    std::unordered_map<std::string, key_action_fptr> key_action;
}; // class Config
}; // namespace asyd
