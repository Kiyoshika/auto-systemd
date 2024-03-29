#pragma once

#include <cstdio>
#include <fstream>
#include <unordered_map>
#include <string>

#include "util.hpp"

namespace asyd
{
// forward declaration
class Config;

class Systemd
{
public:
    typedef void (Systemd::*key_action_fptr)(const std::string&);

    Systemd()
    {
        this->key_action["Description"] = &Systemd::set_description;
        this->key_action["WorkingDirectory"] = &Systemd::set_working_directory;
        this->key_action["ExecStart"] = &Systemd::set_entry_point;
    }

    // Build Systemd object from Config object.
    // Returns true on success, false otherwise.
    void from_config(const asyd::Config& config);

    // Build Systemd object from file.
    // Returns true on success, false otherwise.
    bool from_file(const std::string& filepath);

    // Write Systemd config to file.
    // Returns true on success, false otherwise.
    bool to_file(const std::string& filepath);

    void set_description(const std::string& description)
    {
        this->description = asyd::util::strip_newline(description);
    }

    void set_working_directory(const std::string& working_directory)
    {
        this->working_directory = asyd::util::strip_newline(working_directory);
    }

    void set_entry_point(const std::string& entry_point)
    {
        this->entry_point = asyd::util::strip_newline(entry_point);
    }

private:
    bool is_sudo;
    std::string description;
    std::string working_directory;
    std::string entry_point;

    std::unordered_map<std::string, key_action_fptr> key_action;
}; // class Systemd
}; // namespace asyd
