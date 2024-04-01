#include "cli.hpp"
#include "argparse.hpp"
#include "server.hpp"

using namespace asyd;

int main(int argc, char** argv)
{
    CLI cli;

    /* FOUR ARGUMENT COMMANDS */
    if (argc == 4)
    {
        std::string action = std::string(argv[1]);

        if (action == "new")
        {
            std::string project_type = std::string(argv[2]);
            if (project_type != "server" && project_type != "job")
            {
                std::cerr << "Invalid project type '" << project_type << "' - must be one of 'server' or 'job'.\n";
                return -1;
            }

            std::string project_name = std::string(argv[3]);
            if (!cli.create_project_dir(project_name, project_type))
            {
                std::cerr << "Failed to create project '" << project_name << "'. Does it already exist?\n";
                return -1;
            }
        }
        else
        {
            std::cerr << "Unknown command.\n";
            return -1;
        }
    }

    /* THREE ARGUMENT COMMANDS */
    else if (argc == 3)
    {
        std::string action = std::string(argv[1]);
        std::string project_name = std::string(argv[2]);

        if (action == "remove")
        {
            if (project_name.find("..") != std::string::npos || project_name.find("/") != std::string::npos)
            {
                std::cerr << "What are you trying to do with that project name...?\n";
                return -1;
            }

            if (!cli.remove_project(project_name))
            {
                std::cerr << "Failed to remove project '" << project_name << "'.\n";
                return -1;
            }
        }
        else if (action == "start")
        {
            if (!cli.start_service(project_name))
            {
                std::cerr << "Couldn't start service '" << project_name << "'.\n";
                return -1;
            }
        }
        else if (action == "stop")
        {
            if (!cli.stop_service(project_name))
            {
                std::cerr << "Couldn't stop service '" << project_name << "'.\n";
                return -1;
            }
        }
        else if (action == "restart")
        {
            if (!cli.restart_service(project_name))
            {
                std::cerr << "Couldn't restart service '" << project_name << "'.\n";
                return -1;
            }
        }
        else if (action == "status")
        {
            std::string status;
            if (!cli.check_status(project_name, status))
            {
                std::cerr << "Couldn't check status for service '" << project_name << "'.\n";
                return -1;
            }

            std::cout << status << "\n";
        }
        else if (action == "ls")
        {
            std::string hostname = std::string(argv[2]);
            Server server(hostname);

            std::string output;
            if (!server.list_services(output))
            {
                std::cerr << "Couldn't list services.\n";
                return -1;
            }

            std::cout << output << "\n";
        }
        else
        {
            std::cerr << "Unknown command.\n";
            return -1;
        }
    }

    /* OPTIONAL POSITION ARGUMENT COMMANDS */
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

    return 0;
}
