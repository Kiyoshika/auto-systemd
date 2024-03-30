#include "config.hpp"
#include "argparse.hpp"
#include "systemd.hpp"
#include "server.hpp"

#include <filesystem>
#include <cstdlib>

using namespace asyd;

// there is currently no portable way to do this as of C++17
std::string get_home_dir()
{
    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        return std::string(std::getenv("HOMEDRIVE")) + std::string(std::getenv("HOMEPATH"));
    #else
        return std::string(std::getenv("HOME"));
    #endif
}

void generate_config(Config& config, const std::string& path)
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
    if (config.get_type() == ConfigType::JOB)
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

bool create_project_dir(const std::string& project_name, const std::string& project_type)
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
        Config config(ConfigType::SERVER);
        config.set_project_name(project_name);
        generate_config(config, project_dir);
    }
    else if (project_type == "job")
    {
        Config config(ConfigType::JOB);
        config.set_project_name(project_name);
        generate_config(config, project_dir);
    }
    else
        return false;

    return true;
}



int main(int argc, char** argv)
{
    if (argc == 4 && std::string(argv[1]) == "new")
    {
        std::string project_type = std::string(argv[2]);
        if (project_type != "server" && project_type != "job")
        {
            std::cerr << "Invalid project type '" << project_type << "' - must be one of 'server' or 'job'.\n";
            return -1;
        }

        std::string project_name = std::string(argv[3]);
        if (!create_project_dir(project_name, project_type))
        {
            std::cerr << "Failed to create project '" << project_name << "'. Does it already exist?\n";
            return -1;
        }

        return 0;
    }
    else if (argc == 3 && std::string(argv[1]) == "remove")
    {

    }
    else
    {
        argparse::ArgumentParser program("asyd");
        try
        {
            program.parse_args(argc, argv);
        }
        catch (const std::exception& ex)
        {
            std::cerr << ex.what() << "\n";
            return -1;
        }

        if (program["action"] == true)
        {
            std::cout << program.get<std::string>("action") << "\n";
        }
    }

    Config config(ConfigType::SERVER);
    config.to_file("test.cfg");

    return 0;
}
