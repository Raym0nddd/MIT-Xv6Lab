#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int main(int argc, char *argv[])
{
    int p[2];
    char signal[10] ={0};

    pipe(p);
    if(fork() == 0)
    {
        read(p[0], signal, 10);
        printf("<%d>:received %s\n", getpid(), signal);
        write(p[1], "pong", 10);
        close(p[0]);
        close(p[1]);
    }
    else
    {
        write(p[1], "ping", 10);
        wait(0);
        read(p[0], signal, 10);
        printf("<%d>:received %s\n", getpid(), signal);
        close(p[0]);
        close(p[1]);
    }

    exit(0);
}