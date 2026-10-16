#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

void print_indent(int level) {
  for (int i = 0; i < level - 1; i++) {
    printf("|   ");
  }
  printf("|-- ");
}

void tree(char *path, int level, int max_depth, int only_dir) {
  if (max_depth != -1 && level > max_depth) {
    return;
  }

  int fd;
  struct stat st;
  struct dirent de;
  char buf[256], *p;

  if ((fd = open(path, O_RDONLY)) < 0) {
    fprintf(2, "tree: cannot open %s\n", path);
    return;
  }

  if (fstat(fd, &st) < 0 || st.type != T_DIR) {
    close(fd);
    return;
  }

  if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
    fprintf(2, "tree: path too long\n");
    close(fd);
    return;
  }

  strcpy(buf, path);
  p = buf + strlen(buf);
  *p++ = '/';

  while (read(fd, &de, sizeof(de)) == sizeof(de)) {
    if (de.inum == 0)
      continue;
    if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
      continue;

    // Đảm bảo tên file kết thúc bằng null an toàn
    memmove(p, de.name, DIRSIZ);
    p[DIRSIZ] = 0;

    int child_fd = open(buf, O_RDONLY);
    if (child_fd < 0) {
      continue;
    }

    struct stat st_child;
    if (fstat(child_fd, &st_child) < 0) {
      close(child_fd);
      continue;
    }
    close(child_fd);

    if (only_dir && st_child.type != T_DIR) {
      continue;
    }

    print_indent(level);
    printf("%s\n", p);

    if (st_child.type == T_DIR) {
      tree(buf, level + 1, max_depth, only_dir);
    }
  }
  close(fd);
}

int main(int argc, char *argv[]) {
  char *path = ".";
  int max_depth = -1;
  int only_dir = 0;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-d") == 0) {
      only_dir = 1;
    } else if (strcmp(argv[i], "-L") == 0) {
      if (i + 1 < argc) {
        max_depth = atoi(argv[++i]);
      } else {
        fprintf(2, "tree: -L requires a depth argument\n");
        exit(1);
      }
    } else if (argv[i][0] != '-') {
      path = argv[i];
    }
  }

  // Kiểm tra tính hợp lệ của path trước khi in
  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    fprintf(2, "tree: cannot open %s\n", path);
    exit(1);
  }
  struct stat st;
  if (fstat(fd, &st) < 0 || st.type != T_DIR) {
    fprintf(2, "tree: %s is not a directory\n", path);
    close(fd);
    exit(1);
  }
  close(fd);

  printf("%s\n", path);
  tree(path, 1, max_depth, only_dir);
  exit(0);
}