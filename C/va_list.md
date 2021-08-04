# va_list

va_list 用于 C 语言实现函数的可变参数数量，例如我们常用的 printf() 函数内部就使用了它。

```C
extern int printf (const char * __format, ...);
```

在 `__format` 参数之后的 `...` 代表可变数量的参数。

va_list 还需与一些函数配合使用：

```C
void va_start( va_list ap, parm_n );
T va_arg( va_list ap, T );
void va_end( va_list ap );
```

使用流程一般是定义一个 va_list 变量，然后调用 `va_start()` 函数开始解析，再利用 `va_arg()` 获取函数可变数量的参数，最后调用 `va_end()` 停止解析。一个 demo 如下：

```C
#include <stdio.h>
#include <stdarg.h>
 
int add_nums(int count, ...) 
{
    int result = 0;
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; ++i) {
        result += va_arg(args, int);
    }
    va_end(args);
    return result;
}
 
int main() 
{
    printf("%d\n", add_nums(4, 25, 25, 50, 50) );
    printf("%d\n", add_nums(4, 25, 25, 50) );
    return 0;
}
```

`va_start()` 共有两个参数，第一个参数就是之前定义的 va_list 类型变量 `ap`，我们将它当成一个透明的对象使用即可，第二个参数 `param_n` 简单理解就是**第一个可变参数的前一个参数，也就是 `...` 之前的那个参数变量名**，但这个参数有更深刻的意义，由于函数的参数数量是可变的，如何确定函数调用者到底传了多少个参数呢？`param_n` 就是用来确定函数可变参数的数量，`param_n` 可以直接代表后续参数的数量，例如上述的 `count`，也可以类似 printf() 函数，用一个 format 字段来确定后续参数数量，例如 `"%d%d%s"` 表明后续有 3 个可变参数。

```C
void va_start( va_list ap, parm_n );
```

为了获取具体的可变参数值，就需要使用 `va_arg()` 函数了，其第一个参数 `ap` 仍是 va_list 透明对象，第二个参数 `T` 是类型，并且其决定了该函数返回值的类型，`T` 可以是 int, unsigned int, char, void * 等等。

```C
T va_arg( va_list ap, T );
```

