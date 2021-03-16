# 树莓派Bluez获取btsnoop_hci.log

首先通过 Xshell 远程 ssh 登录树莓派，检查树莓派上有没有 `hcidump` 工具，若没有需要下载一个。

输入 `sudo hcidump -Xt -w btsnoop_hci.log` 即开启了一个监听窗口，再用 ssh 另开一个窗口连接树莓派，利用 `bluetoothctl` 进行常规的扫描连接操作即可。

操作完成后，在开启 hcidump 的窗口内按 `Ctrl + C` 退出，即可在当前路径下看见 `btsnoop_hci.log` 文件了。

再利用 Xtfp 导出文件到 Windows 系统，即可利用 fontline 查看 btsnoop 了。

----

参考：

- [Capturing Bluetooth Host Controller Interface (HCI) Logs](https://www.ti.com/lit/an/swpa234/swpa234.pdf?ts=1615776533744&ref_url=https%253A%252F%252Fwww.google.com%252F)