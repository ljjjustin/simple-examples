#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

int main(int argc, char **argv) {
    int fdpair[2];
    int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, fdpair);

    if (ret < 0) {
        printf("create socketpair failed\n");
        exit(-1);
    }

    ret = fork();
    if (ret > 0) {
        int var = 0;
        /* parent */
        close(fdpair[1]);
        for (;;) {
            sleep(1);
            ++var;
            printf("send to child: %d\n", var);
            write(fdpair[0], &var, sizeof(var));
            read(fdpair[0], &var, sizeof(var));
            printf("recv from child: %d\n", var);
        }
    } else if (ret == 0) {
        /* child */
        int var = 0;
        close(fdpair[0]);
        for (;;) {
            read(fdpair[1], &var, sizeof(var));
            printf("recv from parent: %d\n", var);
            printf("send to parent: %d\n", var);
            write(fdpair[1], &var, sizeof(var));
        }
    } else {
        printf("fork() failed\n");
        exit(-1);
    }
}
