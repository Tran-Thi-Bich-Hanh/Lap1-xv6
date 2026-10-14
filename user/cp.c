#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    int fd_src, fd_dst;
    int n;
    char buf[BUF_SIZE];

    // Kiểm tra số lượng đối số
    if (argc != 3) {
        fprintf(2, "usage: cp src dst\n");
        exit(1);
    }

    // Mở file nguồn (read-only)
    if ((fd_src = open(argv[1], O_RDONLY)) < 0) {
        fprintf(2, "cp: cannot open %s\n", argv[1]);
        exit(1);
    }

    // Mở file đích (tạo mới hoặc ghi đè)
    if ((fd_dst = open(argv[2], O_WRONLY | O_CREATE | O_TRUNC)) < 0) {
        fprintf(2, "cp: cannot open/create %s\n", argv[2]);
        close(fd_src);
        exit(1);
    }

    // Copy dữ liệu theo buffer
    while ((n = read(fd_src, buf, sizeof(buf))) > 0) {
        if (write(fd_dst, buf, n) != n) {
            fprintf(2, "cp: write error to %s\n", argv[2]);
            close(fd_src);
            close(fd_dst);
            exit(1);
        }
    }

    // Kiểm tra lỗi khi đọc
    if (n < 0) {
        fprintf(2, "cp: read error from %s\n", argv[1]);
        close(fd_src);
        close(fd_dst);
        exit(1);
    }

    // Đóng file descriptors
    close(fd_src);
    close(fd_dst);
    exit(0);
}