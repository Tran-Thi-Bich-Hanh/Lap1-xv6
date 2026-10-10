#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// Hàm in ra các khoảng trắng và ký tự nối dạng cây
void print_indent(int level, int *is_last) {
    for (int i = 0; i < level; i++) {
        if (i == level - 1) {
            if (is_last[i]) {
                printf("└── ");
            } else {
                printf("├── ");
            }
        } else {
            if (is_last[i]) {
                printf("    ");
            } else {
                printf("│   ");
            }
        }
    }
}

// Hàm đệ quy duyệt cây
void tree(char *path, int level, int maxDepth, int onlyDir, int *is_last) {
    if (maxDepth != -1 && level >= maxDepth) {
        return;
    }

    int fd;
    struct stat st;

    if ((fd = open(path, 0)) < 0) {
        fprintf(2, "tree: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0) {
        fprintf(2, "tree: cannot stat %s\n", path);
        close(fd);
        return;
    }

    if (st.type == T_DIR) {
        struct dirent de;
        // Dùng malloc để tránh tràn stack trong hàm đệ quy
        struct dirent *entries = malloc(256 * sizeof(struct dirent));
        if(entries == 0){
            fprintf(2, "tree: malloc failed\n");
            close(fd);
            return;
        }
        int count = 0;

        while (read(fd, &de, sizeof(de)) == sizeof(de)) {
            if (de.inum == 0)
                continue;
            
            if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                continue;

            char buf[256]; // Giảm kích thước buffer (xv6 dir paths thường không quá dài)
            strcpy(buf, path);
            char *p = buf + strlen(buf);
            *p++ = '/';
            strcpy(p, de.name);

            struct stat st2;
            if (stat(buf, &st2) < 0)
                continue;

            if (onlyDir && st2.type != T_DIR)
                continue;

            if (count < 256) {
                entries[count++] = de;
            }
        }
        close(fd);

        for (int i = 0; i < count; i++) {
            char buf[256];
            strcpy(buf, path);
            char *p = buf + strlen(buf);
            *p++ = '/';
            strcpy(p, entries[i].name);

            is_last[level] = (i == count - 1);
            print_indent(level + 1, is_last);
            printf("%s\n", entries[i].name);

            struct stat st2;
            if (stat(buf, &st2) >= 0 && st2.type == T_DIR) {
                tree(buf, level + 1, maxDepth, onlyDir, is_last);
            }
        }
        free(entries); // Giải phóng bộ nhớ sau khi đệ quy xong thư mục hiện tại
    } else {
        close(fd);
    }
}

int main(int argc, char *argv[]) {
    char *path = ".";
    int maxDepth = -1;
    int onlyDir = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-L") == 0 && i + 1 < argc) {
            maxDepth = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-d") == 0) {
            onlyDir = 1;
        } else {
            path = argv[i];
        }
    }

    int is_last[64]; 
    printf("%s\n", path);
    tree(path, 0, maxDepth, onlyDir, is_last);
    
    exit(0);
}