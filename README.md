# Auto Systemd (asyd) - Pronounced 'acid'
This project is to simplify the process of "deploying" code to a **linux** server that you have access to.

Typically systemd services/timers are used to either run servers or schedule jobs, but setting them up and deploying changes can be a bit tedious. That's where this tool comes to bridge the gap.

## How it works
`asyd` works by building a project-specific config file which attaches key things such as the hostname, the path to the executable script, etc. to build a systemd service file(s) which are then copied to the remote server along with the executable and any specified directories.

When making "deployments", the new file(s) are copied to the remote server and the service is restarted.

## Server Setup (Prerequisites)
Most people are probably not comfortable with `asyd` using `sudo`, so instead I'd heavily recommend [following this guide](./CREATING_A_SERVICE_ACCOUNT.md) to making a service account for `asyd`.

## WARNING
This is probably obvious, but be mindful about architectural differences if you are building code on, say, a Mac and using `asyd` to deploy the project on a linux server - that executable is NOT going to run properly unless you cross compile.

## Installation
1. `git clone --recursive git@github.com:Kiyoshika/auto-systemd.git`
2. `make` to build the `asyd` executable into the `./out` directory
3. Add the `./out` directory to your path to use `asyd` anywhere

## 1.0 Roadmap
This is the general roadmap to target a "1.0" usable release - all the basic core features to have a functioning command line tool (not necessarily in order):
* ~Create a server~
* Create a job
* ~Remove a project~
* Implement all config command line options
* Add help option to command line
* ~Check status of service~
* ~Start/Stop/Restart a service~
* List all asyd services
* Pull all asyd services from server
* Include a full walkthrough example on deploying a Flask server

## Usage
Before reading on, this assumes you have a working server to connect to via SSH with your own public/private keys setup. E.g., `ssh you@yourserver`

### General
* List all your current projects and whether or not they're currently running:
    * `asyd ls`
* Fetch the status of a service:
    * `asyd status your-project-name`
* Start/stop/restart a service
    * `asyd [start|stop|restart] your-project-name`
* Pull all services from the server to your local system (WARNING: this will overwrite any local service(s) with the same name)
    * `asyd pull you@yourserver`
* Pull a specific service from the server to your local system (WARNING: this will overwrite any local service with the same name)
    * `asyd pull your-project-name you@yourserver`

### Creating a New Project
There are two different types of services: servers and jobs. A server is a continuously running process while a job is a process that is executed on a schedule.

* Create a new server with: `asyd new server your-server-name`
* Create a new job with: `asyd new job your-job-name`
* Delete a project with: `asyd remove your-project-name` - it doesn't matter if it was created as a server or job.

When creating a new project, the CLI will walk you through the initial setup to create the starting config, but these options can be changed at anytime as seen below.

#### Common Config Options
These are config options that apply to both servers and jobs.

Note that you can specify more than one option at a time, but they're listed individually for clarity.
* `-d`: Set the description of the systemd service
    * `asyd -P your-project-name -d "My description of the service"`
* `-u`: Set the username that will be running the service on the remote server. The server admin needs to set this up beforehand
    * `asyd -P your-project-name -u dedicated-user`
* `-h`: Set the hostname of the server you are deploying to
    * `asyd -P your-project-name -h you@yourserver`
* `-D`: Set the path to a directory to copy over to the remote server to make the working directory for the service
    * `asyd -P your-project-name -D /full/local/path/to/my-project`
* `-e`: Set the entry point for the service - i.e., the shell script that runs the server or job. The path is relative to what's set by `-D`.
    * `asyd -P your-project-name -e build/run.exe` - an example for running a compiled executable
* `-r`: Rename the project
    * `asyd -P your-project-name -r new-project-name`

#### Job Config Options
These are config options that apply only to jobs and have no effect if they're applied on a server.
* `-s`: Set the schedule for the job which follows the [OnCalendar](https://silentlad.com/systemd-timers-oncalendar-(cron)-format-explained) format (don't pay attention to the CRON format in the listed article.)
    * `asyd -P your-project-name -s "Mon,Fri *-*-* 06:00:00"` - sets the schedule for every Monday and Friday at 6am (server time)
