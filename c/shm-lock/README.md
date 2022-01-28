# spinlock

本段代码演示spinlock在多个子进程之间的应用。首先会创建一批子进程，然后这些子进程按自然数的顺序输出一个计数器的值。

# 编译运行

```bash
$ make shm-lock
$ ./shm-lock
counter is: 1
counter is: 2
counter is: 3
counter is: 4
counter is: 5
counter is: 6
...
counter is: 23
counter is: 24
counter is: 25
```
# 简单说明

首先lock和count是父子进程都会进行读写和修改的，由于父子进程之间的内存有COW（copy on write）的特点，因此，这些变量必须放在跨进程的共享内存空间，所以需要调用mmap分配，而不是定义为全局变量或通过malloc申请。

其次，spinlock的实现用到了cmpxchgl这条指令，保证上锁的原子性。如果指令没有成功，则暂停若干个时钟周期，继续尝试，直到获取锁，而不是把进程挂起。

由于父子进程都需要拿锁，而且每次只能有一个进程拿到锁，所以不管有多少个进程，计数器总是按自然数的顺序递增输出。
