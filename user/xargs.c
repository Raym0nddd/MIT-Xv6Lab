#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"
#include "kernel/fs.h"

int main(int argc, char *argv[])
{
    int myArgc = argc - 1, P;
    char *myArgv[MAXARG] = {0}, *p;
    char ch, buf[512] = {0};

    for (int i = 1; i < argc; i++) // 复制基础的指令
        myArgv[i - 1] = argv[i];
    p = buf, P = 0;

    while (1)
    {
        if (!read(0, &ch, 1))
            break;

        if (ch == '\n')
        {
            buf[P] = 0;
            myArgv[myArgc++] = p;

            if (fork() == 0)
            {
                // for (int i = 0; i < myArgc; i++)
                //     printf("%s ", myArgv[i]);
                // printf("\n");
                exec(argv[1], myArgv);
                break;
            }

            myArgc = argc - 1;
            P = 0;
            p = buf;
            wait((int *)0);
        }
        else if (ch == ' ')
        {
            buf[P] = 0;
            myArgv[myArgc++] = p;
            p = &buf[++P];
        }
        else
            buf[P++] = ch;
    }

    exit(0);
}
