#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int mySleep(int time)
{
    return sleep(time);
}

int main(int argc, char *argv[])
{
    if(argc <= 1)
    {
        printf("lack of sleep time\n");
        exit(1);
    }

    int tickCounts = atoi(argv[1]);

    if(tickCounts <= 0)
    {
        printf("sleep time is illegal\n");
        exit(1);
    }

    // printf("%d\n",tickCounts);
    mySleep(tickCounts);
    exit(0);
}