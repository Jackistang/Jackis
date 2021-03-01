# Timestamp

Timestamp 只有一个成员变量 `microSecondsSinceEpoch_`，用于表示从 Epoch 开始的微妙数，Epoch 指的是一个特定的时间：1970-01-01 00:00:00 UTC。



```c++
// 给 timestamp 增加 seconds 秒
Timestamp addTime(Timestamp timestamp, double seconds);
```





Reference: [https://blog.csdn.net/KangRoger/article/details/47089827](https://blog.csdn.net/KangRoger/article/details/47089827)