# socketpair

socketpair和管道（pipe）非常类似，主要是用于父子进程之间的网络通信，不同于pipe的单向通信，socketpair支持双向通信。

# 编译运行

```bash
$ make socketpair
$ ./socketpair
send to child: 1
recv from parent: 1
send to parent: 1
recv from child: 1
send to child: 2
recv from parent: 2
send to parent: 2
recv from child: 2
send to child: 3
recv from parent: 3
send to parent: 3
recv from child: 3
...
```
父进程每隔一秒会发送一个数据给子进程，子进程收到数据后，马上返回给父进程同样的数据。
