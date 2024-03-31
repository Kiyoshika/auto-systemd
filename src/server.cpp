#include "server.hpp"
#include "command.hpp"

using namespace asyd;

Server::Server(const std::string& hostname)
{
    this->hostname = hostname;
    this->fetch_info();
}

void Server::set_is_root(bool is_root)
{
    this->is_root = is_root;
}

bool Server::fetch_info()
{
    Command command;
    
    // get home directory
    command.add("ssh")
        .add(hostname)
        .addQuote()
        .add("pwd ~", false)
        .addQuote();

    if (!command.execute())
        return false;

    this->home_directory = command.get_output();

    // get bash directory
    command.add("ssh")
        .add(hostname)
        .addQuote()
        .add("whereis bash", false)
        .addQuote();

    if (!command.execute())
        return false;

    // this returns a full "bash: /usr/bin/bash /maybe/something/else"
    // but we only need the first directory after "bash: "
    const std::string& full_bash_dir = command.get_output();
    std::string bash_dir = "";
    for (size_t i = 6; i < full_bash_dir.length(); ++i)
    {
        if (full_bash_dir[i] == ' ')
            break;

        bash_dir += full_bash_dir[i];
    }
    this->bash_directory = bash_dir;

    return true;
}

void Server::set_hostname(const std::string& hostname)
{
    this->hostname = hostname;
}

bool Server::create_directory(const std::string& path)
{
    Command command;

    command.add("ssh")
        .add(hostname)
        .addQuote()
        .add("mkdir -p")
        .add(path, false)
        .addQuote();

    if (!command.execute())
        return false;

    return true;
}

bool Server::remove_directory(const std::string& path)
{
    // NOTE: the path is sanitized beforehand
    Command command;

    command.add("ssh")
        .add(this->hostname)
        .addQuote()
        .add("rm -rf")
        .add(path, false)
        .addQuote();

    if (!command.execute())
        return false;

    return true;
}

bool Server::copy_from_local(
    const std::string& from_local_path,
    const std::string& to_server_path)
{
    Command command;

    command.add("scp")
        .add("-rp")
        .add(from_local_path)
        .add(this->hostname, false)
        .add(":", false)
        .add(to_server_path);

    if (!command.execute())
        return false;

    return true;
}

bool Server::chmod(
    const std::string& chmod_options,
    const std::string& target_file)
{
    Command command;

    command.add("ssh")
        .add(this->hostname)
        .addQuote()
        .add("chmod")
        .add(chmod_options)
        .add(target_file, false)
        .addQuote();

    if (!command.execute())
        return false;

    return true;
}

bool Server::copy_systemd_file(
    const std::string& local_directory,
    const std::string& service_name)
{
    Command command;

    command.add("scp")
        .add(local_directory, false)
        .add("/", false)
        .add(service_name)
        .add(this->hostname, false)
        .add(":", false);

    if (this->is_root)
        command.add("/etc/systemd/system/", false).add(service_name, false);
    else
        command.add("~/.config/systemd/user/", false).add(service_name, false);

    if (!command.execute())
        return false;

    return true;
}

bool Server::systemd_action(const std::string& action, const std::string& service_name)
{
    Command command;

    command.add("ssh")
        .add(this->hostname)
        .addQuote()
        .add("systemctl");

    if (!this->is_root)
        command.add("--user");
    
    command.add(action, service_name.length() > 0);

    if (service_name.length() > 0)
        command.add(service_name, false);

    command.addQuote();

    if (!command.execute())
        return false;

    return true;
}

bool Server::reload_service()
{
    return this->systemd_action("daemon-reload", "");
}

bool Server::enable_service(const std::string& service_name)
{
    return this->systemd_action("enable", service_name);
}

bool Server::start_service(const std::string& service_name)
{
    return this->systemd_action("start", service_name);
}

bool Server::stop_service(const std::string& service_name)
{
    return this->systemd_action("stop", service_name);
}

bool Server::restart_service(const std::string& service_name)
{
    return this->systemd_action("restart", service_name);
}

bool Server::remove_service(const std::string& service_name)
{
    if (!this->stop_service(service_name))
        return false;

    Command command;

    command.add("ssh")
        .add(this->hostname)
        .addQuote();

    if (this->is_root)
        command.add("rm /etc/systemd/system/", false);
    else
        command.add("rm ~/.config/systemd/user/", false);

    command.add(service_name, false)
        .addQuote();

    if (!command.execute())
        return false;

    return true;
}

const std::string& Server::get_home() const
{
    return this->home_directory;
}

const std::string& Server::get_bash() const
{
    return this->bash_directory;
}
