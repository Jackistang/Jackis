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