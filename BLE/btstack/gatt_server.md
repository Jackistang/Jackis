# BTstack - GATT Server

## .gatt 文件

首先我们需要了解一下 BTstack 特有的 GATT 表示文件 `.gatt`，它是一种类似 csv 格式的表示法，如下所示：

```
// import service_name
#import <service_name.gatt>

PRIMARY_SERVICE, {SERVICE_UUID}
CHARACTERISTIC, {ATTRIBUTE_TYPE_UUID}, {PROPERTIES}, {VALUE}
CHARACTERISTIC, {ATTRIBUTE_TYPE_UUID}, {PROPERTIES}, {VALUE}
...
PRIMARY_SERVICE, {SERVICE_UUID}
CHARACTERISTIC, {ATTRIBUTE_TYPE_UUID}, {PROPERTIES}, {VALUE}
...
```

每一行代表一个 ATT 里规定的 Attribute，`PRIMARY_SERVICE` 表明定义一个 primary service，并且其后面的 `SERVICE_UUID` 代表了此 service 的 uuid，可以是 16bit (1800) ，也可以是 128bit (00001234-0000-1000-8000-00805F9B34FB)。

`CHARACTERISTIC` 表明定义一个 characteristic，`ATTRIBUTE_TYPE_UUID` 就是该 characteristic 的 uuid，`VALUE` 可以是一个字符串（“this is a string”），也可以是一个 16 进制的字节序（ 01 02 03）。`PROPERTIES` 表明此 characteristic 的属性，多个属性可以用 `|` 操作连接，常用的有：

| Property               | Meaning                                                     |
| :--------------------- | :---------------------------------------------------------- |
| READ                   | Characteristic can be read                                  |
| WRITE                  | Characteristic can be written using Write Request           |
| WRITE_WITHOUT_RESPONSE | Characteristic can be written using Write Command           |
| NOTIFY                 | Characteristic allows notifications by server               |
| INDICATE               | Characteristic allows indication by server                  |
| DYNAMIC                | Read or writes to Characteristic are handled by application |

上述的属性只有 `DYNAMIC` 需要着重理解一下，该属性并不是 Spec 里定义的，而是 BTstack 协议栈自定义的属性，添加了 `DYNAMIC` 属性后，当对该 characteristic value 进行读写操作时，协议栈会通过 `att_servet_*()` 函数注册的回调函数通知应用程序处理，适用于 value 值需要动态变化的场景，例如 Battery Service 里的电池电量会不断变化。

当然，为了方便 `.gatt` 文件的服用，BTstack 提供了 `#import <service_name.gatt>` 语法，其类似于 C 语言的 `#include`，将 `service_name.gatt` 文件里的内容复制到 import 的地方。

如何快速生成标准 GATT Service 的 `.gatt` 文件呢？BTstack 也提供了相应的工具 `tool/convert_gatt_service.py` ，在命令行直接运行该工具，会列出一系列的标准 Service，如下所示：

```shell
jackis@jackis-VirtualBox:~/btstack$ tool/convert_gatt_service.py 
List of services cached from https://www.bluetooth.com/specifications/gatt/services/ on 2020-07-22

Specification Type                                     | Specification Name            | UUID
-------------------------------------------------------+-------------------------------+-----------
org.bluetooth.service.alert_notification               | Alert Notification Service    | 0x1811
org.bluetooth.service.automation_io                    | Automation IO                 | 0x1815
org.bluetooth.service.battery_service                  | Battery Service               | 0x180F
org.bluetooth.service.blood_pressure                   | Blood Pressure                | 0x1810
......

To convert a service into a .gatt file template, please call the script again with the requested Specification Type and the output file name
Usage: tool/convert_gatt_service.py SPECIFICATION_TYPE [service_name.gatt]
```

并且在最后会提示使用命令

```shell
Usage: tool/convert_gatt_service.py SPECIFICATION_TYPE [service_name.gatt]
```

生成 `.gatt` 文件，以 Battery Service 为例：

```shell
jackis@jackis-VirtualBox:~/btstack$ tool/convert_gatt_service.py org.bluetooth.service.battery_service battery_service.gatt
Fetching org.bluetooth.service.battery_service from https://www.bluetooth.com/wp-content/uploads/Sitecore-Media-Library/Gatt/Xml/Services/org.bluetooth.service.battery_service.xml

Service Battery Service
- Characteristic Battery Level - properties ['Read', 'Notify']
-- Descriptor Characteristic Presentation Format       - TODO: Please set values
-- Descriptor Client Characteristic Configuration     

Service successfully created battery_service.gatt
Please check for TODOs in the .gatt file
```

打开生成的 `battery_service.gatt` 文件：

