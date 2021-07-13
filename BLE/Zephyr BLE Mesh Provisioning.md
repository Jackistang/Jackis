# Zephyr BLE Mesh Provisioning

## Beaconing

[`bt_mesh_init()` ](https://docs.zephyrproject.org/2.4.0/reference/bluetooth/mesh/core.html#c.bt_mesh_init) 函数里的 `prov` 参数包含了发送 Unprovisioned Beacon 包的内容，`uuid`, `oob_info` 和 `uri` 。当使用 [`bt_mesh_prov_enable()`](https://docs.zephyrproject.org/2.4.0/reference/bluetooth/mesh/provisioning.html#c.bt_mesh_prov_enable) 使能了某个特定承载层后（PB-ADV 或 PB-GATT），该设备开始在指定的承载层发送 Unprovisioned Beacon 。

Mesh Shell 里，

| 命令               | 函数                                      |
| :----------------- | :---------------------------------------- |
| `mesh init`        | `bt_mesh_init()`                          |
| `mesh pb-adv on`   | `bt_mesh_prov_enable(BT_MESH_PROV_ADV)`   |
| `mesh pb-adv off`  | `bt_mesh_prov_disable(BT_MESH_PROV_ADV)`  |
| `mesh pb-gatt on`  | `bt_mesh_prov_enable(BT_MESH_PROV_GATT)`  |
| `mesh pb-gatt off` | `bt_mesh_prov_disable(BT_MESH_PROV_GATT)` |

先输入 `mesh init` 命令，然后在不同承载层发送 unprovisioned beacon 的命令及现象如下图所示：

![](https://i.loli.net/2021/03/18/WGCBOnN5icIAbxU.png)

## 配网流程

Mesh shell 通过 PB-ADV provision 一个设置的流程如下，Zephyr 好像不支持 PB-GATT 的 provisioner 。

```shell
uart:~$ mesh init
Bluetooth initialized
Mesh initialized
Use "pb-adv on" or "pb-gatt on" to enable advertising

uart:~$ mesh cdb-create

uart:~$ mesh provision 0 0
Local node provisioned, net_idx 0x0000 address 0x0001

uart:~$ mesh beacon-listen on
UUID dddda8032aec0d6e0000000000000000, OOB Info 0x0000, URI Hash 0x0
UUID dddda8032aec0d6e0000000000000000, OOB Info 0x0000, URI Hash 0x0
UUID dddda8032aec0d6e0000000000000000, OOB Info 0x0000, URI Hash 0x0

uart:~$ mesh provision-adv dddda8032aec0d6e0000000000000000 0 0x0100 0
Provisioning link opened on PB-ADV
Node provisioned, net_idx 0x0000 address 0x0100 elements 3
Provisioning link closed on PB-ADV
```

很明显能够看到 provision 成功。

其中 [`mesh cdb-create`](https://docs.zephyrproject.org/latest/reference/bluetooth/mesh/shell.html#mesh-cdb-create-netkey) 用于创建配置数据库（Configuration database），该数据库只在网络管理者 provisioner 上有一份实例，用于存储 mesh 网络中的所有信息。`mesh cdb-` 还有很多其他功能，比如显示数据库功能，创建网络密钥(NetKey)，创建应用密钥(AppKey)。

[`mesh provision`](https://docs.zephyrproject.org/latest/reference/bluetooth/mesh/shell.html#mesh-provision-netkeyindex-addr-ivindex) 用于 provision 节点本身。

[`mesh beacon-listen`](https://docs.zephyrproject.org/latest/reference/bluetooth/mesh/shell.html#mesh-beacon-listen-val-off-on) 命令用于开启或关闭接收 unprovisioning beacon 的能力。

[`mesh provision-adv`](https://docs.zephyrproject.org/latest/reference/bluetooth/mesh/shell.html#mesh-provision-adv-uuid-netkeyindex-addr-attentionduration) 命令用于配网一个设备，将其作为节点加入网络。

| 命令                                                         | 函数                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| `mesh cdb-create`                                            | `bt_mesh_cdb_create(net_key)`                                |
| `mesh provision <NetKeyIndex> <addr> [IVIndex]`              | `bt_mesh_provision(net_key, net_idx, 0, iv_index, addr, default_key)` |
| `mesh beacon-listen <val: off, on>`                          | `prov.unprovisioned_beacon`                                  |
| `mesh provision-adv <UUID> <NetKeyIndex> <addr> <AttentionDuration>` | `bt_mesh_provision_adv(uuid, net_idx, addr, attention_duration)` |

 

| 命令                                                 | 函数                                                         |
| ---------------------------------------------------- | ------------------------------------------------------------ |
| `mesh get-comp [page]`                               | `bt_mesh_cfg_comp_data_get(net.net_idx, net.dst, page, &status, &comp)` |
| `mesh app-key-add <NetKeyIndex> <AppKeyIndex> [val]` | `bt_mesh_cfg_app_key_add(net.net_idx, net.dst, key_net_idx, key_app_idx, key_val, &status)` |
| `mesh app-key-add <NetKeyIndex> <AppKeyIndex> [val]` | `bt_mesh_trans_send(&tx, &msg, NULL, NULL)`                  |





```
CONFIG_BT_PERIPHERAL=y
CONFIG_BT_CENTRAL=y
CONFIG_BT_MESH_PB_GATT=y
CONFIG_BT_MESH_GATT_PROXY=y
```



----

# BUG

```shel
<err> bt_driver: Not enough space in buffer

ASSERTION FAIL [err == 0] @ WEST_TOPDIR/zephyr/subsys/bluetooth/host/hci_core.c:333
	k_sem_take failed with err -11
[00:50:32.680,000] <err> os: EAX: 0x00138a94, EBX: 0x00138a94, ECX: 0x0025a584, EDX: 0x0025a5b4
[00:50:32.680,000] <err> os: ESI: 0xfffffff5, EDI: 0x00002008, EBP: 0x0025a5e8, ESP: 0x0025a5b4
[00:50:32.680,000] <err> os: EFLAGS: 0x00000202 CS: 0x0008 CR3: 0x00260000
[00:50:32.680,000] <err> os: call trace:
[00:50:32.680,000] <err> os: EIP: 0x0010f709
[00:50:32.680,000] <err> os:      0x0010fb87 (0x2008)
[00:50:32.680,000] <err> os:      0x0010fc64 (0x10f77f)
[00:50:32.680,000] <err> os:      0x00111605 (0x0)
[00:50:32.680,000] <err> os:      0x00111747 (0x25bd78)
[00:50:32.680,000] <err> os:      0x0012353e (0x25a708)
[00:50:32.690,000] <err> os:      0x00101166 (0x0)
[00:50:32.690,000] <err> os: >>> ZEPHYR FATAL ERROR 3: Kernel oops on CPU 0
[00:50:32.700,000] <err> os: Current thread: 0x143160 (BT Mesh adv)
[00:50:32.750,000] <err> os: Halting system

```

