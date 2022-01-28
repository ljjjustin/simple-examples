# epoll demo

包含连个demo：
* 使用epoll来实现echo服务器。epoll-server.c是服务器端实现，epoll-client.c是客户端实现。
* epoll-read.c是来演示父子进程间的通信，在父进程中，通过epoll来等待并处理子进程发送过来的消息。

# 编译运行

1. 编译echo的服务器端代码及客户端代码。

```bash
$ make epoll-server
$ make epoll-client
$ echo "hello world" | ./epoll-client localhost
...
```
2. 编译运行epoll-read

```bash
$ make epoll-read
$ ./epoll-read
child 30508: 000
child 30509: 000
child 30510: 000
child 30511: 000
child 30511: 001
child 30509: 001
child 30508: 001
child 30510: 001
...
```