```
// Specification Type org.bluetooth.service.battery_service
// https://www.bluetooth.com/wp-content/uploads/Sitecore-Media-Library/Gatt/Xml/Services/org.bluetooth.service.battery_service.xml

// Battery Service 180F
PRIMARY_SERVICE, ORG_BLUETOOTH_SERVICE_BATTERY_SERVICE
CHARACTERISTIC, ORG_BLUETOOTH_CHARACTERISTIC_BATTERY_LEVEL, DYNAMIC | READ | NOTIFY,
// TODO: Characteristic Presentation Format: please set values
#TODO CHARACTERISTIC_FORMAT, READ, _format_, _exponent_, _unit_, _name_space_, _description_
CLIENT_CHARACTERISTIC_CONFIGURATION, READ | WRITE,
```

可以看见它已经按照标准定义了相应的 service，characteristic 和 characteristic configuration。

## battery service server

BTstack 只提供了 ATT Server 的实现，GATT Server 的逻辑通过一个 GATT 编译器实现，该编译器将上述的 `*.gatt` 文件转换成了一个 `*.h` 文件，并在里面存储了 ATT Server 需要的 attribute table。

以 BTstack 实现的 /ble/gatt-service/battery_service_server.c 为例，学习一下如何实现一个 GATT Service 。

根据 **BAS_SPEC_V10** 的规定，Battery Service 的 UUID 为 «Battery Service» - 0x180F，其有一个 Characteristic 为 Battery Level，UUID 为 ，属性为 Read，Notify（可选）。因此来看 /ble/gatt-service/battery_service.gatt 文件：

```
// Specification Type org.bluetooth.service.battery_service
// https://www.bluetooth.com/api/gatt/xmlfile?xmlFileName=org.bluetooth.service.battery_service.xml

// Battery Service 180F
PRIMARY_SERVICE, ORG_BLUETOOTH_SERVICE_BATTERY_SERVICE
CHARACTERISTIC, ORG_BLUETOOTH_CHARACTERISTIC_BATTERY_LEVEL, DYNAMIC | READ | NOTIFY,
```

简单定义了一个 `PRIMARY_SERVICE` 和 `CHARACTERISTIC`，GATT 编译器会将该 `.gatt` 文件编译生成 `battery_service.h` 文件，里面包含：

```C
const uint8_t profile_data[] =
{
    // ATT DB Version
    1,
 
    // add Battery Service
    // #import <battery_service.gatt> -- BEGIN
    // Specification Type org.bluetooth.service.battery_service
    // https://www.bluetooth.com/api/gatt/xmlfile?xmlFileName=org.bluetooth.service.battery_service.xml
    // Battery Service 180F

    // 0x0004 PRIMARY_SERVICE-ORG_BLUETOOTH_SERVICE_BATTERY_SERVICE
    0x0a, 0x00, 0x02, 0x00, 0x04, 0x00, 0x00, 0x28, 0x0f, 0x18, 
    // 0x0005 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_BATTERY_LEVEL-DYNAMIC | READ | NOTIFY
    0x0d, 0x00, 0x02, 0x00, 0x05, 0x00, 0x03, 0x28, 0x12, 0x06, 0x00, 0x19, 0x2a, 
    // 0x0006 VALUE-ORG_BLUETOOTH_CHARACTERISTIC_BATTERY_LEVEL-DYNAMIC | READ | NOTIFY-''
    // READ_ANYBODY
    0x08, 0x00, 0x02, 0x01, 0x06, 0x00, 0x19, 0x2a, 
    // 0x0007 CLIENT_CHARACTERISTIC_CONFIGURATION
    // READ_ANYBODY, WRITE_ANYBODY
    0x0a, 0x00, 0x0e, 0x01, 0x07, 0x00, 0x02, 0x29, 0x00, 0x00, 
    // #import <battery_service.gatt> -- END

    // END
    0x00, 0x00, 
};
```

这里的 `profile_data[]` 数组就是 ATT 里使用的 attribute table，包含了 Battery Service 的实现。

**battery_service_server** 仅仅提供了 2 个接口，调用 `battery_service_server_init()` 函数初始化 server 并设置当前电量值，当电池电量发生变化时，调用 `battery_service_server_set_battery_value()` 函数更新值即可，内部细节被封装的很好，使用很方便。我们来看看这些接口是如何实现的，所涉及到的 ATT 接口都在 att_server.h 文件里。

在 init 函数里，首先调用 `gatt_server_get_get_handle_range_for_service_with_uuid16()` 函数获取指定 service uuid 在 attribute table 里的 start handle 和 end handle。因为 attribute table 是我们用户自己定义的，不同 service 的顺序可能不一样，所以需要调用该接口。然后利用接口 `gatt_server_get_value_handle_for_characteristic_with_uuid16()` 函数获取在 [start handle, end handle] 里指定 uuid 的 characteritic 的 vaule handle ，用于 ATT 读写。还可以利用 `gatt_server_get_client_configuration_handle_for_characteristic_with_uuid16()` 函数获取 client configuration 的 handle，供 ATT 读写。最后利用 `att_server_register_service_handler()` 函数注册了一个**读写**回调函数，

