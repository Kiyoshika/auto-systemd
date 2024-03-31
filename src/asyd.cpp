#include "cli.hpp"
#include "argparse.hpp"

using namespace asyd;

int main(int argc, char** argv)
{
    CLI cli;

    if (argc == 4 && std::string(argv[1]) == "new")
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

        return 0;
    }
    else if (argc == 3 && std::string(argv[1]) == "remove")
    {
        std::string project_name = std::string(argv[2]);
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
