#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>


#define MAXEVENTS 10
#define MAXCHILDREN 4


int listensocks[MAXCHILDREN];


void workerproc(int fd) {

    int i, c = 0;
    int p = getpid();
    char msg[100];

    for (i = 0; i  < MAXCHILDREN; i++) {
        if (listensocks[i] && listensocks[i] != fd) {
            close(listensocks[i]);
        }
    }

    for (;;) {
        sleep(1);
        sprintf(msg, "child %05d: %03d", p, c++);
        write(fd, msg, strlen(msg));
    }

    close(fd);
}

int main(int argc, char **argv) {

    int i, c, on, ret, pid;
    int pollfd, fdpair[2];
    struct epoll_event ev, events[MAXEVENTS];

    pollfd = epoll_create1(0);
    if (pollfd == -1) {
        printf("create epoll fd failed\n");
        exit(-1);
    }

    for (on = 1, i = 0; i < MAXCHILDREN; i++) {
        listensocks[i] = 0;
        ret = socketpair(AF_UNIX, SOCK_STREAM, 0, fdpair);
        if (ret < 0) {
            printf("create socketpair failed\n");
            continue;
        }
        ret = ioctl(fdpair[0], FIONBIO, &on);
        if (ret < 0)
        {
            printf("set NON block failed\n");
            close(fdpair[0]);
            close(fdpair[1]);
            continue;
        }
        ret = ioctl(fdpair[1], FIONBIO, &on);
        if (ret < 0)
        {
            printf("set NON block failed\n");
            close(fdpair[0]);
            close(fdpair[1]);
            continue;
        }


        pid = fork();
        if (pid < 0) {
            printf("fork() failed\n");
            continue;
        } else if (pid) {
            /* parent */
            close(fdpair[1]);
            listensocks[i] = fdpair[0];
        } else {
            /* child */
            close(pollfd);
            close(fdpair[0]);
            workerproc(fdpair[1]);
        }
    }

    for (i = 0, c = 0; i  < MAXCHILDREN; i++) {
        if (listensocks[i]) {
            ev.events = EPOLLIN;
            ev.data.fd = listensocks[i];

            ret = epoll_ctl(pollfd, EPOLL_CTL_ADD, listensocks[i], &ev);
            if (ret == -1) {
                printf("epoll add listen socket failed\n");
                continue;
            }
            c++;
        }
    }

    if (c) {
        int len;
        char msg[100];
        for(;;) {
            ret = epoll_wait(pollfd, events, MAXEVENTS, -1);
            for (i = 0; i < ret; i++) {
                len = read(events[i].data.fd, msg, sizeof(msg));
                msg[len] = '\0';
                printf("%s\n", msg);
            }
        }
    }

    for (i = 0; i  < MAXCHILDREN; i++) {
        if (listensocks[i]) {
            close(listensocks[i]);
        }
    }
    close(pollfd);
}
