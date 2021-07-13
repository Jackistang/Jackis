# Zephyr BLE GATT discover

核心接口：

```C
typedef uint8_t (*bt_gatt_discover_func_t)(struct bt_conn *conn,
					const struct bt_gatt_attr *attr,
					struct bt_gatt_discover_params *params);

/** @brief GATT Discover Attributes parameters */
struct bt_gatt_discover_params {
	/** Discover UUID type */
	const struct bt_uuid *uuid;
	/** Discover attribute callback */
	bt_gatt_discover_func_t func;
	union {
		struct {
			/** Include service attribute declaration handle */
			uint16_t attr_handle;
			/** Included service start handle */
			uint16_t start_handle;
			/** Included service end handle */
			uint16_t end_handle;
		} _included;
		/** Discover start handle */
		uint16_t start_handle;
	};
	/** Discover end handle */
	uint16_t end_handle;
	/** Discover type */
	uint8_t type;
#if defined(CONFIG_BT_GATT_AUTO_DISCOVER_CCC)
	/** Only for stack-internal use, used for automatic discovery. */
	struct bt_gatt_subscribe_params *sub_params;
#endif /* defined(CONFIG_BT_GATT_AUTO_DISCOVER_CCC) */
};

int bt_gatt_discover(struct bt_conn *conn, struct bt_gatt_discover_params *params);
```

Zephyr GATT Discover 使用一个统一的接口 `bt_gatt_discover()`，其参数 `params` 里的 `type` 字段不同，函数的行为也不一样，具体可分为四种发现过程：

- Primary Service Discovery
- Include Service Discovery
- Characteristic Discovery
- Descriptors Discovery

发现过程中的回调函数里的 `attr` 值只有 `uuid`，`handle` 和 `user_data` 是有效值，

- `handle` ：Attribute handle 。
- `uuid`：Attribute type uuid 。
- `user_data`：Attribute value，`user_data` 需要根据不同的 `uuid` 强制转换成其他结构体类型数据。

| uuid                                               | user_data             |
| :------------------------------------------------- | --------------------- |
| `BT_UUID_GATT_PRIMARY` or `BT_UUID_GATT_SECONDARY` | `bt_gatt_service_val` |
| `BT_UUID_GATT_INCLUDE`                             | `bt_gatt_include`     |
| `BT_UUID_GATT_CHRC`                                | `bt_gatt_chrc`        |

相关结构体如下：

```C
struct bt_gatt_service_val {
	/** Service UUID. */
	const struct bt_uuid *uuid;
	/** Service end handle. */
	uint16_t end_handle;
};
struct bt_gatt_include {
	/** Service UUID. */
	const struct bt_uuid *uuid;
	/** Service start handle. */
	uint16_t start_handle;
	/** Service end handle. */
	uint16_t end_handle;
};
struct bt_gatt_chrc {
	/** Characteristic UUID. */
	const struct bt_uuid *uuid;
	/** Characteristic Value handle. */
	uint16_t value_handle;
	/** Characteristic properties. */
	uint8_t	properties;
};
```

发现过程中的回调函数返回 `BT_GATT_ITER_CONTINUE` 表示继续发现过程，返回 `BT_GATT_ITER_STOP` 表示停止发现过程。

下面分别介绍这四种发现过程。

## Primary Service Discovery

参数 `params` 内部：

- `type`：`BT_GATT_DISCOVER_PRIMARY` 。
- `uuid`：要查找的 Primary Service 的 uuid ，**可以为 NULL** 。
- `func`：回调函数。
- `start_handle`：在 union 内部，表示发现过程中的起始 handle 。
- `end_handle`：表示发现过程中的结束 handle 。

该发现过程**流程**为：

- 若 uuid 不为 NULL，则在 $[start\_handle, end\_handle]$ 范围内，发现指定 uuid 的 Service 时会调用回调函数。
- 若 uuid 为 NULL，则在 $[start\_handle, end\_handle]$ 范围内，每发现一个 Service 时都会调用回调函数。

在回调函数中，

```C
bt_gatt_discover_func_t(struct bt_conn *conn,
					const struct bt_gatt_attr *attr,
					struct bt_gatt_discover_params *params);
```

`param` 参数就是之前传递给 `bt_gatt_discover()` 参数的指针，

`attr` 是当前找到的属性的指针，是一个临时值。

