#include "config.hpp"
#include "argparse.hpp"
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

    /* PUBLIC KEY PATH */
    std::cout << "\nFull server public key path (e.g., /home/Users/dev/.ssh/your_key.pub): ";
    std::getline(std::cin, entry);
    config.set_public_key_path(entry);

    /* WORKING DIRECTORY */
    std::cout << "\nFull working directory to your project (e.g., /home/Users/dev/myproject): ";
    std::getline(std::cin, entry);
    config.set_working_directory(entry);

    /* ENTRY POINT */
    std::cout << "\nEntry point relative to working directory (e.g., ./build/myexe or python3 -m build): ";
    std::getline(std::cin, entry);
    config.set_entry_point(entry);

    /* PORT */
    if (config.get_type() == ConfigType::SERVER)
    {
        std::cout << "\nPort your server runs on (e.g., 5000 or 8080): ";
        std::getline(std::cin, entry);
        config.set_port(entry);
    }

    /* SCHEDULE */
    else if (config.get_type() == ConfigType::JOB)
    {
        std::cout << "\nJob schedule in systemd's OnCalendar format (e.g., Mon,Fri *-*-* 06:00:00): ";
        std::getline(std::cin, entry);
        config.set_schedule(entry);
    }

    config.to_file(path + "/config.cfg");
    std::cout << "\nSUCCESSFULLY WRITTEN CONFIG TO '" << path << "'.\n";
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
        generate_config(config, project_dir);
    }
    else if (project_type == "job")
    {
        Config config(ConfigType::JOB);
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
