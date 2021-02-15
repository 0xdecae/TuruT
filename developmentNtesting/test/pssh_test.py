import sys, os, string, threading
import paramiko

cmd = "uname -a"

outlock = threading.Lock()

def workon(host):

    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    ssh.connect(host, username="zathras", password="password1!", look_for_keys=False, allow_agent=False)

    stdin, stdout, stderr = ssh.exec_command(cmd)
    stdin.write('xy\n')
    stdin.flush()

    with outlock:
        print stdout.readlines()

def main():
    hosts = ['192.168.17.155', '192.168.17.156', '192.168.17.157', ] # etc
    threads = []
    for h in hosts:
        t = threading.Thread(target=workon, args=(h,))
        t.start()
        threads.append(t)
    for t in threads:
        t.join()

main()
