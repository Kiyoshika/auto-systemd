#pragma once

#include <string>
#include <filesystem>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>

namespace asyd
{
// forward declarations
class Config;
class Server;

class CLI
{
public:
    CLI() {}

    void generate_config(
        asyd::Config& config,
        const std::string& project_type,
        const std::string& path) const;

    bool create_project_dir(
        const std::string& project_name,
        const std::string& project_type) const;

    bool remove_project(const std::string& project_name) const;

private:
    std::string get_home_dir() const;
}; // class CLI
}; // namespace asyd
