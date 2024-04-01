#pragma once

#include <string>

namespace asyd
{
class Server
{
public:
    Server() {}
    Server(const std::string& hostname);

    void set_hostname(const std::string& hostname);

    void set_is_root(bool is_root);

    // Fetches the service user's home directory
    // and the server's bash directory needed to
    // create the systemd service file
    bool fetch_info();

    bool create_directory(const std::string& path) const;
    bool remove_directory(const std::string& path) const;

    bool chmod(
        const std::string& chmod_options,
        const std::string& target_file) const;

    bool copy_from_local(
        const std::string& from_local_path,
        const std::string& to_server_path) const;

    bool copy_systemd_file(
        const std::string& local_directory,
        const std::string& service_name) const;

    bool reload_service() const;
    bool enable_service(const std::string& service_name) const;
    bool start_service(const std::string& service_name) const;
    bool stop_service(const std::string& service_name) const;
    bool restart_service(const std::string& service_name) const;
    bool remove_service(const std::string& service_name) const;

    // check the status of a service and write it into [output]
    bool check_status(const std::string& service_name, std::string& output) const;

    const std::string& get_home() const;
    const std::string& get_bash() const;

private:
    std::string hostname;
    std::string home_directory;
    std::string bash_directory;

    bool is_root;

    bool systemd_action(const std::string& action, const std::string& service_name) const;
}; // class Server
}; // namespace asyd
