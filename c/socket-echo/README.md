# 编译

```bash
$ make echo-client
$ make echo-server
```

# 执行

启动服务器端
```bash
$ ./echo-server
```

启动客户端

```bash
$ ./echo-client localhost
```

# 输出

客户端

```
=====send message to server:
hello echo server
=====recv=====
hello echo server
```

服务器端

```
=====recv====
hello echo server
```
