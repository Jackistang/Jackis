输入 `JLinkExe` 识别 SEGGER J-Link 驱动，

输入 `connect` ，再输入 `NRF52840_XXAA` 连接 nRF52840DK，

选择 `S` SWO 接口，

再需要选择接口速度，直接回车，

连接成功后，输入 `erase` 擦除芯片，可以选择擦除地址范围。

输入 `loadbin /home/tangjia/Jackistang/rt-thread/bsp/nrf5x/nrf52840/rtthread.bin 0x0` 下载程序到 nRF52840DK 里，其中 0x0 是指下载起始地址。

下载完成后输入 `q` 退出 JLink 连接，即可看见 LED1 已经在闪烁了。





[GDB/MI](https://sourceware.org/gdb/current/onlinedocs/gdb/GDB_002fMI.html#GDB_002fMI) 是 GDB 为 Eclipse 等 IDE 留出的 MI 文本接口。

输入 `JLinkGDBServer -device nrf52840_xxAA -if SWD -speed 4000` 启动 JLink 调试器提供的 GDB Server 。

- -device 选择芯片类型
- -if 选择调试接口（Interface）
- -speed 选择速度

GDB Server 默认的端口为 2331 。

再另开一个窗口，输入 `/opt/gcc-arm-none-eabi-6_2-2016q4/bin/arm-none-eabi-gdb rt-thread.elf` 开启 GDB Client，

然后在输入 `target remote localhost:2331`，GDB Client 连接到本地的 2331 端口，与 GDB Server 成功建立连接。



## GDB Server 相关

在 launch.json 文件里的 `configurations` JSON 数组里的一个对象中，

```json
{
    "server": "JLinkGDBServer",
    "serverArgs": [
        "-device", "nrf52840_xxAA",
        "-if", "SWD",
        "-speed", "4000"
    ],
    "serverPort": 2331,
}
```

`server` 就是 GDB Server 的路径，而 `serverArgs` 是提供的命令行参数，上述配置项相当于 VSCode 在启动 GDB 调试时，会开启一个命令行，输入 `JLinkGDBServer -device nrf52840_xxAA -if SWD -speed 4000` 启动一个 GDB Server，与我们之前在命令行里手动输入一致。

`serverPort` 就是 GDB Server 进程绑定的端口，默认为 2331 。

## GDB Client 相关







- `program` 是 debug 的可执行文件的路径
- `request` 明确 "launch" 或 "attach" 属性 ？？？
- `setupCommands` JSON 数组里存储了一系列的 GDB 命令，在启动 GDB 后会自动在命令行里输入这些命令。
- `miDebuggerPath` 是 gdb 调试器的路径
- `miDebuggerServerAddress` 是调试远端设备时使用的，形式为 "ip address:port"，例如 "localhost:1234" 。





1. 命令行启动 GDB Server，GDB Client
2. 命令行启动 GDB Server，VSCode 启动 GDB Client
3. VSCode 同时启动 GDB Server，GDB Client





建议：

1. `.rtthread_jlink_download` 文件里的下载地址不要固定，其他芯片的起始地址不一定是 0x08000000，例如 nRF52840 就是 0x0 。
2. Linux 平台下的 JLink 下载工具名称为 JLinkExe，而插件仍会使用 Windows 平台里的名称 "JLink.exe"，这最好能够根据平台的不同适配一下，否则下载时会出现下述错误：

```shell
tangjia@FA001334:~/Jackistang/rt-thread/bsp/nrf5x/nrf52840$ "/opt/SEGGER/JLink/JLink.exe" -device nrf52840_xxAA -CommandFile "/home/tangjia/Jackistang/rt-thread/bsp/nrf5x/nrf52840/.rtthread_jlink_download"
bash: /opt/SEGGER/JLink/JLink.exe: 没有那个文件或目录
```

3. `RTT_Studio.Debuger.JLink: Device` 这个配置项是用来配置芯片名称的，例如 nrf52840_xxAA，这个名称应该不同的调试器有不同的名称，例如 JLink 和 ST-Link 的设备名称可能都不同，但是用户可以自己找到，是否可以做成图形化选择呢，可以方便使用。



https://code.visualstudio.com/docs/cpp/launch-json-reference#_configure-vs-codes-debugging-behavior

https://wiki.segger.com/J-Link_Commander

https://marketplace.visualstudio.com/items?itemName=metalcode-eu.gnu-debugger

https://github.com/metalcode-eu/gnu-debugger/blob/master/package.json