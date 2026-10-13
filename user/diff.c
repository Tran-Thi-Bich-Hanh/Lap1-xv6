#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    int f1, f2;
    int n1, n2;
    char buf1[100], buf2[100];

    int pos = 1;
    int quiet = 0;
    int diff = 0;

    if (argc != 3 && argc != 4) {
        printf("Usage: diff [-q] file1 file2\n");
        exit(1);
    }

    if (argc == 4) {
        if (strcmp(argv[1], "-q") != 0) {
            printf("Usage: diff [-q] file1 file2\n");
            exit(1);
        }

        quiet = 1;

        f1 = open(argv[2], O_RDONLY);
        f2 = open(argv[3], O_RDONLY);
    }
    else {
        f1 = open(argv[1], O_RDONLY);
        f2 = open(argv[2], O_RDONLY);
    }

    if (f1 < 0 || f2 < 0) {
        printf("Failed to open\n");

        if (f1 >= 0)
            close(f1);

        if (f2 >= 0)
            close(f2);

        exit(1);
    }

    while (1) {

        n1 = readline(f1, buf1, sizeof(buf1));
        n2 = readline(f2, buf2, sizeof(buf2));

        if (n1 <= 0 && n2 <= 0) {
            break;
        }

        if (n1 <= 0) {
            diff = 1;

            if (!quiet) {
                printf("f1:%d: < EOF\n", pos);
                printf("f2:%d: > %s", pos, buf2);
            }

            pos++;
            continue;
        }

        if (n2 <= 0) {
            diff = 1;

            if (!quiet) {
                printf("f1:%d: < %s", pos, buf1);
                printf("f2:%d: > EOF\n", pos);
            }

            pos++;
            continue;
        }

        if (strcmp(buf1, buf2) != 0) {
            diff = 1;

            if (!quiet) {
                printf("f1:%d: < %s", pos, buf1);
                printf("f2:%d: > %s", pos, buf2);
            }
        }

        pos++;
    }

    if (quiet) {
        if (diff)
            printf("diff: files differ\n");
    }

    close(f1);
    close(f2);

    exit(0);
}