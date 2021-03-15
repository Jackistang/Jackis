# Connection

## What are connection?

连接是建立持久数据传输的一种方式，允许在两个设备之间同步交换数据。

## Why are they important?

可以**双向**通信数据，更低的**功耗**，更高地**传输速度**。

![](https://i.loli.net/2021/03/03/VIku8HMr91OgsaE.png)

## How do devices connect?

![](https://i.loli.net/2021/03/03/KgIOjsbxXve3EkT.png)

Peripheral 发送可连接的广播包，Central 收到广播包后，在同一个信道 channel 上发送连接包进入 Initiating 态。之后 Master 和 Slave 的第一次交互即代表了连接建立。

## Connection parameters?

![](https://i.loli.net/2021/03/03/ADP9NL4pXe8RGJk.png)

有一些重要的参数用于保持连接的**同步**和**持久**：

- Connection **Interval**：

定义了 connection event 发生的时刻，connection event 就是 Initialing 状态中 master 向 slave 发送一个数据包，然后 slave 回复一个数据包（可以是个空数据包）的过程。

connection interval 的时间为 7.5 ms - 4.0 sec (1.25ms increments)。

- Slave **Latency**：Slave 并不是每个 connInterval 都需要回复包，可以间隔 latency 来回复，便于 Slave 降低功耗。
- Supervision **Timeout**：自上一次数据交互后超过 timeout 时间没有再进行数据交互，即认为连接丢失了，$SupervisionTimeout > (1 + SlaveLatency) * ConnInterval * 2$ 。
- **Ch**annel **M**ap：

![](https://i.loli.net/2021/03/03/Vde13zLIHCXMFua.png)

定义了在哪些频道上可以进行数据发送，可以避免一些毕竟拥塞的频道。

- **Hop** Sequence：connection event 下一次发生在哪一个频道，会有跳频的现象。

