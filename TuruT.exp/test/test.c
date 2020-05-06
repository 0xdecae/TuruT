#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>

int main()
{
	setuid(0);
	char* cmd = "echo hello";
	//char* args[] = {"/home/zathras/fungus/config.sh"}
	execv(cmd, NULL, NULL);
	return 0;
}
