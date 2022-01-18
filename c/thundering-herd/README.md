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

# 参考

https://lwn.net/Articles/542629/
