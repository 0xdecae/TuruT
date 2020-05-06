#include <unistd.h>

int main()
{
	setuid(0);
	char *args[] = {NULL, "/home/zathras/fungus/config.sh", NULL};
	args[0]="/bin/sh";
	execve("/bin/sh", args, NULL);
	return 0;
}