- `handle` 是当前 Service 的 handle，
- `uuid` 是属性类型 -- 0x2800(Primary Service)，
- `user_data` 强制转换为 `bt_gatt_service_val` 结构体指针。

## Include Service Discovery

TODO：很少使用，等接触过再补充。

## Characteristic Discovery

参数 `params` 内部：

- `type`：`BT_GATT_DISCOVER_CHARACTERISTIC` 。
- `uuid`：要查找的 Characteristic 的 uuid ，可以为 NULL 。
- `func`：回调函数。
- `start_handle`：在 union 内部，表示发现过程中的起始 handle 。
- `end_handle`：表示发现过程中的结束 handle 。

该发现过程**流程**为：在 $[start\_handle, end\_handle]$ 范围内，每发现一个 Characteristic 或者发现指定 uuid 的 Characteristic 时会调用回调函数。

在回调函数中，

`attr` 是当前找到的属性的指针，是一个临时值。

- `handle` 是当前 Characteristic 的 handle，
- `uuid` 是属性类型 -- 0x2803(Characteristic)，
- `user_data` 强制转换为 `bt_gatt_chrc` 结构体指针。

## Descriptors Discovery

参数 `params` 内部：

- `type`：`BT_GATT_DISCOVER_DESCRIPTOR` 。
- `uuid`：要查找的 Descriptors 的 uuid （好像没有作用）。
- `func`：回调函数。
- `start_handle`：在 union 内部，表示发现过程中的起始 handle 。
- `end_handle`：表示发现过程中的结束 handle 

该发现过程**流程**为：在 $[start\_handle, end\_handle]$ 范围内，每发现一个 Descriptor 就会调用回调函数。

在回调函数中，

- `handle` 是当前 Descriptor 的 handle，
- `uuid` 是属性类型，用于识别不同的 Descriptor ，
- `user_data` 为 NULL 。

## Attribute Discovery

参数 `params` 内部：

- `type`：`BT_GATT_DISCOVER_ATTRIBUTE` 。
- `uuid`：要查找的 Descriptors 的 uuid （好像没有作用）。
- `func`：回调函数。
- `start_handle`：在 union 内部，表示发现过程中的起始 handle 。
- `end_handle`：表示发现过程中的结束 handle 

该发现过程**流程**为：在 $[start\_handle, end\_handle]$ 范围内，每发现一个 Attribute 就会调用回调函数。



## 利用 Zephyr 接口获取 Server 所有服务和特征

首先发现全部 Service，形成 JSON 数组：

```json
[
    {
        "handle": "0x0001",
        "UUID": "0x1801",	// 以后再考虑统一为 128bit UUID
        "end_handle": "0x0008"
    },
    {
        "handle": "0x0009",
        "UUID": "0x1800",
        "end_handle": "0x000d"
    },
    {
        "handle": "0x000e",
        "UUID": "0x180a",
        "end_handle": "0x0012"
    }
]
```

然后再发现所有 Service 里 $[handle, end\_handle]$ 之间的所有 Characteristic 。

```json
[
    {
        "handle": "0x0001",
        "UUID": "0x1801",	// 以后再考虑统一为 128bit UUID
        "end_handle": "0x0008",
        "characteristics": [
            {
                "handle": "0x0002",	// Characteristic Value handle
                "UUID": "0x2a05",
                "properties": [
                    "broadcast", "read", "write_without_response", "write", "notify", "indicate", "auth"
                ]
            },
            {
            	"handle": "0x0005",
            	"UUID": "0x2b2a",
            	"properties": [
                    "read"
                ]
            }
        ]
    },
	......
]
```

然后再发现每一个 Characteristic 的描述符。

```json
[
    {
        "handle": "0x0001",
        "UUID": "0x1801",	// 以后再考虑统一为 128bit UUID
        "end_handle": "0x0008",
        "characteristics": [
            {
                "handle": "0x0002",	// Characteristic Value handle
                "UUID": "0x2a05",
                "properties": [
                    "broadcast", "read", "write_without_response", "write", "notify", "indicate", "auth"
                ],
                "descriptors": [{
                    "handle": "",
                    "UUID": ""
                }
                ]
            },
            ......
        ]
    },
	......
]
```



一个全局变量 cJSON * cur_json，

发现 Service , Characteristic, Descriptor 时，直接向 `cur_json` 里添加对象。