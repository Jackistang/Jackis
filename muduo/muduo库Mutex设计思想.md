# muduo 库 Mutex 设计思想

该 Mutex 提供了互斥锁基础功能和扩展功能。

## 基础功能

mutex 的本职工作就是保护临界区，因此 muduo 库里的 MutexLock 封装了 phread_mutex_t ，使用 RAII 来初始化和销毁 pthread_mutex_t ，同时提供了 `lock()` 和 `unlock()` 函数。

注意上述的上锁和解锁函数并不是给用户使用的，而是给同文件里的 MutexLockGuard 使用的，MutexLockGuard 对象构造时上锁，析构时解锁，令临界区的范围与 MutexLockGuard 对象的生命周期一致，这也是 RAII 的一种经典用法。使用方式如下：

```C++
MutexLock mutex;
void doit()
{
    MutexLockGuard lock(mutex);

    // This is Critical section
}
```



## 扩展功能

muduo 库里的 MutexLock 多添加了一个成员 `pid_t holder_`，是为了保存持有该互斥锁的线程信息。因此需要在上锁后调用 `assignHolder()` 函数保存当前线程信息，在解锁前调用 `unassignHolder()` 函数清除当前线程信息。

同时提供了接口 `assertLocked()` 来断言该互斥锁已被当前线程所持有，一定程度上避免了死锁。

但是作者在使用 pthread_condition_t 封装 Conditon 时遇到了问题，调用 `pthread_cond_wait()` 函数前，互斥锁已经被上锁，当条件变量不满足条件时，pthread 库会将互斥锁解锁，并使当前线程休眠。 

```C++
MutexLock& mutex_;
pthread_cond_t pcond_;

pthread_cond_wait(&pcond_, mutex_.getPthreadMutex());
```

但这样有一个问题，pthread 库解锁互斥锁时并没有将 MutexLock 里的线程信息清空，这与设计不相符，而且会导致断言。因此 MutexLock 类里添加了 UnassignGuard 内类。

```C++
class MutexLock : noncopyable
{
private:
  friend class Condition;
  class UnassignGuard : noncopyable
  {
   public:
    explicit UnassignGuard(MutexLock& owner)
      : owner_(owner)
    {
      owner_.unassignHolder();
    }
    ~UnassignGuard()
    {
      owner_.assignHolder();
    }
   private:
    MutexLock& owner_;
  };  
};
```

UnassignGuard 类使用 RAII 在构造时清空线程信息，在析构时设置线程信息。

因此 Condition 类里的 `wait()` 函数就可以安全的实现了，在调用 `pthread_cond_wait()` 前构造一个 UnassignGuard 对象，清空互斥锁里的线程信息，然后 pthread 库释放互斥锁，线程休眠；当条件满足时，pthread 重新持有互斥锁，并且 `pthread_cond_wait()` 函数返回，之后 `wait()` 函数返回时，UnassignGuard 对象析构，重新设置线程信息。

```C++
void wait()
{
    MutexLock::UnassignGuard ug(mutex_);
    pthread_cond_wait(&pcond_, mutex_.getPthreadMutex());
}
```



