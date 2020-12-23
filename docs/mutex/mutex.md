# mutex 使用的相关技术

## static_assert

资料：[C++11 静态断言(static_assert)](https://www.cnblogs.com/lvdongjie/p/4489835.html) 。

> C++0x中引入了**static_assert** 这个关键字，用来做编译期间的断言，因此叫做静态断言。
>
> 其语法很简单：**static_assert**（常量表达式，提示字符串）。
>
> 如果第一个参数常量表达式的值为真（true或者非零值），那么 **static_assert** 不做任何事情，就像它不存在一样，否则会产生一条编译错误，错误位置就是该 **static_assert** 语句所在行，错误提示就是第二个参数提示字符串。

## \_\_typeof\_\_ GNU C 扩展

[GNU 官方资料](https://gcc.gnu.org/onlinedocs/gcc/Typeof.html)，[详解 GNU C 标准中的 typeof 关键字](https://blog.csdn.net/zhanshen2015/article/details/51495273) 。

GNU C 扩展标准中 typedef 用于推断表达式或类型的类型名。与 C++ 11 里的 decltype 类似。

## mutex_lock 使用说明

mutex_lock 简单封装了 pthread_mutex_t ，一般不直接使用 mutex_lock 的成员函数，而是与 mutex_lock_guard 搭配着一起使用。使用 RAII 的手法，令临界区的范围与 mutex_lock_guard 对象的生命周期一致。

```C++
mutex_lock mutex;
void doit()
{
    mutex_lock_guard lock(mutex);

    // This is Critical section
}
```

但是可能存在这样一种误用的情况：

```C++
mutex_lock mutex;
void doit()
{
    mutex_lock_guard(mutex);
    //mutex_lock_guard lock(mutex);

    // This is not Critical section
}
```

这也会编译通过，但创建的临时对象很快就销毁了，临界区并没有被锁住。因此定义了 `mutex_lock_guard(x)` 这个宏，出现上述情况在编译期间就会报错。

## google-glog CHECK()

TODO