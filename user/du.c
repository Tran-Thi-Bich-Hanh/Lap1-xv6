
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define MAXPATH 512

int opt_a = 0;
int opt_s = 0;

int makepath(char *buf, char *path, char *name)
{
    int i = 0;
    int j = 0;

    while (path[i] != '\0' && i < MAXPATH - 1) {
        buf[i] = path[i];
        i++;
    }

    if (i > 0 && buf[i - 1] != '/') {
        buf[i++] = '/';
    }

    while (name[j] != '\0' && i < MAXPATH - 1) {
        buf[i++] = name[j++];
    }

    buf[i] = '\0';
    return i;
}


int du(char *path)
{
    int fd;
    int total = 0;
    int n;
    struct stat st;
    struct dirent de;
    char child[MAXPATH];

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        printf("du: cannot open %s\n", path);
        return 0;
    }

    if (fstat(fd, &st) < 0) {
        printf("du: cannot stat %s\n", path);
        close(fd);
        return 0;
    }
    if (st.type == T_FILE) {
        total = st.size;
        close(fd);

        if (opt_a)
            printf("%d\t%s\n", total, path);

        return total;
    }

    if (st.type != T_DIR) {
        close(fd);
        return 0;
    }

    while ((n = read(fd, &de, sizeof(de))) == sizeof(de)) {
        if (de.inum == 0)
            continue;

        if (strcmp(de.name, ".") == 0 ||
            strcmp(de.name, "..") == 0)
            continue;

        makepath(child, path, de.name);

        total += du(child);
    }

    close(fd);

    if (opt_a || !opt_s)
        printf("%d\t%s\n", total, path);

    return total;
}

int main(int argc, char *argv[])
{
    char *path = ".";
    int i;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-a") == 0) {
            opt_a = 1;
        } else if (strcmp(argv[i], "-s") == 0) {
            opt_s = 1;
        } else {
            path = argv[i];
        }
    }

    if (opt_s) {
        int total = du(path);
        printf("%d\t%s\n", total, path);
    } else {
        du(path);
    }

    exit(0);
}
