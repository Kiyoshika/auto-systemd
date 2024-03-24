# Creating a Service Account for Asyd
This is a short guide on how to setup a "service account" for `asyd` to properly setup configuration without using a root user.

The below steps will need to be performed on your remote server as a sudo user.

1. Create the service account, in this case we'll call it `serviceuser`
    * `sudo adduser serviceuser`
2. Allow `serviceuser` to connect via ssh (this is how `asyd` will connect to your server)
    * `mkdir /home/serviceuser/.ssh`
    * `cp ~/.ssh/authorized_keys /home/serviceuser/.ssh`
3. Create user-specific systemd directory
    * `mkdir -p /home/serviceuser/.config/systemd/user`
4. Give necessary permissions to the service account's home directory
    * `chown -R serviceuser:serviceuser /home/serviceuser`
    * `chmod -R u+rwx /home/serviceuser`

After following these steps, you should be able to use `asyd` with your new service account!