```C
// register service with ATT Server
battery_service.start_handle   = start_handle;
battery_service.end_handle     = end_handle;
battery_service.read_callback  = &battery_service_read_callback;
battery_service.write_callback = &battery_service_write_callback;
att_server_register_service_handler(&battery_service);
```

当 ATT 读写 [start_handle, end_handle] 之间的 attribute 时，都会回调注册的函数。先看看注册的写回调函数：

```C
static int battery_service_write_callback(hci_con_handle_t con_handle, uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size){
	UNUSED(transaction_mode);
	UNUSED(offset);
	UNUSED(buffer_size);

	if (attribute_handle == battery_value_handle_client_configuration){
		battery_value_client_configuration = little_endian_read_16(buffer, 0);
		battery_value_client_configuration_connection = con_handle;
	}
	return 0;
}
```

当本次写的 attribute handle 是之前存储的 client configuration handle 时，从写入数据 `buffer` 读取了 16bit 的数据。这就是写入的 client configuration value 值。

读回调函数里主要处理了读取的数据：

```C
static uint16_t battery_service_read_callback(hci_con_handle_t con_handle, uint16_t attribute_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size){
	UNUSED(con_handle);

	if (attribute_handle == battery_value_handle_value){
		return att_read_callback_handle_byte(battery_value, offset, buffer, buffer_size);
	}
	if (attribute_handle == battery_value_handle_client_configuration){
		return att_read_callback_handle_little_endian_16(battery_value_client_configuration, offset, buffer, buffer_size);
	}
	return 0;
}
```

 当读取的 attribute handle 是之前存储的 battery value characteristic handle 时，利用接口 `att_read_callback_handle_byte()` 向 `buffer` 里写入一个字节的数据。当然还可以写入 16bit，32bit 和任意bit 的数据，接口为 `att_read_callback_handle_*()` 。

battery service server 提供的另一个接口用于更新电池电量：

```C
void battery_service_server_set_battery_value(uint8_t value){
	battery_value = value;
	if (battery_value_client_configuration != 0){
		battery_callback.callback = &battery_service_can_send_now;
		battery_callback.context  = (void*) (uintptr_t) battery_value_client_configuration_connection;
		att_server_register_can_send_now_callback(&battery_callback, battery_value_client_configuration_connection);
	}
}
```

由于 BTstack 内部设计，不是每一个时刻都可以发生数据，需要等待协议栈内部准备好，因此这里用 `att_server_register_can_send_now_callback()` 函数注册了一个回调函数，当协议栈内部好可以发送 ATT 数据时会回调它：

```C
static void battery_service_can_send_now(void * context){
	hci_con_handle_t con_handle = (hci_con_handle_t) (uintptr_t) context;
	att_server_notify(con_handle, battery_value_handle_value, &battery_value, 1);
}
```

在回调函数里利用 `att_server_notify()` 函数发送一个 notify 。

## att_server

本小节从整体角度考虑 att_server.h 里的接口该如何使用。

首先通过 `att_server_init()` 初始化 ATT Server，该函数可传入一个读回调函数，一个写回调函数，用于 ATT 读写一条 arrtibute 的情况。而 `att_server_register_service_handler()` 函数也可以注册读写回调函数，不过其优先级更高，而且只对 [start_handle, end_handle] 之间的 attribute 读写有效。

由于 BTstack 并不能在任意时刻发送数据，因此需要注册回调函数，等待合适的时候协议栈回调该函数，执行发送动作。共有两种类型的接口：

1. 利用 *ATT_EVENT_CAN_SEND_NOW* 事件的发送方式。
2. 利用注册的特定回调函数完成发送。

类型 1 的接口需要通过 `att_server_register_packet_handler()` 函数注册一个回调函数，在该回调函数里判断是否收到 *ATT_EVENT_CAN_SEND_NOW* 事件，若收到则进行发送操作，例如：

```C
static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    UNUSED(channel);
    UNUSED(size);

    switch (packet_type) {
        case HCI_EVENT_PACKET:
            switch (hci_event_packet_get_type(packet)) {
                case ATT_EVENT_CAN_SEND_NOW:
                    att_server_notify(con_handle, ATT_CHARACTERISTIC_0000FF11_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE, (uint8_t*) counter_string, counter_string_len);
                    break;
            }
            break;
    }
}
```

当然配套使用的接口 `att_server_request_can_send_now_event()` 用于发送 *ATT_EVENT_CAN_SEND_NOW* 事件。

类型 2 的接口 `att_server_register_can_send_now_callback()`，`att_server_request_to_send_notification()` 和 `att_server_request_to_send_indication()` 这些接口注册一个回调函数，该函数回调时可以用 `att_server_notify()` 和 `att_server_indicate()` 发送数据。

----

参考：

- [BTstack - GATT Server](http://bluekitchen-gmbh.com/btstack/#profiles/#gatt-server)
- [Example - GATT Counter](http://bluekitchen-gmbh.com/btstack/#examples/examples/#sec:gattcounterExample)