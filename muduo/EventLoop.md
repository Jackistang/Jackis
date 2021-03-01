## EventLoop 类

一个线程只能由一个 Loop （one loop per thread），因此需要一种机制来保证该情况。

EventLoop 里用变量 `__thread EventLoop* t_loopInThisThread` 存储当前线程里 EventLoop 的指针，并在构造函数保证当前线程最多仅有一个 EventLoop 。

muduo 库里的接口有些可以跨线程调用，有些只能在某个特定的线程调用，比如 EventLoop 只能在 IO 线程里调用，EventLoop 提供了 `assertInLoopThread()` 和 `isInLoopThread()` 在运行时检查这些前置条件。

## __thread 关键字

\_\_thread 是 GCC 内置的线程局部存储措施，\_\_thread 修饰的变量是表示每个线程都有一份独立的实体，各个线程的变量值互不干扰。



## Poller 类

### 储备知识

#### POSIX - 函数 poll

```C
#include <poll.h>
int poll(struct pollfd fdarray[], nfds_t nfds, int timeout);
					// 返回值：准备就绪的描述符数目；若超时，返回0；若出错，返回-1
                        
struct pollfd {
    int fd;			/* file descriptor to check, or < 0 to ignore */
    short event;	/* events of interest on fd */
    short revents;	/* events that occurred on fd */
};
```

poll 函数传入一个 pollfd 结构体数组，并指定其数组大小 `nfds`。其中 `fd` 成员为文件描述符，`event` 成员为我们感兴趣的事件，`revent` 成员说明每个描述符发生了哪些事件，由内核设置。

常用事件如下：

| 标志名   | 说明                       |
| -------- | -------------------------- |
| POLLIN   | 可以不阻塞地读数据         |
| POLLPRI  | 可以不阻塞地读优先级数据   |
| POLLOUT  | 可以不阻塞地写数据         |
| POLLERR  | 已出错                     |
| POLLHUP  | 已挂断                     |
| POLLNVAL | 描述符没有引用一个打开文件 |

poll 函数的最后一个参数 `timeout` 表明我们愿意等待多长时间。

- timeout == -1 : 永久等待。
- timeout == 0  : 不等待。
- timeout > 0    : 等待 timeout 毫秒。

#### 边缘触发(Edge Trigger)和水平触发(Level Trigger)

水平触发(LT)，也被称为条件触发：只要条件满足，就会触发一个事件（只要有数据没有被读取，内核就一直通知你）。

边缘触发(ET)：每当状态变化时，触发一个事件。

### 使用

Poller 对象是 EventLoop 的间接成员，只供其 owner EventLoop 在 IO 线程里调用。

## Channel 类

每个 Channel 对象自始至终只属于一个 EventLoop ，每个 Channel 对象自始至终只负责一个文件描述符（fd）的 IO 事件分发，但它并不拥有这个 fd，也不会在析构时关闭这个 fd 。

