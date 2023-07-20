#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void find(const char *path, const char *findName)
{
    // printf("good--2\n");

    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(path, 0)) < 0)
    {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    strcpy(buf, path); // 复制当前路径到buf
    p = buf + strlen(buf);
    *p++ = '/';

    while (read(fd, &de, sizeof(de)) == sizeof(de)) // 遍历当前文件夹
    {
        if (de.inum == 0) // 无效引用
            continue;

        memmove(p, de.name, strlen(de.name));
        p[strlen(de.name)] = 0;

        if (stat(buf, &st) < 0)
        {
            printf("find: cannot stat %s\n", buf);
            continue;
        }

        switch (st.type)
        {
        case T_FILE:
            if (!strcmp(de.name, findName))
                printf("%s\n", buf);
            break;

        case T_DIR:
            if (strcmp(de.name, ".") && strcmp(de.name, ".."))
                find(buf, findName);
            break;
        }
    }

    close(fd);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("too less argument\n");
        exit(1);
    }

    char buf[512], *findName, *p;
    const char *startName;
    int fd;
    struct dirent de;
    struct stat st;

    if (argc == 2)
    {
        fd = open(".", 0);
        startName = ".";
        findName = argv[1];
    }
    else
    {
        fd = open(argv[1], 0);
        startName = argv[1];
        findName = argv[2];
    }

    // printf("good\n");

    if (fd < 0)
    {
        fprintf(2, "find: cannot open %s\n", argv[1]);
        exit(1);
    }

    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "find: cannot stat %s\n", argv[1]);
        close(fd);
        exit(1);
    }

    switch (st.type)
    {
    case T_FILE: // 不是文件
        printf("find: %s is not a dir\n", startName);
        break;

    case T_DIR:
        strcpy(buf, startName); // 复制当前路径到buf
        p = buf + strlen(buf);
        *p++ = '/';

        while (read(fd, &de, sizeof(de)) == sizeof(de)) // 遍历当前文件夹
        {
            if (de.inum == 0) // 无效引用
                continue;
            memmove(p, de.name, strlen(de.name));
            p[strlen(de.name)] = 0;

            if (stat(buf, &st) < 0)
            {
                printf("find: cannot stat %s\n", buf);
                continue;
            }

            switch (st.type)
            {
            case T_FILE:
                if (!strcmp(de.name, findName))
                    printf("%s\n", buf);
                break;

            case T_DIR:
                if (strcmp(de.name, ".") && strcmp(de.name, ".."))
                    find(buf, findName);
                break;
            }
        }

        break;
    }

    // printf("good3\n");
    close(fd);
    exit(0);
}