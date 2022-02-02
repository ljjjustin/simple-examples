#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <sys/wait.h>


#define CHILDREN  4
#define MAXEVENTS 50

static unsigned int NREQS = 0;

void sig_handler(int signo)
{
    printf("%d got %d reqs\n", getpid(), NREQS);
}


void workerproc(int listenfd, int exclusive) {

    int i, rc, len, ready;
    int newfd, pollfd;
    char msg[1024];
    struct sockaddr_in cliaddr;
    struct epoll_event ev, events[MAXEVENTS];

    pollfd = epoll_create1(0);
    if (pollfd == -1) {
        printf("create epoll fd failed\n");
        exit(-1);
    }

    if (exclusive) {
        ev.events = EPOLLIN|EPOLLEXCLUSIVE;
    } else {
        ev.events = EPOLLIN;
    }
    ev.data.fd = listenfd;
    rc = epoll_ctl(pollfd, EPOLL_CTL_ADD, listenfd, &ev);
    if (rc == -1) {
        printf("epoll add listen socket failed\n");
        exit(-1);
    }

    char *ack = "HTTP/1.1 200 OK\r\n";

    for(;;) {
        ready = epoll_wait(pollfd, events, MAXEVENTS, -1);
        if (ready < 0) {
            if (errno != EINTR) {
                perror("epoll_wait error\n");
            }
            continue;
        }

        for (i = 0; i < ready; i++) {
            int efd = events[i].data.fd;
            if (efd == listenfd) {
                /* new connection is comming, call accept */
                socklen_t length = sizeof(cliaddr);

                rc = accept(listenfd, (struct sockaddr *)&cliaddr, &length);
                if (rc == -1) {
                    if (errno == EINTR || errno == EAGAIN) {
                        continue;
                    } else {
                        printf("accept error: %s(errno: %d)\n", strerror(errno), errno);
                        goto finish;
                    }
                }
                newfd = rc;
                /* add to epoll queue */
                int on = 1;
                rc = ioctl(newfd, FIONBIO, &on);
                if (rc < 0) {
                    printf("set socket NON block failed: %s(errno: %d)\n", strerror(errno), errno);
                    continue;
                }
                ev.events = EPOLLIN;
                ev.data.fd = newfd;
                rc = epoll_ctl(pollfd, EPOLL_CTL_ADD, newfd, &ev);
                if (rc == -1) {
                    printf("epoll add new connection socket failed\n");
                    continue;
                }
            } else {
                NREQS++;
                /* new messaging */
                len = read(efd, msg, sizeof(msg));
                if (len > 0) {
                    /* EOF: write back */
                    write(efd, ack, strlen(ack));
                }
                rc = epoll_ctl(pollfd, EPOLL_CTL_DEL, efd, NULL);
                if (rc == -1) {
                    printf("epoll del socket failed\n");
                    continue;
                }
                close(efd);
            }
        }
    }

finish:

    close(pollfd);
}

int main(int argc, char **argv) {

    int i, rc, pid, listenfd;
    int children[CHILDREN];
    struct sockaddr_in servaddr;

    if (argc > 2 || argc < 1) {
        printf("usage: %s <exclusive>\n", argv[0]);
        exit(-1);
    }
    int exclusive=0;
    if (argc == 2 && !strcmp("exclusive", argv[1])) {
        exclusive=1;
    }
    printf("use epoll exclusive: %d\n", exclusive);

    signal(SIGUSR1, sig_handler);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(1);
    }
    int on = 1;
    rc = ioctl(listenfd, FIONBIO, &on);
    if (rc < 0)
    {
        printf("set socket NON block failed: %s(errno: %d)\n", strerror(errno), errno);
        exit(-1);
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(5555);

    rc = bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (rc == -1) {
        printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(1);
    }
    rc = listen(listenfd, 1000);
    if (rc < 0) {
        printf("call listen failed: %s(errno: %d)\n", strerror(errno), errno);
        exit(1);
    }

    for (i = 0; i < CHILDREN; i++) {
        children[i] = 0;
        pid = fork();
        if (pid < 0) {
            printf("fork() failed\n");
            continue;
        } else if (pid) {
            /* parent */
            children[i] = pid;
            continue;
        } else {
            /* child */
            workerproc(listenfd, exclusive);
        }
    }

    /* wait for all children quit */
    for (i = 0; i < CHILDREN; i++) {
        int status=0;
        if (children[i] > 0) {
            waitpid(children[i], &status, 0);
        }
    }
    close(listenfd);
}
