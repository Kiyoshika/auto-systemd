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
        .add(this->hostname)
        .addQuote()
        .add("pwd ~", false)
        .addQuote();

    if (!command.execute())
        return false;

    this->home_directory = command.get_output();

    // get bash directory
    command.add("ssh")
        .add(this->hostname)
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

bool Server::create_directory(const std::string& path) const
{
    Command command;

    command.add("ssh")
        .add(this->hostname)
        .addQuote()
        .add("mkdir -p")
        .add(path, false)
        .addQuote();

    if (!command.execute())
        return false;

    return true;
}

bool Server::remove_directory(const std::string& path) const
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
    const std::string& to_server_path) const
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
    const std::string& target_file) const
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
    const std::string& service_name) const
{
    Command command;

    command.add("scp")
        .add(local_directory, false)
        .add("/", false)
        .add(service_name)
        .add(this->hostname, false)
        .add(":", false);

    if (this->is_root)
        command.add("/etc/systemd/system/", false);
    else
        command.add("~/.config/systemd/user/", false);

    command.add("asyd-" + service_name, false);

    if (!command.execute())
        return false;

    return true;
}

bool Server::systemd_action(const std::string& action, const std::string& service_name) const
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
        command.add("asyd-" + service_name, false);

    command.addQuote();

    if (!command.execute())
        return false;

    return true;
}

bool Server::reload_service() const
{
    return this->systemd_action("daemon-reload", "");
}

bool Server::enable_service(const std::string& service_name) const
{
    return this->systemd_action("enable", service_name);
}

bool Server::start_service(const std::string& service_name) const
{
    return this->systemd_action("start", service_name);
}

bool Server::stop_service(const std::string& service_name) const
{
    return this->systemd_action("stop", service_name);
}

bool Server::restart_service(const std::string& service_name) const
{
    return this->systemd_action("restart", service_name);
}

bool Server::remove_service(const std::string& service_name) const
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

    command.add("asyd-" + service_name, false)
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

bool Server::check_status(const std::string& service_name, std::string& output) const
{
    Command command;

    command.add("ssh")
        .add(this->hostname)
        .addQuote()
        .add("systemctl");

    if (!this->is_root)
        command.add("--user");

    command.add("status")
        .add("asyd-" + service_name, false)
        .addQuote();

    if (!command.execute())
        return false;

    output = command.get_output();
    return true;
}

bool Server::list_services(std::string& output) const
{
    Command command;

    command.add("ssh")
        .add(this->hostname)
        .addQuote()
        .add("systemctl --user --type=service --all", false)
        .addQuote();

    if (!command.execute())
        return false;

    output = command.get_output();
    output = this->filter_service_list_output(output);
    return true;
}

std::vector<std::string> Server::split_newlines(const std::string& command_output) const
{
    std::vector<std::string> tokens;
    std::string current_token = "";

    for (const char c : command_output)
    {
        if (c == '\n')
        {
            if (current_token.length() > 0)
            {
                tokens.push_back(current_token);
                current_token = "";
            }

            continue;
        }

        current_token += c;
    }

    if (current_token.length() > 0)
        tokens.push_back(current_token);

    return tokens;
}

bool Server::service_is_loaded(const std::string& service_string) const
{
    size_t idx = 0;

    // services with a "dot" at the begining are services that once existed
    // but have been removed
    if (service_string[0] == '\xe2')
        return false;

    // consume any leading space before service name
    while (service_string[idx] == ' ' || service_string[idx] == '\t')
        idx++;

    // find position of first space/tab
    std::string service_name = "";
    while (service_string[idx] != ' ' && service_string[idx] != '\t')
        service_name += service_string[idx++];

    // only care about "asyd-" services
    if (!(service_name.length() >= 5 && strncmp(service_name.c_str(), "asyd-", 5) == 0))
        return false;

    // consume spaces/tabs
    while (service_string[idx] == ' ' || service_string[idx] == '\t')
        idx++;

    // extract text until first space
    std::string text = "";
    while (service_string[idx] != ' ' && service_string[idx] != '\t')
        text += service_string[idx++];

    return text == "loaded";
}

std::string Server::filter_service_list_output(const std::string& command_output) const
{
    std::vector<std::string> services = this->split_newlines(command_output);

    std::string output = services[0];

    for (size_t i = 1; i < services.size() - 1; ++i)
        if (this->service_is_loaded(services[i]))
            output += ("\n   *   " + services[i].substr(7)); // strips the asyd- prefix

    return output + "\n";
}
