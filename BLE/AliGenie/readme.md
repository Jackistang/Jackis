# AliGenie BLE Mesh Provisioning 整个过程

结合 Mesh Spec 1.0.1 和 AliGenie 5.0 版本，可知天猫精灵在和设备进行 Provisioning 过程中，最终的路线图如下：

![](https://i.loli.net/2021/08/01/z2T9YuXCcWZw8js.png)

## Beacon

在 Provisioning 的开始阶段，需要由 Device 发送 Unprovisioned Beacon 广播包，Mesh Beacon 的广播包格式如下：

![](https://i.loli.net/2021/08/01/KOXgFLC8ApNJGI4.png)

其中 `Type` 字段是  «Mesh Beacon» ，而 `Beacon Type` 则有两个值可以选，

![](https://i.loli.net/2021/08/01/NF2wXf7ZyLDGoQB.png)

这里为 0x00 代表 Unprovisioned Device Beacon 。

而对于 Unprovisioned Beacon 类型的 `Beacon Data`，Spec 规定了其格式为：

![](https://i.loli.net/2021/08/01/i9FgdXYQymIMjW8.png)

第 0 个字节的 `0x00` 代表上述的 Unprovisioned Beacon ，`Device UUID` 的 16 个字节由厂家定义，AliGenie 将其定义为：

| Field       | Size（Octets） | Notes                                                        |
| ----------- | :------------- | ------------------------------------------------------------ |
| CID         | 2              | 公司ID，设置为0x01A8：Taobao                                 |
| PID         | 1              | Bit0-3 蓝牙广播包版本号，目前是0x01<br />bit4为1：一机一密<br />bit5为1：支持OTA<br />bit6~7：  00：BLE4.0<br />                  01：BLE4.2<br />                  10：BLE5.0<br />                  11：BLE5.0以上 |
| ProductID   | 4              | 阿里巴巴平台颁发，一型一号                                   |
| MAC地址     | 6              | 阿里巴巴平台颁发，一机一号                                   |
| FeatureFlag | 1              | bit7-1：uuid版本号，目前版本为1<br />bit0    0：处于未配网广播状态<br />            1：处于静默广播状态 |
| RFU         | 2              | 0x00 Reserved for future use                                 |

`OOB Infomation` 为全 0，代表不使用任何 OOB 手段传输公钥（Public Key），而是直接通过 BLE 广播包传输公钥（之后会有介绍）。 

![](https://i.loli.net/2021/08/01/RPwlXyUIphVrdAk.png)

最后，Beacon 里不包含 `URI Hash` 。

对于 Unprovisioned Beacon，AliGenie 给出了两种状态

- 未配网状态：`Device UUID` 里的 FeatureFlag 的 bit0 为 0，广播时长 40ms，广播间隔 100ms，广播持续时间默认 10 分钟。
- 静默广播状态：`Device UUID` 里的 FeatureFlag 的 bit0 为 1，广播时长 120ms，广播间隔 60s，不响应 Provionser 的配网消息。

当 Device 处于未配网状态，超时后也未被配网时，自动进入静默广播状态。

## Invitation

天猫精灵收到 Unprovisioned Beacon 时，会开始进行 `1: Invitation & Capabilities` 流程：

![](https://i.loli.net/2021/08/01/2ZlAiPFRJN8SaUX.png)

天猫精灵首先会向设备发送 Provisioning Invite PDU，其中包含参数 `Attention Duration`，该参数用于 Device 进行一些操作（例如灯闪烁，蜂鸣器响）来吸引人注意的持续时间。

![](https://i.loli.net/2021/08/01/978Q3KajZSJ2DHy.png)

之后 Device 返回 Provisioning Capabilities PDU，根据 AliGenie 5.0 规定，我们需要将 `Static OOB Type` 位置 1，而 `Public Key Type`，`Output OOB` 和 `Input OOB` 都不需要。

![](https://i.loli.net/2021/08/01/HGOek1T4Rc8osbt.png)

## Exchange public keys

在本阶段 Provisioner 和 Device 通过**椭圆曲线 diffie-hellman 密钥交换算法**（ECDH）方式共同协商出了一个密钥（Session Key），用于后续信息的加密传输。

关于 ECDH 的详细介绍可以参考 [BLE安全机制从入门到放弃](https://blog.csdn.net/weixin_42583147/article/details/93175701)，下面仅简单介绍一些。

ECDH 利用了椭圆曲线中的数学问题来解决密钥传输问题，公式可抽象为：
$$
Y = x * G
$$
该公式已知椭圆曲线上的点 $Y$、基点 $G$ 的时候，很难求出 $x$ 。其中算术符号 $*$ 表示的不是普通的乘法，而是一种在椭圆曲线上的特殊算法。

以 Bob 和 Alice 为例，看其如何在有 Eve 监听的情况下协商出一个密钥。

 ![](https://i.loli.net/2021/08/01/oMF47ygZU9ulnsP.png)

参考上图，Bob 利用公式 $pb = sb * G$ 生成了公钥 $pb$，根据上述信息可知，已知 $pb$ 和 $G$ 很难求出 $pa$ 。Alice 也利用相同的方式生成了公钥 $pa$ 。之后他们俩相互交换公钥 $pa$, $pb$ ，然后就可以计算出一个双方都拥有的密钥 $DHey$ ：
$$
\begin{align}
DHkey &= sb * pa \\
      &= sb * sa * G \\

DHkey &= sa * pb \\
	  &= sa *pb * G 

\end{align}
$$
上述运算符 $*$ 具有交换律，因此 Alice 和 Bob 计算的 $DHkey$ 值一致，可以用于后续加密计算。

而一直监听的 Eve 只拥有数据 $G$, $pa$, $pb$，根据上述公式特性，很难根据 $G$, $pa$, 或 $pb$ 计算出一个私钥，因此无法得到上述的 $DHkey$，从而 Bob 和 Alice 成功在有窃听风险的环境中协商出了一个安全的密钥。

再来看看 Provisioning 过程中的 Exchange public keys 阶段， 

![](https://i.loli.net/2021/08/01/u2YLvFtbmVWxJQc.png)

天猫精灵向设备发送 Provisioning Start PDU，其携带的参数里 `Authentication Method` 表明选择 `Static OOB` 认证方式。之后双方随机生成一个私钥，再根据 ECDH 的公式计算出公钥，双方交换公钥，再次利用 ECDH 公式即可协商出一个密钥（ECDHKey）用于后续信息加密传输。当然，在这一过程中我们走的是 `2a：Start without OOB Public Key` 这条路径，如下图：

![](https://i.loli.net/2021/08/01/iyLREAZHkoJT5Wb.png)

其实还有 `2b：Start using OOB Public Key` 这条路径，此时 Provisioner 通过 OOB 的方式获取 Device 的公钥，除此之外没有其他区别，流程如下图：

![](https://i.loli.net/2021/08/01/4UdxOm3PXBErpbt.png)

## Authentication

在认证阶段，通信双方都需要验证对方的身份，整体流程如下：

![](https://i.loli.net/2021/08/01/PVXRgDQOZSehki3.png)

首先通信双方通过 Provisioning Confirmation 交换一个 128-bit 的 `Confirmation` 值：

![](https://i.loli.net/2021/08/01/QwIPTLG8KbNEVZW.png)

Provisioner 的 `Confirmation` 值的计算公式如下：
$$
ConfirmationProvisioner = AES_-CMAC_{ConfirmationKey}(RandomProvisioner || AuthValue)
$$
其中输入参数有一个 `RandomProvisioner` 和一个 `AuthValue` 值，Random 值是随机生成的，而 `AuthValue` 则是由厂商定义，AliGenie 将其定义为：
$$
AuthValue = SHA256(Product ID,MAC,Secret)
$$
将 ProductID，MAC，Secret 三元组通过字符串用**英文逗号**连接，然后进行 SHA256 摘要计算，取前16字节。注：这里用于计算 SHA256 的**英文字母全部为小写**。SHA256 类似一个 Hash 散列函数，输入是一个字符串，输出是一个 256-bit 的散列值，AliGenie 取前 16 个字节。

![](https://i.loli.net/2021/08/01/X1gode5ijk3YaHJ.png)

Device 端也是类似的操作。

当双方都计算得到 `Confirmation` 值后，通过 Provisioning Confirmation PDU 交换该值，之后 Provisioner 通过 Provisioning Random PDU 发送 `RandomProvisioner` 值，这样 Device 就可以利用自身的 `AuthValue` 和收到的 `RandomProvisioenr` 来验证收到的 `Confirmation` 是否正确，从而达到认证的目的。

## Distribution of provisioning data

认证完成后，天猫精灵就准备下放数据了，利用之前的 ECDHKey 生成一个 session key，然后基于这个 session key 将数据（network key, NetKey Index, Key Refresh Flag, IV Update Flag, IV index, unicast value）加密，通过 Provisioning Data PDU 发送给 Device，若 Device 端成功接收，返回 Provisioning Complete，流程如下：

![](https://i.loli.net/2021/08/01/tmSHARWNsuLBhpP.png)

至此，Provisioning 过程结束，接下来就是 Configuration 过程了。

