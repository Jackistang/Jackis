# ARRAY_SIZE 宏

`ARRAY_SIZE(array)` 宏用来获取数组 array 的大小，由于数组在函数调用时会退化成指针，这可能会导致某些隐藏的问题，因此 ARRAY_SIZE 宏利用 GNU C 扩展使用了一种方式检查数组类型的方法。如果传入的 array 是数组类型，就正常获取数组的大小，如果不是则会在编译期报错。

首先我们来看一些辅助宏的实现。

## ZERO_OR_COMPILE_ERROR

可以根据名称推断 `ZERO_OR_COMPILE_ERROR(cond)` 的结果要么返回 0，要么产生编译错误。

- `cond` 为 true（非 0 值）：结果为 0 。
- `cond` 为 false（0）：产生编译错误。

实现原理也很简单，参见下述代码：

```C
/* Evaluates to 0 if cond is true-ish; compile error otherwise */
#ifndef ZERO_OR_COMPILE_ERROR
#define ZERO_OR_COMPILE_ERROR(cond) ((int) sizeof(char[1 - 2 * !(cond)]) - 1)
#endif
```

如果传入的是 true（非 0 值），`1 - 2 * !(cond)` 就是 1，这样宏扩展结果为 `((int) sizeof(char[1]) - 1)` ，最终结果为 0 。

如果传入的是 false（0），`1 - 2 * !(cond)` 就是 -1，这样宏扩展结果为 `((int) sizeof(char[-1]) - 1)`，`char[-1]` 不合法，就会产生编译错误。

## IS_ARRAY

`IS_ARRYA(array)` 宏用于判断传入参数是否为数组类型，若是则返回 0，若不是则产生编译错误。在讲解实现之前我们需要了解一下 GNU C 里面的 `typeof` 和 `__builtin_types_compatible_p` 。

### typeof

`typeof` 用于获取表达式的类型或者类型名的类型。例如：

```C
typeof (x[0])
```

此处 x 是一个数组，里面存储了 int 型数据，那么上述结果就是 int 类型。

又例如

```C
typeof (int *)
```

这次我们直接用类型名作为参数，得到的结果为 int * 类型。

### __builtin_types_compatible_p

` int __builtin_types_compatible_p (type1, type2)` 是 GNU C 定义的内联函数，用于比较 `type1` 与 `type2` 类型是否兼容（compatible），若类型兼容则返回 1，否则返回 0 。例如：

```C
int array[6]
__builtin_types_compatible_p(__typeof__(array), __typeof__(int[6]))      == 1
__builtin_types_compatible_p(__typeof__(array[0]), __typeof__(int))      == 1
```

### IS_ARRAY 实现

`IS_ARRAY` 实现代码如下：

```C
/* Evaluates to 0 if array is an array; compile error if not array (e.g.
 * pointer)
 */
#ifndef IS_ARRAY
#define IS_ARRAY(array) \
        ZERO_OR_COMPILE_ERROR( \
        !__builtin_types_compatible_p(__typeof__(array), \
            __typeof__(&(array)[0])))
#endif
```

首先获取 `__typeof__(array)` 和 `__typeof__(&(array)[0])` 的类型，然后利用 `__builtin_types_compatible_p` 判断这两个类型是否兼容，进行 `!` 运算后传入 `ZERO_OR_COMPILE_ERROR`。

这个实现很巧妙，我通过实验来验证了效果，如下所示：

```C
int array[6]
__builtin_types_compatible_p(__typeof__(array), __typeof__(int[6]))      == 1
__builtin_types_compatible_p(__typeof__(&array[0]), __typeof__(int *))   == 1
__builtin_types_compatible_p(__typeof__(array), __typeof__(&(array)[0])) == 0

char *arr
__builtin_types_compatible_p(__typeof__(arr), __typeof__(int *))         == 1
__builtin_types_compatible_p(__typeof__(&(arr)[0]), __typeof__(int *))   == 1
__builtin_types_compatible_p(__typeof__(arr), __typeof__(&(arr)[0]))     == 1
```

由于数组可能退化成指针，这里就只考虑了数组类型和指针类型（其他类型会直接报编译错误）。

可以看到 `IS_ARRAY(array)` 传入的参数为数组类型，那么 `array` 的类型是 `int[6]`，`&array[0]` 的类型是 `int *` ，两者是不兼容的，`__builtin_types_compatible_p` 返回 0，经过 `!` 运算后变成 1 传入 `ZERO_OR_COMPILE_ERROR` ，最终得到结果 0 。

若传入的参数为指针类型，那么 `array` 和 `&array[0]` 的类型都是 `int *`，两者是兼容的，`__builtin_types_compatible_p` 返回 1，经过 `!` 运算后变成 0 传入 `ZERO_OR_COMPILE_ERROR` ，最终产生编译错误。这里需要注意 `&array[0]` 类型是 `int *` 的理解，回想一下我们在给函数传递数组时，一般是下面这种形式：

```C
void demo(int *array, int len)
{
    int a;
    if (len > 0) 	a = array[0];
}
```

 我们习惯的用法还是将 array 当作数组使用，利用 `[]` 来访问元素。可以这么用，但要意识到这时 array 实际上是个指针，没有保存数组相关的信息。很显然，`&array[0]` 的类型就是 `int *` 。

## ARRAY_SIZE 实现

搞懂了上述惯用法后，`ARRAY_SIZE` 实现就变得很简单了。如下所示：

```C
/* Evaluates to number of elements in an array; compile error if not
 * an array (e.g. pointer)
 */
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) \
        ((unsigned long) (IS_ARRAY(array) + \
            (sizeof(array) / sizeof((array)[0]))))
#endif
```

核心代码是 `IS_ARRAY(array) + (sizeof(array) / sizeof((array)[0]))` 。

`IS_ARRAY(array)` 用于进行类型检查，若不是数组则产生编译错误，若是数组返回结果为 0，对最终计算结果无影响。

`sizeof(array) / sizeof((array)[0])` 用于获取实际的数组大小。由于 `array` 是一个数组，那么 `sizeof(array)` 的结果是整个数组所需要的字节数，而 `sizeof((array)[0])` 的结果是数组中一个元素所需要的字节数，两者相除即可得到数组大小。

`ARRAY_SIZE(array)` 的介绍到此结束，这其实算是一种惯用法，为常规的获取数组大小提供了一种类型检查的机制，类似的惯用法还有很多，下次碰到了在介绍。

---

参考：

[Using the GNU Compiler Collection (GCC) - 6.59 Other Built-in Functions Provided by GCC](https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html)

[Using the GNU Compiler Collection (GCC) - 6.7 Referring to a Type with `typeof`](https://gcc.gnu.org/onlinedocs/gcc/Typeof.html#Typeof)

