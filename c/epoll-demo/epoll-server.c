#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/epoll.h>
#include <sys/ioctl.h>


#define MAXEVENTS 10
#define MAXCHILDREN 4


int main(int argc, char **argv) {

    int i, c, on, rc;
    int newfd, pollfd, listenfd;
    struct sockaddr_in servaddr, cliaddr;
    struct epoll_event ev, events[MAXEVENTS];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(1);
    }
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
    rc = listen(listenfd, 100);
    if (rc < 0) {
        printf("call listen failed: %s(errno: %d)\n", strerror(errno), errno);
        exit(1);
    }

    pollfd = epoll_create1(0);
    if (pollfd == -1) {
        printf("create epoll fd failed\n");
        exit(-1);
    }

    ev.events = EPOLLIN;
    ev.data.fd = listenfd;

    rc = epoll_ctl(pollfd, EPOLL_CTL_ADD, listenfd, &ev);
    if (rc == -1) {
        printf("epoll add listen socket failed\n");
        goto out;
    }

    int len;
    char msg[100];
    for(;;) {
        rc = epoll_wait(pollfd, events, MAXEVENTS, -1);
        if (rc < 0) {
            printf("epoll wait error: %d\n", errno);
            continue;
        }

        for (i = 0; i < rc; i++) {
            if (events[i].data.fd == listenfd) {
                /* new connection is comming, call accept */
                socklen_t length = sizeof(cliaddr);

                rc = accept(listenfd, (struct sockaddr *)&cliaddr, &length);
                if (rc == -1) {
                    printf("accept error: %s(errno: %d)\n", strerror(errno), errno);
                    continue;
                }
                newfd = rc;
                /* add to epoll queue */
                rc = ioctl(newfd, FIONBIO, &on);
                if (rc < 0)
                {
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
                /* new messaging */
                len = read(events[i].data.fd, msg, sizeof(msg));
                if (len) {
                    msg[len] = '\0';
                    printf("%s\n", msg);
                    write(events[i].data.fd, msg, len);
                }
                /* remove from epoll queue */
                rc = epoll_ctl(pollfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                if (rc == -1) {
                    printf("epoll add new connection socket failed\n");
                    continue;
                }
                close(events[i].data.fd);
            }
        }
    }

out:
    close(listenfd);
    close(pollfd);
}
