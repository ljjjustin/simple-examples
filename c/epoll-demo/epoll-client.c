#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
    int err, len, servfd;
    struct sockaddr_in servaddr;
    char sendline[4096], recvline[4096];

    if (argc != 2) {
        printf("usage: %s <server ip>\n", argv[0]);
        exit(1);
    }

    servfd = socket(AF_INET, SOCK_STREAM, 0);
    if (servfd == -1) {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5555);
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) < 0) {
        printf("inet_pton error for %s\n", argv[1]);
        exit(1);
    }

    if (connect(servfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        printf("connect error: %s(errno: %d)\n", strerror(errno), errno);
        exit(1);
    }

    printf("=====send message to server: \n");
    fgets(sendline, sizeof(sendline)-1, stdin);
    if (send(servfd, sendline, strlen(sendline), 0) < 0) {
        printf("connect error: %s(errno: %d)\n", strerror(errno), errno);
        exit(1);
    }
    len = recv(servfd, recvline, sizeof(recvline), 0);
    if (len >= 0) {
        recvline[len] = '\0';
        printf("=====recv=====\n%s\n", recvline);
    }

    close(servfd);
}
