#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void primes(char *num)
{
    int index = 0;
    for (int i = 0; i < 34; i++)
        if (num[i])
        {
            index = i;
            break;
        }
    int currentNum = index + 2;
    num[index] = 0;

    printf("prime %d\n", currentNum); // 输出当前最小值（质数）

    for (int i = index + 1; i < 34; i++) // 清空该质数的所有倍数
        if (num[i])
        {
            num[i] = (i + 2) % currentNum;
        }

    // printf("currentNum is %d left nums are:\n", currentNum);
    // for (int i = index + 1; i < 34; i++)
    //     if (num[i])
    //         printf("%d ", i + 2);
    // printf("\n");

    for (int i = index + 1; i < 34; i++)
    {
        if (num[i])
        {
            // char tmp[34] = {0};
            int p[2];
            pipe(p);
            if (!fork())
            {
                close(p[1]);
                read(p[0], num, 34);

                // printf("read: ");
                // for (int j = 0; j < 34; j++)
                //     if (num[j])
                //         printf("%d ", j + 2);
                // printf("\n");

                primes(num);
                close(p[0]);
            }
            else
            {
                close(p[0]);

                // printf("write: ");
                // for (int j = 0; j < 34; j++)
                //     if (num[j])
                //         printf("%d ", j + 2);
                // printf("\n");

                write(p[1], num, 34);
                wait(&p[0]);
                close(p[1]);
            }
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    char num[34] = {0};
    int tmp;

    for (int i = 0; i < 34; i++)
    {
        num[i] = 1;
    }
    primes(num);

    wait(&tmp);
    exit(0);
}