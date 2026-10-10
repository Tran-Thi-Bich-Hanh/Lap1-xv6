#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

#define MAX_ENTRIES 128
#define MAX_DEPTH 64

struct entry_info {
    char name[DIRSIZ + 1];
    short type;
};

// Hàm duyệt đệ quy in cây thư mục
void tree_rec(char *path, int current_depth, int max_depth, int only_dir, int is_last_level[MAX_DEPTH]) {
    int fd;
    struct stat st;
    struct dirent de;
    struct entry_info entries[MAX_ENTRIES];
    int count = 0;

    // Giới hạn độ sâu đệ quy (-L)
    if (max_depth != -1 && current_depth >= max_depth) {
        return;
    }

    if ((fd = open(path, 0)) < 0) {
        fprintf(2, "tree: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0) {
        fprintf(2, "tree: cannot stat %s\n", path);
        close(fd);
        return;
    }

    if (st.type != T_DIR) {
        close(fd);
        return;
    }

    // Đọc tất cả các entry hợp lệ trong thư mục hiện tại
    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
        if (de.inum == 0)
            continue;
        
        // Bỏ qua "." và ".."
        if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
            continue;

        // Tạo đường dẫn con để kiểm tra stat (loại file hay directory)
        char subpath[512];
        if (strlen(path) + 1 + strlen(de.name) + 1 > sizeof(subpath))
            continue;

        strcpy(subpath, path);
        int len = strlen(subpath);
        if (len > 0 && subpath[len - 1] != '/') {
            subpath[len] = '/';
            subpath[len + 1] = 0;
        }
        safestrcpy(subpath + strlen(subpath), de.name, DIRSIZ + 1);

        struct stat subst;
        if (stat(subpath, &subst) < 0)
            continue;

        // Nếu có cờ -d, chỉ xét thư mục con
        if (only_dir && subst.type != T_DIR)
            continue;

        if (count < MAX_ENTRIES) {
            safestrcpy(entries[count].name, de.name, DIRSIZ + 1);
            entries[count].type = subst.type;
            count++;
        }
    }
    close(fd);

    // In từng entry và đệ quy vào các thư mục con
    for (int i = 0; i < count; i++) {
        int is_last = (i == count - 1);
        is_last_level[current_depth] = is_last;

        // In các tiền tố thụt lề
        for (int j = 0; j < current_depth; j++) {
            if (is_last_level[j]) {
                printf("    ");
            } else {
                printf("│   ");
            }
        }

        // In nhánh kết nối hiện tại
        if (is_last) {
            printf("└── ");
        } else {
            printf("├── ");
        }

        printf("%s\n", entries[i].name);

        // Đệ quy nếu entry là thư mục
        if (entries[i].type == T_DIR) {
            char subpath[512];
            strcpy(subpath, path);
            int len = strlen(subpath);
            if (len > 0 && subpath[len - 1] != '/') {
                subpath[len] = '/';
                subpath[len + 1] = 0;
            }
            safestrcpy(subpath + strlen(subpath), entries[i].name, DIRSIZ + 1);

            tree_rec(subpath, current_depth + 1, max_depth, only_dir, is_last_level);
        }
    }
}

int main(int argc, char *argv[]) {
    char *path = ".";
    int max_depth = -1; // -1: không giới hạn độ sâu
    int only_dir = 0;

    // Xử lý tham số dòng lệnh
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            only_dir = 1;
        } else if (strcmp(argv[i], "-L") == 0) {
            if (i + 1 < argc) {
                max_depth = atoi(argv[++i]);
            } else {
                fprintf(2, "usage: tree [path] [-L depth] [-d]\n");
                exit(1);
            }
        } else if (argv[i][0] != '-') {
            path = argv[i];
        } else {
            fprintf(2, "usage: tree [path] [-L depth] [-d]\n");
            exit(1);
        }
    }

    // In đường dẫn gốc
    printf("%s\n", path);

    int is_last_level[MAX_DEPTH];
    for (int i = 0; i < MAX_DEPTH; i++) {
        is_last_level[i] = 0;
    }

    tree_rec(path, 0, max_depth, only_dir, is_last_level);

    exit(0);
}