# Rule 02. Declarations and Initialization



## DCL30-C. Declare objects with appropriate storage durations

以合理的**存储期（storage durations）**声明对象。

这条规则的核心是要通过对象的**存储期**来明确其**生命周期（lifetime）**，不能出现访问生命周期外对象的情况，尤其是在使用指针的时候。

一个错误示例如下，`squirrel_away()` 函数退出时，`local` 数组的生命周期结束，此时 `ptr` 访问了声明周期外的变量，是未定义的行为。

```C
void squirrel_away(char **ptr_param) {
  char local[10];
  /* Initialize array */
  *ptr_param = local;
}
 
void rodent(void) {
  char *ptr;
  squirrel_away(&ptr);
  /* ptr is live but invalid here */
}
```

改进方式如下，`local` 具有**静态（static）**存储期，`squirrel_away()` 函数返回时，`ptr` 指向的内容仍是有效的。

```C

char local[10];
  
void squirrel_away(char **ptr_param) {
  /* Initialize array */
  *ptr_param = local;
}
 
void rodent(void) {
  char *ptr;
  squirrel_away(&ptr);
  /* ptr is valid in this scope */
}
```

对象的存储期可分为：

- static - 静态存储期
- thread
- automatic - 自动存储期
- allocated

static 就是代码里用 `static` 修饰的对象，automatic 就是函数内不使用 `static` 声明的对象，allocated 就是用 `malloc` 相关函数分配的对象，而 thread 比较特殊，是用 `__thread` 修饰的对象，表明该对象是该线程专属的。

---

参考：

- [Thread-local storage - WIKIPEDIA](https://en.wikipedia.org/wiki/Thread-local_storage#Windows_implementation)
- [Thread-Local Storage - GCC Documents](https://gcc.gnu.org/onlinedocs/gcc/Thread-Local.html)

## DCL31-C. Declare identifiers before using them

使用标识符前必须**显式声明**他们。

这条规则的核心是使用变量、函数时必须显式地声明他们。C90 标准允许变量和函数的**隐式声明**，但这在新的 C11 标准中被废弃了，并且新代码也不推荐使用隐式声明，以下有一些例子：

**Noncompliant Code Example (Implicit int)**

变量声明时如果缺乏类型定义，编译器会尝试隐式声明为 int 类型，例如下述的 `foo` 。

```C
extern foo;
```

但 `foo` 可能是其他类型的变量，最终导致错误。

**Noncompliant Code Example (Implicit Function Declaration)**

当调用一个函数，而该函数并未声明，则 C90 标准会**隐式声明**一个标识符 `extern int identifier();`，其函数可以接收任意个数任意类型的参数，且返回值为 int 类型。例如下述例子：

```C
#include <stddef.h>
/* #include <stdlib.h> is missing */
  
int main(void) {
  for (size_t i = 0; i < 100; ++i) {
    /* int malloc() assumed */
    char *ptr = (char *)malloc(0x10000000);
    *ptr = 'a';
  }
  return 0;
}
```

`malloc()` 函数的头文件 stdlib.h 并未 include，因此 C90 编译器隐式声明了 `int malloc()`，如果系统的 int 是 32bit，而指针是 64bit，这就会导致 malloc 返回的 64bit 数据被截断为  32bit，最终导致错误。

**Noncompliant Code Example (Implicit Return Type)**

当一个函数并未显式声明其返回值类型时，如果函数返回一个整数，C90 编译器隐式声明其返回值为 int 类型。下述例子中 `foo()` 函数的返回值类型被隐式声明为 int 类型，其返回的 `UINT_MAX` 被错误的转换成了 -1 。

```C
#include <limits.h>
#include <stdio.h>
  
foo(void) {
  return UINT_MAX;
}
 
int main(void) {
  long long int c = foo();
  printf("%lld\n", c);
  return 0;
}
```

## DCL36-C. Do not declare an identifier with conflicting linkage classifications

不要声明会导致**链接冲突**的标识符。

标识符的链接属性可分为三类：

- 外部链接（**External linkage**）：该标识符在整个程序中（所有编译单元和库）代表**同一**对象或函数。用 **extern 声明的标识符**或**具有文件作用域范围且未用 static 修饰的标识符**都具有外部链接属性。
- 内部链接（**Internal linkage**）：该标识符在一个翻译单元（单个文件）内代表**同一**对象或函数。用 static 修饰的标识符具有内部链接属性。
- 无链接（**No linkage**）：可以在文件的其他地方声明名称相同的标识符。

多次声明具有不同连接属性的标识符，有时会有未定义的行为，应尽可能避免。

下图中显示了同时声明两个不同链接属性的标识符的最终结果，列为第一次声明，行为第二次声明。

![](https://i.loli.net/2021/07/15/aXlIjfE4hoOMSUi.png)

一个简单例子如下，`i2` 和 `i5` 都会产生未定义的行为。 

```C
int i1 = 10;         /* Definition, external linkage */
static int i2 = 20;  /* Definition, internal linkage */
extern int i3 = 30;  /* Definition, external linkage */
int i4;              /* Tentative definition, external linkage */
static int i5;       /* Tentative definition, internal linkage */
 
int i1;  /* Valid tentative definition */
int i2;  /* Undefined, linkage disagreement with previous */
int i3;  /* Valid tentative definition */
int i4;  /* Valid tentative definition */
int i5;  /* Undefined, linkage disagreement with previous */
 
int main(void) {
  /* ... */
  return 0;
}
```

