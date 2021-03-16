# Android手机获取btsnoop

从我目前了解的情况来看，Android 手机获取 btsnoop 根据手机厂家不同，获取的难易程度也不一样。

常规操作是打开手机的**“开发者选项”**，如何打开手机对应型号的“开发者选项”请自行百度；然后再打开**“启动蓝牙HCI信息收集日志”**，

![](https://i.loli.net/2021/03/15/92NJLqpjV8n7EDZ.png)

重新开启蓝牙，如果幸运的话，可以直接在手机文件系统里搜到 `btsnoop_hci.log` 文件。

![](https://i.loli.net/2021/03/15/KBEo3ur1sIkbd4q.png)

如果没有该文件，请继续看下面的操作。

----

下述操作手机需要有 root 权限，蓝绿厂手机就不用考虑了，推荐小米。

先在电脑上安装 `platform-tools` 工具链 ，里面有Android Debug Bridge（adb）工具，下载[工具链](https://github.com/Jackistang/Jackis/blob/main/BLE/platform-tools_r31.0.0-windows.zip)，解压到某个文件夹，并且将该工具的目录路径添加到 PATH 环境变量。

手机通过 USB 连接电脑，并且开启**“USB调试模式”**，PC 端打开 cmd，输入 `adb devices` 就能够看到手机设备了。

```shell
C:\Users\M2019\Desktop>adb devices
List of devices attached
f8761e1 device
```

下一步需要在手机端安装 `adbd-insecure.apk` 文件，先下载该[文件](https://github.com/Jackistang/Jackis/blob/main/BLE/adbd-Insecure-v2.00.apk)，在文件路径下输入 `adb install adbd-insecure.apk` 安装，手机端同意即可。

```shell
C:\Users\M2019\Desktop>ls
'BLE Mesh model.md'   adbd-Insecure-v2.00.apk  'ble mesh.md'   btsnoop_hci.log   desktop.ini   esp32.md   zephyr.md

C:\Users\M2019\Desktop>adb install adbd-Insecure-v2.00.apk
Performing Push Install
adbd-Insecure-v2.00.apk: 1 file pushed, 0 skipped. 42.4 MB/s (752250 bytes in 0.017s)
        pkg: /data/local/tmp/adbd-Insecure-v2.00.apk
```

之后手机打开该软件，将 “Enable insecure adbd” 和 “Enable at boot” 勾选。PC 端再输入 `adb root` ，没有报错即可进行下一步。

![](https://i.loli.net/2021/03/14/mPXvp54I93SVUjO.png)

```shell
C:\Users\M2019\Desktop>adb root
adbd is already running as root
```

依次输入

```shell
adb root
adb remount
adb pull /etc/bluetooth/bt_stack.conf C:\Users\<your name>\Desktop
```

将 bt_stack.conf 文件从手机上导出到电脑桌面，然后在电脑桌面修改该文件，将选项 `BtSnoopLogOutput` 使能，即修改成下述形式。

```shell
# Enable BtSnoop logging function
# valid value : true, false
BtSnoopLogOutput=true
```

`BtSnoopFileName` 选项值就是 `btsnoop_hci.log` 最终存储的位置。 

```shell
# BtSnoop log output file
BtSnoopFileName=/sdcard/btsnoop_hci.log
```

再输入

```shell
adb push C:\Users\<your name>\Desktop\bt_stack.conf /etc/bluetooth
```

覆盖掉原始配置文件。

如果上述步骤都没有错误，手机重新打开蓝牙，就能够找到文件 `/sdcard/btsnoop_hci.log` 了，输入

```shell
adb pull /sdcard/btsnoop_hci.log C:\Users\<your name>\Desktop
```

将 btsnoop 导出到桌面，之后就可以分析协议内容了。

---

如果有问题欢迎交流，这毕竟也只是我试验出来的一种方法。

参考：

- [使用Android手机抓取蓝牙HCI包](https://supperthomas-wiki.readthedocs.io/en/latest/03_xupenghu/04_btsnoop_hci/android_hci.html)
- [Android Bluetooth HCI log 详解](https://www.jianshu.com/p/73f7366161d1)
- [Android问题adbd cannot run as root in production builds解决](https://blog.csdn.net/JiYaRuo/article/details/107788033)
- [Android使用adb命令直接修改文件](https://blog.csdn.net/weixin_33978044/article/details/91689567)