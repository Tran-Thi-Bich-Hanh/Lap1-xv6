#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    int mask;
    if (argc < 3) {
        fprintf(2, "Usage: trace mask command [args...]\n");
        exit(1);
    }

    mask = atoi(argv[1]);
    trace(mask);
    exec(argv[2], &argv[2]);

    fprintf(2, "trace: exec failed\n");
    exit(1);
}