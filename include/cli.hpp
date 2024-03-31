#pragma once

#include <string>
#include <filesystem>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstring>

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

    bool start_service(const std::string& project_name) const;
    bool stop_service(const std::string& project_name) const;
    bool restart_service(const std::string& project_name) const;

private:
    std::string get_home_dir() const;
    std::string get_asyd_dir() const;
    std::string get_asyd_project_dir(const std::string& projct_name) const;

    // start/stop/restart service
    bool service_action(const std::string& project_name, const std::string& action) const;
}; // class CLI
}; // namespace asyd
