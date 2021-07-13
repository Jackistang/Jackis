# Zephyr BLE UUID

在 Zephyr BLE 协议栈里，UUID 有 16bit, 32bit 和 128bit 三种类型，结构体为：

```C
/** @brief This is a 'tentative' type and should be used as a pointer only */
struct bt_uuid {
	uint8_t type;
};

struct bt_uuid_16 {
	/** UUID generic type. */
	struct bt_uuid uuid;
	/** UUID value, 16-bit in host endianness. */
	uint16_t val;
};

struct bt_uuid_32 {
	/** UUID generic type. */
	struct bt_uuid uuid;
	/** UUID value, 32-bit in host endianness. */
	uint32_t val;
};

struct bt_uuid_128 {
	/** UUID generic type. */
	struct bt_uuid uuid;
	/** UUID value, 128-bit in little-endian format. */
	uint8_t val[BT_UUID_SIZE_128];
};
```

`bt_uuid_16`, `bt_uuid_32` 和 `bt_uuid_128` 分别表示 16bit, 32bit 和 128bit 的 UUID，但是在实际使用的过程中，往往用的是其结构体内部的 `uuid` 这个结构体的指针。

类似与面向对象的多态概念，根据其 `type` 字段的值不同，我们可以将其强制转换成不同结构体类型的指针。

例如

```C
void print_uuid(struct bt_uuid* uuid)
{
    switch (uuid->type) {
        case BT_UUID_TYPE_16:  print_uuid_16((struct bt_uuid_16 *)uuid);	break;
        case BT_UUID_TYPE_32:  print_uuid_32((struct bt_uuid_32 *)uuid);	break;
        case BT_UUID_TYPE_128: print_uuid_128((struct bt_uuid_128 *)uuid);	break;
    }
}
```

协议栈还提供了相应的操作接口：

```C
int bt_uuid_cmp(const struct bt_uuid *u1, const struct bt_uuid *u2);
bool bt_uuid_create(struct bt_uuid *uuid, const uint8_t *data, uint8_t data_len);
void bt_uuid_to_str(const struct bt_uuid *uuid, char *str, size_t len);
```

