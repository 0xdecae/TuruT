#include <stdlib.h>
int main()
{
        setuid(0);
        system("/home/zathras/fungus/config.sh");
        return 0;
}

