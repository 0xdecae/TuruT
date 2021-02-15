#usr/bin/python

import time, os, sys, subprocess, paramiko, select

hosts = ['192.168.17.154']

def main():

    for ip in hosts:
        print "Attempting to ssh into {}".format(ip)
        ssh_exec(ip)
        print "Finished ssh_exec for {}".format(ip)

    exit(0)


def ssh_exec(host):


    client = paramiko.SSHClient()
    client.set_missing_host_key_policy(paramiko.client.AutoAddPolicy())
    client.connect(host, username="deadbeef", password="deadbeef", look_for_keys=False, allow_agent=False)
    print "Finished setting up sshclient for {}".format(host)

    channel = client.invoke_shell()
    stdin = channel.makefile('wb')
    stdout = channel.makefile('rb')

    # Send the command (non-blocking)
    stdin, stdout, stderr = client.exec_command("cat /etc/passwd")

    # Wait for the command to terminate
    while not stdout.channel.exit_status_ready():
        # Only print data if there is data to read in the channel
        if stdout.channel.recv_ready():
            rl, wl, xl = select.select([stdout.channel], [], [], 0.0)
            if len(rl) > 0:
                # Print data from stdout
                print stdout.channel.recv(1024),


    client.close()



if __name__ == "__main__":
    main()
