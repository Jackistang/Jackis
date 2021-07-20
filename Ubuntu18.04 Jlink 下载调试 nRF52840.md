# Ubuntu18.04 JLink GDB 调试 nRF52840

上文[Ubuntu 18.04 JLink 下载固件](https://club.rt-thread.org/ask/article/2890.html)中解决了如何在 Ubuntu 环境中使用 JLink 下载固件到 nRF52840DK，本文则聚焦于如何使用 gdb 调试 nRF52840DK，并使用 VSCode 自带的调试功能进行 Debug 。

本文按照从易到难分为三个部分：

1. 命令行启动 GDB Server 和 GDB Client
2. 命令行启动 GDB Server，VSCode 启动 GDB Client
3. VSCode 同时启动 GDB Server 和 GDB Client

在使用 GDB 调试开发板上的程序时，一般先利用调试工具（JLink，ST-Link）提供的 GDBServer 工具启动一个 GDB Server 进程，并绑定端口；另一端利用调试前端（arm-none-eabi-gdb）通过端口去连接这个 Server 进程，依此来达到调试的目的。

## 命令行启动 GDB Server 和 GDB Client

JLink 工具箱里面提供了 JLinkGDBServer 来启动 GDB Server，我们现在来实验，开启一个命令行，输入

```shell
JLinkGDBServer -device nrf52840_xxAA -if SWD -speed 4000
```

去连接设备，其中 -device, -if, -speed 的作用参考前文[Ubuntu 18.04 JLink 下载固件](https://club.rt-thread.org/ask/article/2890.html)。这样我们就成功地利用 JLink 开启了一个 GDB Server，默认端口为 2331 。

现在让我们来连接 GDB Server，在 "bsp/nrf5x/nrf52840" 目录下，输入

```shell
/opt/gcc-arm-none-eabi-6_2-2016q4/bin/arm-none-eabi-gdb rt-thread.elf
```

打开 GDB Client ，这个时候还不能调试，因为 Client 端还并未和 Server 相连，需要我们手动输入

```shell
target remote localhost:2331
```

这样 Client 就通过 “localhost:2331” 这个本地端口连接上了 Server，于是输入 `c` 运行程序。

## 命令行启动 GDB Server，VSCode 启动 GDB Client

还是上述的例子，GDB Server 的启动仍是输入命令：

```shell
JLinkGDBServer -device nrf52840_xxAA -if SWD -speed 4000
```

而 GDB Client 我们则使用 VSCode 来启动，因为这样就可以 VSCode 的图形化工具来调试，而不需要自己敲命令来调试了。

VSCode 的调试配置文件是 launch.json，里面有一些与本次实验相关的配置项，罗列在了下方：

- `program` 是 debug 的可执行文件的路径
- `setupCommands` JSON 数组里存储了一系列的 GDB 命令，在启动 GDB 后会自动在命令行里输入这些命令。
- `miDebuggerPath` 是 gdb 调试器的路径
- `miDebuggerServerAddress` 是调试远端设备时使用的，形式为 "ip address:port"，例如 "localhost:1234" 。

想一想我们最开始使用命令行启动 GDB Client 时，使用的命令是：

```shell
/opt/gcc-arm-none-eabi-6_2-2016q4/bin/arm-none-eabi-gdb rt-thread.elf
```

这里面有 gdb 调试器路径 - "miDebuggerPath"，需要调试的可执行文件 - program 。

然后我们在启动 GDB Client 后，还输入了一条命令 `target remote localhost:2331` 去连接 GDB Server，VSCode 里的 miDebuggerServerAddress 就是配置 Server 的端口号的。

关于 launch.json 其他配置项的描述，可以参考文档[VSCode 调试](https://supperthomas-wiki.readthedocs.io/en/latest/WIKI_FAQ/09_vscode/vscode_debug.html)。

配置好后，按 F5 即可开始调试。

## VSCode 同时启动 GDB Server 和 GDB Client

上述实验中 GDB Server 都需要一个单独的命令行去开启，终究是有一些麻烦，有没有办法不需要单独开启 GDB Server，在 VSCode 里按 F5 直接同时启动 GDB Server 和 GDB Client，并顺利调试呢？

VSCode 插件 gnu-debugger (1.0.4) 提供了解决方案，其在 launch.json 配置文件添加了一些特殊配置项，用于开启 GDB Server 。

```json
{
  // Visual Studio Code launch.json for XMC 2Go development board 
  "version": "0.2.0",
  "configurations": [
    {
      "type": "gnu-debugger",
      "request": "launch",
      "name": "GNU debugger",
      "program": "${workspaceFolder}/build.nosync/firmware.elf",
      "toolchain": "${config:arm-none-eabi.bin}",
      "client": "arm-none-eabi-gdb",
      "server": "JLinkGDBServer",
      "serverArgs": [
        "-device", "nrf52840_xxAA",
        "-if", "SWD",
        "-speed", "4000"
      ],
      "serverHost": "localhost",
      "serverPort": 2331,
      "customVariables": [
        "port0",
        "port1",
        "port2",
      ],
      "autoRun": false,
      "debugOutput": false,
      "preLaunchTask": "build firmware"
    }
  ]
}
```

其中与 GDB Server 相关的有：

```json
      "server": "JLinkGDBServer",
      "serverArgs": [
        "-device", "nrf52840_xxAA",
        "-if", "SWD",
        "-speed", "4000"
      ],
	  "serverHost": "localhost",
      "serverPort": 2331,
```

- "server"：JLinkGDBServer （其他调试工具提供的 GDBServer 也可）的路径。
- "serverArgs"：运行上述工具时附加的命令行参数。
- "serverHost"：GDB Server 的 IP 地址，嵌入式调试一般为本机地址，localhost 或 127.0.0.1 。
- "serverPort"：GDB Server 进程绑定的端口。

可以看出上述这些配置项就等同于命令：

```shell
JLinkGDBServer -device nrf52840_xxAA -if SWD -speed 4000
```

因此自动开启了一个 GDB Server 。

再来看 GDB Client 相关内容：

```json
      "program": "${workspaceFolder}/build.nosync/firmware.elf",
      "toolchain": "${config:arm-none-eabi.bin}",
      "client": "arm-none-eabi-gdb",
```

- "toolchain"：gdb 调试器的路径。
- "client"：gdb 调试器的具体名称，这里为 "arm-none-eabi-gdb" 。
- "program"：想要调试的可执行文件的路径。

借助该插件，并适当地配置 launch.json 文件，我们就能够实现按 F5 一键调试的功能。

----

通过上述学习过程可以发现，只要掌握了最基础的 GDB 调试知识（命令行启动 Server 和 Client），就可以根据需求灵活地集成 Client 到 VSCode，或一些 Eclipse IDE，等等。算是一通百通了。

参考：

- [Configure VS Code's debugging behavior](https://code.visualstudio.com/docs/cpp/launch-json-reference#_configure-vs-codes-debugging-behavior)
- [J-Link Commander](https://wiki.segger.com/J-Link_Commander)
- [GNU source level debugger for Visual Studio Code](https://marketplace.visualstudio.com/items?itemName=metalcode-eu.gnu-debugger)

