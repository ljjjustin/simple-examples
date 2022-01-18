# 惊群问题

验证多进程同时监听同一个端口的情况下，内核是否存在惊群的问题。

备注：测试是在centos7上执行的，内核版本是：3.10.0-1160.45.1.el7.x86_64

# 编译

```bash
$ make accept-client
$ make accept-server
```

# 执行

1. 启动服务器端
```bash
$ ./accept-server
```
2. 执行下面的命令，确认server在同一个socket上监听
```bash
# for pid in $(pgrep accept-server); do ls -l /proc/$pid/fd | grep socket; done
lrwx------ 1 root root 64 1月  18 10:00 3 -> socket:[48508933]
lrwx------ 1 root root 64 1月  18 10:00 3 -> socket:[48508933]
lrwx------ 1 root root 64 1月  18 10:00 3 -> socket:[48508933]
lrwx------ 1 root root 64 1月  18 10:00 3 -> socket:[48508933]
lrwx------ 1 root root 64 1月  18 10:00 3 -> socket:[48508933]

# 上面的结果显示所有的accept-server在同一个socket上监听。
```

3. 启动客户端

启动accept-client，给accept-server发送100条消息。

```bash
$ for for i in $(seq 1 100); do c=$(printf "%03d" $i); ./accept-client localhost msg$c; done

```

服务器端输出如下：

```text
26270 received: msg001
26271 received: msg002
26272 received: msg003
26273 received: msg004
26270 received: msg005
26271 received: msg006
26272 received: msg007
26273 received: msg008
26270 received: msg009
26271 received: msg010
26272 received: msg011
26273 received: msg012
26270 received: msg013
26271 received: msg014
26272 received: msg015
26273 received: msg016
26270 received: msg017
26271 received: msg018
26272 received: msg019
26273 received: msg020
26270 received: msg021
26271 received: msg022
26272 received: msg023
26273 received: msg024
...
```

# 结论

服务器端会创建4个子进程，然后在子进程中执行accept，等待新的连接，然后处理新的连接。
从上面的输出可以看到，各个子进程处理的连接数是完全一致的，没有负载不一致的情况。
**由此可见，现在的Linux内核已经修复了惊群的问题。**

# 对比

惊群的问题在早期的Linux系统上确实存在，后来内核引入了SO_REUSEPORT特性，来解决这个问题，作为对比，需要修改accept-server的代码。
使用SO_REUSEPORT之后的代码是`reuseport-server.c`。我们编译一下，然后启动服务器端。

```bash
$ make reuseport-server
$ ./reuseport-server
```

然后查看reuseport-server监听的socket文件：

```bash
$ for pid in $(pgrep -f reuseport-server); do ls -l /proc/$pid/fd | grep socket; done
lrwx------ 1 root root 64 1月  18 10:42 3 -> socket:[48538952]
lrwx------ 1 root root 64 1月  18 10:42 3 -> socket:[48538954]
lrwx------ 1 root root 64 1月  18 10:42 3 -> socket:[48538956]
lrwx------ 1 root root 64 1月  18 10:42 3 -> socket:[48539735]
```

从上面的结果可以看出，现在四个reuseport-server是在不同的socket文件上监听。

最后，同样用accept-client发起100次请求，输出如下：

```text
31042 received: msg001
31043 received: msg002
31042 received: msg003
31042 received: msg004
31044 received: msg005
31041 received: msg006
31044 received: msg007
31041 received: msg008
31042 received: msg009
31043 received: msg010
31042 received: msg011
31044 received: msg012
31042 received: msg013
31043 received: msg014
31041 received: msg015
31044 received: msg016
31042 received: msg017
31042 received: msg018
31044 received: msg019
31042 received: msg020
31041 received: msg021
31043 received: msg022
31042 received: msg023
31042 received: msg024
31043 received: msg025
31043 received: msg026
31044 received: msg027
31041 received: msg028
31041 received: msg029
31044 received: msg030
...
```

统计程序的输出结果，四个进程的输出数量是**相同**的。

从这里可以看出，在使用SO_REUSEPORT的情况下，即使四个进程不在同一个socket上监听相同的端口，其负载仍然是均衡的，这种均衡是内核实现的。

因为同一个端口对应多个不同的监听socket，不同的socket可能有属于不同的进程，进程的数量常常不是固定的，如何保证负载均衡的效果，数据包来了，如何快速的找到与之对应的socket等。这些都是Linux内核在不断优化中的问题。

# 参考

https://en.wikipedia.org/wiki/Thundering_herd_problem
https://lwn.net/Articles/542629/
https://segmentfault.com/a/1190000020524323
https://segmentfault.com/a/1190000020536287
https://blog.csdn.net/dog250/article/details/51510823
