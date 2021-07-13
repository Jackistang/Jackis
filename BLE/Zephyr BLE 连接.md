# Zephyr BLE 连接

**辅助函数：**

- `bt_conn_foreach()` 可以遍历所有建立的连接。
- `bt_conn_lookup_addr_le()` 利用对端蓝牙地址得到连接句柄 `struct bt_conn *`。
- `bt_conn_get_dst()` 根据连接句柄得到对端蓝牙地址。



**句柄声明周期：**BLE 连接句柄使用了引用计数，当引用计数为 0 时，该结构体声明周期结束，内存空间会被释放。

- `bt_conn_ref()` 连接句柄引用计数加 1 。
- `bt_conn_unref()` 连接句柄引用计数减 1 。



**连接**：

- `bt_conn_le_create()` 用于建立连接。
- `bt_conn_disconnect()` 用于断开连接。