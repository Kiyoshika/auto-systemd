#pragma once

#include <string>
#include <fstream>
#include <utility>
#include <functional>
#include <unordered_map>

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
        this->key_action["project_description"] = &Config::set_project_description;
        this->key_action["service_username"] = &Config::set_service_username;
        this->key_action["server_hostname"] = &Config::set_server_hostname;
        this->key_action["public_key_path"] = &Config::set_public_key_path;
        this->key_action["working_directory"] = &Config::set_working_directory;
        this->key_action["entry_point"] = &Config::set_entry_point;
        this->key_action["port"] = &Config::set_port;
        this->key_action["schedule"] = &Config::set_schedule;
    }

    ConfigType get_type() const;

    // Reads config settings from file.
    // Returns true on success, false otherwise.
    bool from_file(const std::string& filepath);

    // Writes config settings to file.
    // Returns true on succcess, false otherwise.
    bool to_file(const std::string& filepath) const;

    void set_project_description(const std::string& project_description)
    {
        this->project_description = project_description;
    }

    void set_service_username(const std::string& service_username)
    {
        this->service_username = service_username;
    }

    void set_server_hostname(const std::string& server_hostname)
    {
        this->server_hostname = server_hostname;
    }

    void set_public_key_path(const std::string& public_key_path)
    {
        this->public_key_path = public_key_path;
    }

    void set_working_directory(const std::string& working_directory)
    {
        this->working_directory = working_directory;
    }

    void set_entry_point(const std::string& entry_point)
    {
        this->entry_point = entry_point;
    }

    void set_port(const std::string& port)
    {
        this->port = port;
    }

    void set_schedule(const std::string& schedule)
    {
        this->schedule = schedule;
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

    const std::string& get_public_key_path() const
    {
        return this->public_key_path;
    }

    const std::string& get_working_directory() const
    {
        return this->working_directory;
    }

    const std::string& get_entry_point() const
    {
        return this->entry_point;
    }

    const std::string& get_port() const
    {
        return this->port;
    }

    const std::string& get_schedule() const
    {
        return this->schedule;
    }

private:
    enum asyd::ConfigType config_type;

    std::string project_description;    // -d
    std::string service_username;       // -u
    std::string server_hostname;        // -h
    std::string public_key_path;        // -k
    std::string working_directory;      // -D
    std::string entry_point;            // -e
    std::string port;                   // -p
    std::string schedule;               // -s

    std::unordered_map<std::string, key_action_fptr> key_action;

    std::pair<std::string, std::string> parse_line(const std::string& current_line) const;
}; // class Config
}; // namespace asyd
