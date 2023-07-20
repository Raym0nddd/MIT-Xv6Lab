#include "kernel/types.h"
#include "user/user.h"

#define RD 0
#define WT 1

int main(int argc, char *argv[])
{
    int c2p[2], p2c[2];
    char signal[10] ={0};

    pipe(c2p);
    pipe(p2c);
    if(fork() == 0)
    {
        close(c2p[RD]);
        close(p2c[WT]);
        read(p2c[RD], signal, 10);
        printf("%d: received %s\n", getpid(), signal);
        write(c2p[WT], "pong", 10);
        close(c2p[WT]);
        close(p2c[RD]);
    }
    else
    {
        close(c2p[WT]);
        close(p2c[RD]);
        write(p2c[WT], "ping", 10);
        wait(0);
        read(c2p[RD], signal, 10);
        printf("%d: received %s\n", getpid(), signal);
        close(c2p[RD]);
        close(p2c[WT]);
    }

    exit(0);
}