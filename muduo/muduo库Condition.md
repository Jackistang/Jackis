# muduo 库 Condition

条件变量主要用于 “生产者消费者问题” 。

muduo 库里的 Condition 的语义与 pthread 库里的条件变量语义一致。需搭配着互斥锁一起使用。

Condition 只能显示构造，且需要传入一个 MutexLock 对象，Contition 里保存该互斥锁的一个引用。

其有四个成员函数：

- `wait()`：在调用 wait 函数前 MutexLock 需要先上锁。若此时条件不满足，则将 MutexLock 解锁，然后线程阻塞在 wait 函数里。当条件满足时，重新给 MutexLock 上锁，并且从 wait 函数返回。
- `waitForSeconds(double seconds)`：在调用 waitForSeconds 函数前 MutexLock 需要先上锁，若条件不满足，则将 MutexLock 解锁，然后线程阻塞在 waitForSeconds 函数里。这里唤醒线程有两种方式：一种是条件满足，另一种是等待时间已到。若超时则 waitForSeconds 函数返回 true，否则返回 false。该函数返回前会重新给  MutexLock 上锁。
- `notify()`：通知一个及以上阻塞在 wait 函数处的线程条件已满足。
- `notifyAll()`：通知所有阻塞在 wait 函数处的线程条件已满足。