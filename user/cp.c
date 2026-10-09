#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

char buf[512]; 

int main(int argc, char *argv[]) {
    int fd_src, fd_dst;
    int n;

    if (argc != 3) {
        fprintf(2, "usage: cp src dst\n"); //
        exit(1);
    }
c
    while ((n = read(fd_src, buf, sizeof(buf))) > 0) {
        if (write(fd_dst, buf, n) != n) {
            fprintf(2, "cp: write error\n");
            exit(1);
        }
    }

    if (n < 0) {
        fprintf(2, "cp: read error\n");
        exit(1);
    }

    close(fd_src);
    close(fd_dst);

    exit(0);
}