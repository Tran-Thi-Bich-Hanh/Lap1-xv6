#include "kernel/types.h"
#include "user/user.h"

void
sieve(int p[2])
{
    int prime;
    int num;

    close(p[1]);

    if (read(p[0], &prime, sizeof(prime)) == 0)
    {
        close(p[0]);
        return;
    }

    printf("prime %d\n", prime);

    int q[2];
    pipe(q);

    if (fork() == 0)
    {
        close(p[0]);
        sieve(q);
    }
    else
    {
        close(q[0]);

        while (read(p[0], &num, sizeof(num)) == sizeof(num))
        {
            if (num % prime != 0)
            {
                write(q[1], &num, sizeof(num));
            }
        }

        close(p[0]);
        close(q[1]);

        wait(0);
    }
}

int
main(int argc, char *argv[])
{
    int p[2];

    pipe(p);

    if (fork() == 0)
    {
        sieve(p);
    }
    else
    {
        close(p[0]);

        for (int i = 2; i <= 280; i++)
        {
            write(p[1], &i, sizeof(i));
        }

        close(p[1]);
        wait(0);
    }

    exit(0);
}