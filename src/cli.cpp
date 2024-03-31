#include "cli.hpp"
#include "config.hpp"
#include "server.hpp"
#include "systemd.hpp"

using namespace asyd;

// there is currently no portable way (that I know of) to do this as of C++17
std::string CLI::get_home_dir() const
{
    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        return std::string(std::getenv("HOMEDRIVE")) + std::string(std::getenv("HOMEPATH"));
    #else
        return std::string(std::getenv("HOME"));
    #endif
}

void CLI::generate_config(
    Config& config,
    const std::string& project_type,
    const std::string& path) const
{
    std::string entry = "";

    /* PROJECT DESCRIPTION */
    std::cout << "\nProject Description: ";
    std::getline(std::cin, entry);
    config.set_project_description(entry);

    /* SERVICE USERNAME */
    std::cout << "\nService username (the username on the remote server that will run the services) (DEFAULT: sudo): ";
    std::getline(std::cin, entry);
    if (entry == "")
        entry = "sudo";
    config.set_service_username(entry);

    /* SERVER HOSTNAME */
    std::cout << "\nServer hostname (e.g., you@yourserverip): ";
    std::getline(std::cin, entry);
    config.set_server_hostname(entry);

    /* WORKING DIRECTORY */
    std::cout << "\nFull working directory to your project (e.g., /home/you/myproject): ";
    std::getline(std::cin, entry);
    if (entry.length() > 0 && entry[0] != '/')
    {
        std::cerr << "FULL WORKING DIRECTORY MUST START WITH A FORWARD SLASH '/'.\n";
        std::filesystem::remove_all(path);
        return;
    }
    config.set_working_directory(entry);

    /* ENTRY POINT */
    std::cout << "\nEntry point script relative to working directory (e.g., build/start.sh): ";
    std::getline(std::cin, entry);
    config.set_entry_point(entry);

    /* SCHEDULE */
    if (project_type == "job")
    {
        std::cout << "\nJob schedule in systemd's OnCalendar format (e.g., Mon,Fri *-*-* 06:00:00): ";
        std::getline(std::cin, entry);
        config.set_schedule(entry);
    }

    Server server(config.get_server_hostname());
    if (!server.fetch_info())
    {
        std::cerr << "THERE WAS A PROBLEM FETCHING SERVER INFO FROM '" << config.get_server_hostname() << "'.\n";
        std::filesystem::remove_all(path);
        return;
    }
    config.set_server_home_directory(server.get_home());
    config.set_server_bash_directory(server.get_bash());

    if (!config.to_file(path + "/config.cfg"))
    {
        std::cerr << "THERE WAS A PROBLEM WRITING CONFIG FILE TO '" << path << "'.\n";
        std::filesystem::remove_all(path);
        return;
    }

    Systemd service;
    service.from_config(config);

    if (!service.to_file(path + "/" + config.get_project_name() + ".service"))
    {
        std::cerr << "THERE WAS A PROBLEM WRITING SYSTEMD SERVICE TO '" << path << "'.\n";
        std::filesystem::remove_all(path);
        return;
    }

    if (!config.setup_server(path))
    {
        std::cerr << "THERE WAS A PROBLEM SETTING UP THE SERVER. CHECK YOUR CONNECTION SETTINGS.\n";
        std::filesystem::remove_all(path);
        return;
    }

    config.to_file(path + "/config.cfg");
    std::cout << "\nSUCCESSFULLY CONFIGURED SERVER AND WROTE LOCAL CONFIG TO '" << path << "'.\n";
}

bool CLI::create_project_dir(
    const std::string& project_name,
    const std::string& project_type) const
{
    std::string asyd_dir = get_home_dir() + "/.asyd/";
    if (!std::filesystem::exists(asyd_dir))
        std::filesystem::create_directory(asyd_dir);

    std::string project_dir = asyd_dir + project_name;
    if (std::filesystem::exists(project_dir))
        return false;

    std::filesystem::create_directory(project_dir);

    if (project_type == "server")
    {
        Config config;
        config.set_project_name(project_name);
        generate_config(config, project_type, project_dir);
    }
    else if (project_type == "job")
    {
        Config config;
        config.set_project_name(project_name);
        generate_config(config, project_type, project_dir);
    }
    else
        return false;

    return true;
}

bool CLI::remove_project(const std::string& project_name) const
{
    std::string project_home_dir = get_home_dir() + "/.asyd/" + project_name;
    if (!std::filesystem::exists(project_home_dir))
        return false;

    Config config;
    config.from_file(project_home_dir + "/config.cfg");

    Server server(config.get_server_hostname());
    if (!server.remove_directory("~/.asyd/" + project_name))
        return false;

    if (!server.remove_service(project_name + ".service"))
        return false;

    std::filesystem::remove_all(project_home_dir);

    std::cout << "SUCCESSFULLY REMOVED PROJECT '" << project_name << "' FROM SERVER AND LOCAL CONFIG.\n";
    return true;
}
