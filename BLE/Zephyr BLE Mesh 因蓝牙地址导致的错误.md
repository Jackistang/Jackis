# Zephyr BLE Mesh 因蓝牙地址导致的错误

本文记录了我在利用 Zephyr 学习 BLE Mesh 过程中，遇到了一个因蓝牙地址导致的错误，以及解决该问题的过程，希望以后学习的人能够少踩坑。

## 背景

我按照 [Zephyr - Bluetooth 环境搭建 - QEMU 篇](https://www.jianshu.com/p/6d86f879639a) 搭建好了 Zephyr 的 Bluetooth 环境，虚拟机上运行的 Ubuntu，物理机（Win10）上的蓝牙控制器接入虚拟机使用。当我运行 `samples/bluetooth/mesh` 例程时，手机 nRF Mesh 上没有任何现象出现，nRF Connect 上也没有收到 unprovisioned beacon 。这明显是有问题的，于是我打开了 bluetooth 的日志，在 `prj.conf` 文件里添加

```C
CONFIG_BT_DEBUG_LOG=y
```

之后我就看到了报错信息：

```shell
Bluetooth initialized
Mesh initialized
[00:00:00.210,000] <inf> bt_hci_core: No ID address. App must call settings_load()
[00:00:00.230,000] <inf> bt_hci_core: Identity: 58:A0:23:B6:A4:80 (public)
[00:00:00.230,000] <inf> bt_hci_core: HCI: version 5.0 (0x09) revision 0x0100, manufacturer 0x0002
[00:00:00.230,000] <inf> bt_hci_core: LMP: version 5.0 (0x09) subver 0x0100
[00:00:00.230,000] <inf> bt_mesh_prov_device: Device UUID: 00000000-0000-0000-0000-0000000~
[00:00:00.430,000] <wrn> bt_hci_core: opcode 0x2005 status 0x0c
[00:00:00.430,000] <err> bt_mesh_adv_legacy: Advertising failed: err -5
[00:00:05.370,000] <wrn> bt_hci_core: opcode 0x2005 status 0x0c
[00:00:05.370,000] <err> bt_mesh_adv_legacy: Advertising failed: err -5

```

可以看见第一个 **warning** 是运行 opcode 为 0x2005 的 hci 操作时，返回的状态是 0x0c，我在 Core Spec 里找了相关内容，如下

hci command:

- opcode: 0x2005 -- LE Set Random Address command
- status: 0x0c -- Command disallowed

> The Command Disallowed error code indicates that the command requested cannot be executed because the Controller is in a state where it cannot process this command at this time. This error shall not be used for command OpCodes where the error code Unknown HCI Command is valid. 

通过相应的提示可以看出这是因为蓝牙 Host 给 Controller 发送设置随机地址的 HCI 命令，Controller 返回 Host 操作不允许。

至于另一个 **error** 我也在源代码里面找到了描述：

```C
#define	EIO		 5	/* I/O error */
```

但这个好像用处不大。

## 解决过程

上面把问题出现的过程，以及问题的分析已经说的很全面了，现在该如何解决了。

最开始我直接复制了错误提示去 Zephyr GitHub 项目的 issue 里搜索，想看看有没有人遇到跟我一样的问题，很可惜没有。

然后我又怀疑是虚拟机的蓝牙 Controller 出现了问题，于是我尝试运行 `samples/bluetooth/beacon` 例程，发现能够正常收到 beaon 广播包。这说明 Controller 那一端没有问题，能够正常打广播，应该是 Host 这边使用方法不当。

最后没办法了，我重新看了一遍 Zephyr 里关于搭建 bluetooth 环境的教程，发现了一些关键点：

> There are 4 possible hardware setups to use with Zephyr and Bluetooth:
>
> 1. Embedded
> 2. QEMU with an external Controller
> 3. Native POSIX with an external Controller
> 4. Simulated nRF52 with BabbleSim

这介绍了 Zephyr Bluetooth 可以使用的 4 种硬件环境，我这里是 QEMU with an external Controller 环境。

> **Host on Linux with an external Controller**
>
> This setup relies on a “dual-chip” [configuration](https://docs.zephyrproject.org/latest/guides/bluetooth/bluetooth-arch.html#bluetooth-configs) which is comprised of the following devices:
>
> 1. A [Host-only](https://docs.zephyrproject.org/latest/guides/bluetooth/bluetooth-arch.html#bluetooth-build-types) application running in the [QEMU](https://docs.zephyrproject.org/latest/application/index.html#application-run-qemu) emulator or the `native_posix` native port of Zephyr
> 2. A Controller, which can be one of two types:
>    - A commercially available Controller
>    - A [Controller-only](https://docs.zephyrproject.org/latest/guides/bluetooth/bluetooth-arch.html#bluetooth-build-types) build of Zephyr

然后它这里又介绍了 Host-only 程序运行在 QEMU 模拟器中，Controller 又分为两种类型：

- 商用 Controller
- 用 Zephyr 自己构建 Controller

看到这里我就突然理解前面为什么 HCI 命令 `LE Set Random Address command` 会报错了，商业 Controller 应该不会允许你私自更改地址的，它应该是一个固定值，输入 `hciconfig` 可以看到地址是已有了的。

```shell
jackis@jackis-VirtualBox:~/bluez$ hciconfig
hci0:	Type: Primary  Bus: USB
	BD Address: 58:A0:23:B6:A4:80  ACL MTU: 1021:4  SCO MTU: 96:6
	UP RUNNING 
	RX bytes:814 acl:0 sco:0 events:62 errors:0
	TX bytes:3230 acl:0 sco:0 commands:62 errors:0
```

但是 BLE Mesh 为了保证网络的安全性，地址都是随机的，因为 Controller 允许该特性，所以我就在 Zephyr 的内核配置 `guiconfig` 界面搜索 `bt address` 关键字，找到了本问题的解决方法 `BT_MESH_DEBUG_USE_ID_ADDR`，如下图：

![](https://i.loli.net/2021/03/18/vKaVBQjYTSR841n.png)

根据介绍可知，该配置选项开启后，强迫使用本地身份地址进行广播操作，这对于 debug 十分有用，但是在产品中绝对不能使用。

选中它，重新编译运行程序，已经没有报错信息了，unprovisioned beacon 也收到了。

也可以在 `prj.conf` 里加入下述代码来配置该功能

```C
CONFIG_BT_MESH_DEBUG_USE_ID_ADDR=y
```



-----

每遇见一个问题，都是对你以往学习知识的一个回顾，解决它就能进步。