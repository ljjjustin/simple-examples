#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


int main(int argc, char **argv)
{
    int err, len;
    int listenfd, connfd;
    char buff[4096];
    struct sockaddr_in servaddr, cliaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(5555);

    err = bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (err == -1) {
        printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(1);
    }

    if (listen(listenfd, 100) < 0) {
        printf("call listen failure!\n");
        exit(1);
    }

    for(;;) {
        socklen_t length = sizeof(cliaddr);

        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &length);
        if (connfd == -1) {
            printf("accept error: %s(errno: %d)\n", strerror(errno), errno);
            break;
        }

        len = recv(connfd, buff, sizeof(buff), 0);
        buff[len] = '\0';
        printf("=====recv====\n%s\n", buff);

        send(connfd, buff, len, 0);

        close(connfd);
    }

    close(listenfd);
    exit(0);
}
