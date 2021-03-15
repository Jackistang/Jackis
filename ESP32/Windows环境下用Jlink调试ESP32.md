# Windows环境下用Jlink调试ESP32

本文记录了我在 Windows 环境下用 Jlink 调试 ESP32 时遇到的问题，以及相应的解决办法，并在文章末尾处附加了解决问题时查找的资料。

调试之前首先需要将 Jlink 与 ESP32 相连，接口图如下：

| ESP32 pin     | JTAG probe pin |
| ------------- | -------------- |
| 3.3V          | Pin 1(VTref)   |
| GPIO 9 (EN)   | Pin 3 (nTRST)  |
| GND           | Pin 4 (GND)    |
| GPIO 12 (TDI) | Pin 5 (TDI)    |
| GPIO 14 (TMS) | Pin 7 (TMS)    |
| GPIO 13 (TCK) | Pin 9 (TCK)    |
| GPIO 15 (TDO) | Pin 13 (TDO)   |

引脚图如下：

![](https://i.loli.net/2021/03/01/oHXTL6vx8qpRd1O.jpg)

Jlink 与 ESP32 连接好后，我们就可以开始调试了。

ESP32 利用 openocd 来进行调试，我们可以使用命令 `openocd -f interface/jlink.cfg -f board/esp-wroom-32.cfg` 来启动一个 gdb server ，启动成功后我们就可以使用 gdb（交叉编译器版本）连接这个 server 了。但是遇到了下述问题，

## Jlink 驱动

错误信息：

```shell
PS C:\Users\M2019\.platformio\packages\tool-openocd-esp32> .\bin\openocd.exe -f interface/jlink.cfg -f board/esp-wroom-32.cfg
Open On-Chip Debugger  v0.10.0-esp32-20201202 (2020-12-02-17:38)
Licensed under GNU GPL v2
For bug reports, read
        http://openocd.org/doc/doxygen/bugs.html
WARNING: boards/esp-wroom-32.cfg is deprecated, and may be removed in a future release.
Info : FreeRTOS creation
Info : FreeRTOS creation
Info : Listening on port 6666 for tcl connections
Info : Listening on port 4444 for telnet connections
Warn : Failed to open device: LIBUSB_ERROR_NOT_SUPPORTED.
Error: No J-Link device found.
```

错误信息提示没有找到 J-Link 设备，这是由于 Windows 下没有 libusb，需要更新 Jlink 驱动。解决办法如下：

1. 从 [Zadig 官网](http://zadig.akeo.ie/) 下载 Zadig 工具（Zadig_X.X.exe）并运行。
2. 在 Zadig 工具中，进入 “Options” 菜单中选中 “List All Devices”。
3. 检查设备列表，选择相应的 Jlink 驱动，注意设备列表的 Jlink 驱动可能没有 Jlink 名称，然后点击 "Replace Driver"。

我这里用两个 Jlink （不同固件）测试，驱动名称分别为：

![](https://i.loli.net/2021/03/01/PmrV3fIyKehklcu.png)

![](https://i.loli.net/2021/03/01/6YhpmECkIFfVbHu.png)

---

参考：

- [配置 ESP-WROVER-KIT 上的 JTAG 接口](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-guides/jtag-debugging/configure-ft2232h-jtag.html)
- [如何使用 J-Link 调试 ESP32](https://docs.ai-thinker.com/esp32/docs/esp32_openocd)
- [Esp32 , OpenOCD ,JTAG Error](https://esp32.com/viewtopic.php?t=8517)

## Jlink 时钟问题

更新了 Jlink 驱动后，又遇见了下述问题：

```shell
PS C:\Users\M2019\.platformio\packages\tool-openocd-esp32> .\bin\openocd.exe -s share/openocd/scripts -f interface/jlink.cfg -f board/esp-wroom-32.cfg
Open On-Chip Debugger  v0.10.0-esp32-20201202 (2020-12-02-17:38)
Licensed under GNU GPL v2
For bug reports, read
        http://openocd.org/doc/doxygen/bugs.html
WARNING: boards/esp-wroom-32.cfg is deprecated, and may be removed in a future release.
Info : FreeRTOS creation
Info : FreeRTOS creation
Info : Listening on port 6666 for tcl connections
Info : Listening on port 4444 for telnet connections
Info : J-Link V9 compiled Dec 13 2019 11:14:50
Info : Hardware version: 9.70
Info : VTarget = 3.287 V
Error: JTAG I/O operation failed: 0x1.
Error: jaylink_jtag_io() failed: device: unspecified error.
Error: An adapter speed is not selected in the init script. Insert a call to adapter_khz or jtag_rclk to proceed.

Assertion failed!
```

第一个错误提示为 `Error: JTAG I/O operation failed: 0x1.`，这种问题会出现在 openocd-esp32 2020 及以后的版本。

解决办法为：

- 进入 `openocd-esp32/share/openocd/scripts/interface` 目录，在 `interface` 文件夹中找到 `jlink.cfg` 并打开。
- 在 `jlink.cfg` 中添加 `adapter_khz 9600`，然后保存文件并关闭。

这样，就能成功地启动 gdb server 了。

---

参考：

- [ESP32 使用 Jlink 进行 JTAG 调试的流程](https://blog.csdn.net/zztiger123/article/details/105516768)
- [使用 ESP-Prog / Jlink 进行 JTAG 调试时的常见错误及解决办法](https://blog.csdn.net/zztiger123/article/details/106527952)



-----

办法总比困难多，遇见困难就是一个提升自我的过程。