# Zephyr GATT read & write

## Read

Zephyr 接口：

```C
typedef uint8_t (*bt_gatt_read_func_t)(struct bt_conn *conn, uint8_t err,
				    struct bt_gatt_read_params *params,
				    const void *data, uint16_t length);

struct bt_gatt_read_params {
	/** Read attribute callback. */
	bt_gatt_read_func_t func;
	/** If equals to 1 single.handle and single.offset are used.
	 *  If >1 Read Multiple Characteristic Values is performed and handles
	 *  are used.
	 *  If equals to 0 by_uuid is used for Read Using Characteristic UUID.
	 */
	size_t handle_count;
	union {
		struct {
			/** Attribute handle. */
			uint16_t handle;
			/** Attribute data offset. */
			uint16_t offset;
		} single;
		/** Handles to read in Read Multiple Characteristic Values. */
		uint16_t *handles;
		struct {
			/** First requested handle number. */
			uint16_t start_handle;
			/** Last requested handle number. */
			uint16_t end_handle;
			/** 2 or 16 octet UUID. */
			const struct bt_uuid *uuid;
		} by_uuid;
	};
};

int bt_gatt_read(struct bt_conn *conn, struct bt_gatt_read_params *params);
```

