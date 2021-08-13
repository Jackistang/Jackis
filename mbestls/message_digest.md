# Message Digest (MD)

**消息摘要**（Message Digest），有时也被称为**单向散列函数**，用于计算消息的“指纹”。

消息摘要的输入是**任意长度**的比特序列，而其输出是固定比特的值。



mbedtls 里使用 MD 的方式：

mbedtls 里用 `mbedtls_md_context_t` 这个结构体代表 MD 的内部状态，我们将其当做透明对象使用即可，`mbedtls_md_info_t` 包含我们选用的消息摘要算法，例如 MD5, SHA1 这种。

可以使用 `mbedtls_md_list()` 函数获取所有的 MD 算法，用 `mbedtls_md_info_from_string()` 可以根据 MD 算法的名称获取 `mbedtls_md_info_t` 对象，而用 `mbedtls_md_info_from_type()` 则是根据 enum 值来获取对象。

一般进行消息摘要计算的流程如下：

使用 `mbedtls_md_init()` 初始化 MD 内部状态，利用 `mbedtls_md_info_from_type()` 获取 MD 算法对象，然后用 `mbedtls_md_setup()` 函数将 MD 内部状态绑定一个 MD 算法，这样整个 MD 算法就初始化完毕了，再调用 `mbedtls_md_starts()` 开启计算，多次调用 `mbedtls_md_update()` 传入字节序列，当所有数据传输完毕后，调用 `mbedtls_md_finish()` 将 MD 的计算值。

