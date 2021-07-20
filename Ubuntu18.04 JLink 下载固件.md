# Ubuntu 18.04 JLink 下载固件

我最近在 Linux 平台开发 RT-Thread 的程序，借助 VSCode 插件 RT-Thread Studio 能够很方便地编译下载和调试 QEMU 程序，但这次使用插件下载固件到 nRF52840DK 时发生了问题，因此我比较系统地了解了 JLink 如何下载固件，并解决了 RT-Thread Studio 插件里遗留的 BUG ，因此便有了这篇文章。

## JLink 环境搭建

JLink 环境搭建比较简单，从 SEGGER 官网下载 [J-Link Software and Documentation Pack](https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack)，并安装即可，网上有很多教程，我就不再赘述了。

## JLink 命令行下载固件

解决问题需要从根源找起，RT-Thread Studio 插件下载固件，最终也是调用的 SEGGER 提供的 JLink 工具，只要我们弄懂了最原始的 JLink 工具如何下载程序，那么就能够很明显地发现插件里的问题。

JLink 工具里有下载功能的是 JLinkExe，Windows 系统里的名称是 JLink.exe 。JLinkExe 里有一系列的命令可以执行，详细命令可以参考 [J-Link Commander](https://wiki.segger.com/J-Link_Commander)，下面来演示一下如何用 JLinkExe 下载固件。

首先进入 "bsp/nrf5x/nrf52840" 目录，输入 `JLinkExe` 命令，进入了交互式环境，

![](https://i.loli.net/2021/07/16/6B2ptAwTnLqrh8Y.png)

然后再输入 `device NRF52840_XXAA` 设置设备名称；输入 `if SWD` 选择 SWD 接口，if 是 interface 的意思；最后输入 `speed 4000` 设置 SWD 接口的速度为 4000 kHz 。

![](https://i.loli.net/2021/07/16/fVOog8wsIEP9uNv.png)

再输入 `connect` 连接设备，

![](https://i.loli.net/2021/07/16/HDL21xZrj3ehYNJ.png)

这个时候就已经可以下载固件了，使用命令

```shell
loadbin /home/tangjia/Jackistang/rt-thread/bsp/nrf5x/nrf52840/rtthread.bin  0x0
```

下载固件，这里 `loadbin` 就是下载命令，`/home/tangjia/Jackistang/rt-thread/bsp/nrf5x/nrf52840/rtthread.bin` 就是 bin 格式的固件，`0x0` 是下载的起始地址，nRF52840 是从 0x0 地址开始的，而 STM32 一般是从 0x08000000 开始的。

最后输入 `q` 退出 JLinkExe 交互模式，就可以看见 LED1 已经在闪烁了。

上面介绍了 JLinkExe 如何使用交互式环境下载固件，但是在实际应用过程中，更倾向于一条命令就完成下载功能，而无需交互式。

JLinkExe 也提供了相应的方案，[J-Link_Commander - Batch_processing](https://wiki.segger.com/J-Link_Commander#Batch_processing) 提出了批处理的方法，它将一些交互式的命令导出成了命令行参数使用，例如上述的一些交互式配置命令可以改为：

```shell
JLinkExe -device NRF52840_XXAA -if SWD -speed 4000 -autoconnect 1
```

但是 `loadbin` 下载固件的命令并不能这么使用，那该如何下载呢？

[J-Link_Commander - Using J-Link Command Files](https://wiki.segger.com/J-Link_Commander#Using_J-Link_Command_Files) 提供了解决办法，J-Link Command Files 是指将一系列的 JLinkExe 的交互式命令存储在一个文件里，例如 "CommandFile.jlink"，可以通过命令

```shell
JLinkExe -CommandFile /home/tangjia/CommandFile.jlink
```

来加载命令文件，命令文件里的交互式命令会依次执行，我们可以在命令文件里面放入 `loadbin /home/tangjia/Jackistang/rt-thread/bsp/nrf5x/nrf52840/rtthread.bin  0x0` ，并最终执行命令：

```shell
JLinkExe -device NRF52840_XXAA -if SWD -speed 4000 -autoconnect 1 -CommandFile /home/tangjia/CommandFile.jlink
```

这样就实现了使用 JLink 一键下载的功能。

## VSCode 插件 RT-Thread Studio BUG 解决

在理解了上述 JLink 下载固件的基本概念后，我们就可以来解决 RT-Thread Studio 插件下载的 BUG 了。

点击**下载**按钮，在 TERMINAL 终端会显示执行的命令，

```shell
tangjia@FA001334:~/Jackistang/rt-thread/bsp/nrf5x/nrf52840$ "/opt/SEGGER/JLink/JLink.exe" -device NRF52840_XXAA -CommandFile "/home/tangjia/Jackistang/rt-thread/bsp/nrf5x/nrf52840/.rtthread_jlink_download"
bash: /opt/SEGGER/JLink/JLink.exe: 没有那个文件或目录
```

报错信息显示不存在 /opt/SEGGER/JLink/JLink.exe 这个文件，这个文件是 Windows 系统上的 JLink 名称，Linux 平台上为 "JLinkExe"，但是插件里不能配置，于是我使用 `sudo ln -s /opt/SEGGER/JLink/JLinkExe /opt/SEGGER/JLink/JLink.exe ` 创建一个软链接来临时使用。

然后就执行了命令：

```shell
"/opt/SEGGER/JLink/JLinkExe" -device NRF52840_XXAA -CommandFile "/home/tangjia/Jackistang/rt-thread/bsp/nrf5x/nrf52840/.rtthread_jlink_download"
```

可以很清楚地看见使用 **Batch_processing** 指定了设备名称为 **NRF52840_XXAA**，这个名称是在插件里配置的，然后又指定了一个 **J-Link Command Files**，`/home/tangjia/Jackistang/rt-thread/bsp/nrf5x/nrf52840/.rtthread_jlink_download`，是在 bsp 目录下的 `.rtthread_jlink_download` 文件，打开这个文件，里面有一系列交互式命令：

```
si 1
si 1
speed 15000
r
h
loadbin rtthread.bin 0x08000000
setpc 0x08000000
r
g
q

```

`si 1` 是用来设置 SWD 接口，

![](https://i.loli.net/2021/07/16/aGJTmwbxpZQVEHo.png) 

`speed 15000` 是用来设置接口速度，

`r` 用来复位设备，

![](https://i.loli.net/2021/07/16/pYjhBmZLDQn5t9P.png)

`h` 用于中断设备，

之后就使用 `loadbin rtthread.bin 0x08000000` 下载固件到 0x08000000，

并且利用命令 `setpc 0x08000000` 将 PC 指针设置为 0x08000000，

然后在用 `r` 复位设备，`g` 启动设备 CPU，最后用 `q` 退出交互式环境。

这样，整个下载过程就完成了，并且下载完成后还自动复位了设备。

---

VSCode 插件 RT-Thread Studio 的 BUG 记录：

1. "JLink.exe" 的名称问题，Linux 系统上名称为 "JLinkExe" 。
2. 固件下载地址的问题，使用插件的**下载**功能时，会自动在 bsp 目录下生成 `.rtthread_jlink_download` 文件，并且其下载地址固定为 0x08000000，更改后也没有用，因为下次下载时这个文件又会变成默认的配置。这里需要改进一下。

