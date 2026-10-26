# 第 4 章 · 函数、重载与 lambda

> **卷一 · 语言**　|　前置知识：第 1–3 章　|　建议用时：45 分钟

> 函数是 C++ 的第一个抽象：它把一段行为装进一个名字里，然后允许你用不同的参数反复调用它。

控制流把语句组织成路径，函数把路径组织成可复用的单元。这一章讲函数的声明与定义、参数传递、返回值、重载、默认实参、递归、函数指针，以及 C++11 引入的 lambda。

C++ 的函数比 C 多了几层：重载、引用参数、默认实参、成员函数、lambda、模板函数。本章先把前几层讲清楚，模板函数留到第 9 章。

## 4.1 本章语法地图

| 语法 | 本章出现的位置 |
|------|----------------|
| 函数声明 | `int add(int, int);` |
| 函数定义 | `int add(int left, int right) { ... }` |
| 参数传递 | 按值、按引用、按 `const` 引用 |
| 默认实参 | `int power(int base, int exponent = 2);` |
| 返回引用 | `int &static_counter();` |
| 重载 | 同名、不同参数列表 |
| 递归 | `factorial(n - 1)` |
| `constexpr` 函数 | `constexpr int factorial(int n);` |
| 函数指针 | `int (*operation)(int, int);` |
| lambda | `[capture](参数) { ... }` |
| `std::function` | `std::function<int(int, int)>` |
| `main` 参数 | `int main(int argc, char *argv[])` |

## 4.2 声明、定义与调用

函数声明告诉编译器“有这个函数，参数和返回类型是什么”；函数定义给出函数体：

```cpp
int add(int left, int right);   // 声明

int add(int left, int right)    // 定义
{
    return left + right;
}
```

声明可以出现很多次，定义只能出现一次（`inline` 函数和模板有例外）。如果只声明不定义，编译可以通过，链接会失败：

```text
undefined reference to `add(int, int)'
```

这是“编译错误”和“链接错误”的经典分界。第 8 章会从翻译单元和 ODR 的角度再讲一遍。

函数声明里的参数名可以省略：

```cpp
int add(int, int);
```

定义里必须有参数名（除非你不需要用它）。声明和定义的参数类型必须一致；参数名可以不同。

### `inline` 与 `constexpr`

`inline` 最初是给编译器一个“可以内联展开”的建议，现代编译器基本自己决定是否内联。它在语义上更重要的作用是：**允许同一个函数定义出现在多个翻译单元里**。所以头文件里的函数定义通常要写 `inline`，否则多个 `.cpp` 包含它就会重复定义。

`constexpr` 函数隐含 `inline`。它要求函数在给定常量表达式参数时能在编译期求值；运行时也可以调用：

```cpp
constexpr int square(int value)
{
    return value * value;
}

constexpr int a = square(5);   // 编译期
int b = square(runtime_value); // 运行期也可以
```

## 4.3 参数传递

### 按值传递

```cpp
void by_value(int value)
{
    value = 999;   // 只改副本
}
```

按值传递会复制实参。对内置类型，复制的代价很小；对 `std::string`、`std::vector` 或自定义类，复制可能很贵。按值传递的优点是函数拿到一份独立的数据，不会意外修改调用者的对象。

### 按引用传递

```cpp
void by_reference(int &value)
{
    value = 999;   // 修改调用者的对象
}
```

引用参数不复制，函数直接操作实参。它适合：

- 需要修改实参；
- 对象很大，复制代价高；
- 需要保持多态（引用绑定到基类，虚函数仍然生效）。

非 const 左值引用不能绑定到临时对象：

```cpp
void f(int &);
f(42);          // 错误：42 是纯右值
```

### 按 const 引用传递

```cpp
void by_const_reference(const int &value)
{
    // 不能修改 value
}
```

`const T &` 既能绑定左值，也能绑定临时对象（生命期延长到函数调用结束），而且不复制。它是“只读大对象”的默认选择：

```cpp
void print(const std::string &text);
```

对很小的内置类型（`int`、`double`、指针），按值通常更简单，也可能更快；不要机械地把所有参数都写成 `const &`。

### 指针参数

指针参数和引用参数都能避免复制，但语义不同：

```cpp
void maybe_update(int *value)
{
    if (value != nullptr) {
        *value = 42;
    }
}
```

指针可以是 `nullptr`，可以重新指向，也可以做算术；引用则保证绑定到一个对象。需要表达“参数可选”时，指针或 `std::optional` 更合适；需要表达“一定有一个对象”时，引用更合适。

### 数组参数

数组不能按值传给函数。数组名会退化为指向首元素的指针：

```cpp
void print(int values[10]);   // 实际等价于 int *values
void print(int *values);
void print(int values[]);
```

三种写法在参数列表里完全等价，数组长度信息丢失。要保留长度，可以：

- 额外传一个 `std::size_t size`；
- 用 `std::array<T, N>` 的引用：`template <std::size_t N> void print(const std::array<int, N> &);`
- 用 `std::vector<int>` 或 `std::span`（C++20）。

第 5 章会展开。

### 默认实参

```cpp
int power(int base, int exponent = 2);

int power(int base, int exponent)
{
    // ...
}

power(3);      // exponent = 2
power(3, 4);   // exponent = 4
```

规则：

- 默认实参写在声明里，通常不在定义里重复。
- 一旦某个参数有默认值，它右边的所有参数都必须有默认值。
- 默认实参在调用点求值，可以是表达式，但不能依赖局部变量。
- 虚函数的默认实参在静态类型上确定，可能和动态类型不一致（第 7 章会提）。

默认实参让接口更好用，但也可能让重载决议变复杂。不要用默认实参去“模拟”多个重载，除非语义确实相同。

## 4.4 返回值

### 按值返回

```cpp
int add(int left, int right)
{
    return left + right;
}
```

返回值从函数内部传回调用者。对类类型，按值返回可能涉及拷贝；但现代 C++ 有**返回值优化**（RVO）和移动语义，很多情况下不会真的拷贝。

C++17 起，某些纯右值的复制消除是**强制**的：

```cpp
std::string make()
{
    return std::string("hello");   // C++17 保证不复制
}
```

但命名返回值优化（NRVO）仍然是可选的，不能依赖它一定发生。不要为了“优化”而写出难以理解的代码。

### 返回引用

```cpp
int &static_counter()
{
    static int value = 0;
    return value;
}
```

返回引用可以避免拷贝，也允许调用者修改返回的对象。但必须保证返回的引用在调用者使用时仍然有效：

```cpp
int &bad()
{
    int local = 42;
    return local;   // 错误：返回局部变量的引用，悬垂
}
```

局部变量在函数返回时销毁，返回它的引用是未定义行为。返回静态变量、全局变量、成员变量或传入的引用参数才可能安全。

### 返回指针

返回指针同样有生命期问题：

```cpp
int *bad_pointer()
{
    int local = 42;
    return &local;   // 悬垂指针
}
```

如果指针指向动态分配的对象，调用者还要负责释放；现代 C++ 通常用 `std::unique_ptr` / `std::shared_ptr` 表达所有权，而不是裸指针。

### `[[nodiscard]]`

C++17 的 `[[nodiscard]]` 提醒调用者不要丢弃返回值：

```cpp
[[nodiscard]] int compute();

compute();   // 编译器警告：返回值被丢弃
```

它适合“失败信息不能忽略”的函数，例如返回错误码的函数。它不是强制，只是诊断。

## 4.5 重载

C++ 允许同名函数有不同参数列表：

```cpp
const char *describe(int);
const char *describe(double);
const char *describe(const char *);
const char *describe(const std::string &);
```

调用时，编译器根据实参类型选择最匹配的重载。这就是**重载决议**。

规则很复杂，先记住几个要点：

- 返回类型不参与重载。`int f();` 和 `double f();` 不能共存。
- 参数个数、类型、顺序不同才能重载。
- 顶层 `const` 不区分：`f(int)` 和 `f(const int)` 是同一个函数。
- 引用限定可以区分：`f(int &)` 和 `f(int &&)` 是不同重载。
- 编译器按“精确匹配 → 提升 → 标准转换 → 用户定义转换”的顺序选择。
- 如果多个重载同样好，调用就是**二义**（ambiguous），编译错误。

```cpp
void f(int);
void f(long);

f(42);      // int 精确匹配
f(42L);     // long 精确匹配
f(3.14);    // 二义：double -> int 和 double -> long 都是标准转换
```

### 重载与默认实参

默认实参可能在重载决议之后才填充。如果两个重载在调用点看起来都可行，仍然可能二义：

```cpp
void g(int a);
void g(int a, int b = 0);

g(1);   // 二义
```

不要写这种接口。

### 重载与 `const`

成员函数可以用 `const` 区分：

```cpp
class Buffer {
public:
    int &operator[](std::size_t);
    const int &operator[](std::size_t) const;
};
```

非 const 对象调用非 const 版本，const 对象调用 const 版本。这是第 7 章的内容。

## 4.6 递归

递归是函数直接或间接调用自己：

```cpp
int factorial(int n)
{
    return n <= 1 ? 1 : n * factorial(n - 1);
}
```

递归要有**基准情形**（base case），否则会无限递归，最终栈溢出。每次递归调用都会占用栈空间；深度很大时，即使逻辑正确也可能崩溃。

递归适合树、图、分治等天然递归的结构：

```cpp
int fibonacci(int n)
{
    return n < 2 ? n : fibonacci(n - 1) + fibonacci(n - 2);
}
```

这个实现的时间复杂度是指数级的，因为重复计算了大量子问题。教学示例可以这样写，生产代码要用动态规划或迭代。

`constexpr` 递归可以在编译期求值：

```cpp
constexpr int factorial(int n)
{
    return n <= 1 ? 1 : n * factorial(n - 1);
}

static_assert(factorial(5) == 120);
```

C++14 起，`constexpr` 函数可以包含循环、局部变量和分支；递归不是唯一写法。

## 4.7 作用域与生命期

函数引入几层作用域：

- **块作用域**：函数体、`if` 块、循环体里的名字。
- **函数作用域**：标签（`goto` 的目标）在整个函数内可见。
- **命名空间作用域**：函数外、命名空间内的名字。
- **类作用域**：成员名在类内可见。

```cpp
int value = 1;          // 全局

void demo()
{
    int value = 2;      // 局部，遮蔽全局
    {
        int value = 3;  // 再遮蔽
        std::cout << value; // 3
    }
    std::cout << value; // 2
}
```

名字查找从内向外，找到第一个匹配就停。局部变量可以遮蔽全局变量，这通常是坏主意；`-Wshadow` 会提醒你。

### 静态局部变量

```cpp
int &counter()
{
    static int value = 0;
    return value;
}
```

静态局部变量在第一次执行到声明时初始化，之后一直存在到程序结束。它的初始化是线程安全的（C++11 起），但后续的读写不是；多线程下需要同步。

### 函数对象与 lambda 的生命期

lambda 是一个对象，捕获的变量会成为它的成员：

- 按值捕获：在 lambda 创建时复制一份；
- 按引用捕获：保存引用，lambda 使用时访问原对象。

按引用捕获的 lambda 如果比被捕获对象活得更久，就会悬垂。第 4.8 节会展开。

## 4.8 lambda

lambda 是匿名函数对象：

```cpp
auto add = [](int left, int right) {
    return left + right;
};

std::cout << add(2, 3);   // 5
```

它比普通函数多了**捕获列表**：

```cpp
int offset = 10;
auto add_offset = [offset](int value) {
    return value + offset;
};
```

`[offset]` 按值捕获，lambda 内部有 `offset` 的副本。`[&offset]` 按引用捕获，lambda 内部访问原变量。

### 捕获方式

| 写法 | 含义 |
|------|------|
| `[]` | 不捕获 |
| `[x]` | 按值捕获 `x` |
| `[&x]` | 按引用捕获 `x` |
| `[=]` | 按值捕获所有用到的外部变量（不推荐滥用） |
| `[&]` | 按引用捕获所有用到的外部变量（不推荐滥用） |
| `[this]` | 捕获当前对象的指针（成员函数里） |
| `[x, &y]` | 混合捕获 |
| `[x = expr]` | 初始化捕获，C++14 |

按值捕获的变量默认是 `const` 的副本；如果要在 lambda 里修改它，加 `mutable`：

```cpp
int count = 0;
auto next = [count]() mutable {
    return ++count;
};
```

`mutable` 只让副本可修改，不会影响外部的 `count`。

### 泛型 lambda（C++14）

参数可以写 `auto`：

```cpp
auto twice = [](auto value) {
    return value + value;
};

twice(21);   // int
twice(1.5);  // double
```

编译器会为每种参数类型生成一个 `operator()`，本质上是模板。

### lambda 与函数指针

不捕获任何变量的 lambda 可以转换成函数指针：

```cpp
int (*operation)(int, int) = [](int left, int right) {
    return left + right;
};
```

捕获了变量的 lambda 不能转换成普通函数指针，因为它需要保存状态。

### 返回类型

lambda 的返回类型通常由编译器推导。如果函数体有多条 `return`，所有返回类型必须一致；否则要显式写尾置返回类型：

```cpp
auto choose = [](bool flag) -> int {
    if (flag) {
        return 1;
    }
    return 2.5;   // 会被转换成 int
};
```

### lambda 的生命期陷阱

```cpp
auto make_lambda()
{
    int local = 42;
    return [&local]() { return local; };  // 悬垂！
}
```

返回按引用捕获局部变量的 lambda，调用时访问的是已经销毁的对象，未定义行为。按值捕获 `[local]` 才安全。

## 4.9 函数指针与 `std::function`

函数指针保存函数的地址：

```cpp
int add(int left, int right);
int (*operation)(int, int) = add;
int result = operation(2, 3);
```

函数指针可以作为参数，实现回调：

```cpp
int apply(int (*operation)(int, int), int left, int right)
{
    return operation(left, right);
}
```

`std::function` 能保存任意可调用对象（函数、lambda、函数对象），代价是类型擦除和可能的堆分配：

```cpp
#include <functional>

std::function<int(int, int)> operation = add;
operation = [](int left, int right) { return left * right; };
```

选择建议：

- 编译期已知、不需要保存状态：模板参数或函数指针；
- 需要保存状态、接口需要统一类型：`std::function`；
- 泛型算法：模板参数，避免类型擦除开销。

## 4.10 `main` 的参数

```cpp
int main(int argc, char *argv[])
{
    for (int i = 0; i < argc; ++i) {
        std::cout << argv[i] << '\n';
    }
}
```

- `argc` 是参数个数，至少为 1（程序名本身）。
- `argv` 是参数字符串数组，`argv[0]` 是程序名，`argv[argc]` 是 `nullptr`。
- `argv` 的字符串由运行环境管理，程序不应修改它们的内容。
- 参数需要数字时，用 `std::stoi`、`std::stod` 等转换，并处理异常。

[`args.cpp`](args.cpp) 演示了遍历参数和转换最后一个参数。运行：

```sh
make 04-functions/args
./04-functions/args alpha beta 42
```

## 4.11 代码导览

本章有两个程序：

- [`functions.cpp`](functions.cpp)：函数调用、参数传递、重载、默认实参、递归、函数指针、lambda、`std::function`。
- [`args.cpp`](args.cpp)：`main` 的 `argc` / `argv`，以及字符串到整数的转换。

`functions.cpp` 的输出刻意展示了三件事：

1. 按值传递修改的是副本；
2. 按引用传递修改的是原对象；
3. 按值捕获和按引用捕获的 lambda 看到的值不同。

运行：

```sh
make 04-functions/functions 04-functions/args
./04-functions/functions
./04-functions/args alpha beta 42
```

## 4.12 常见陷阱

| 写法 | 问题 | 建议 |
|------|------|------|
| 返回局部变量的引用/指针 | 悬垂，UB | 返回值、静态对象或动态对象 |
| `int f();` 后不定义就调用 | 链接错误 | 提供定义或使用库 |
| 头文件里定义非 `inline` 函数 | 多个翻译单元重复定义 | 加 `inline` 或移到 `.cpp` |
| 重载只改返回类型 | 不能重载 | 改参数列表或函数名 |
| `f(42)` 在 `f(int)` / `f(long)` 间二义 | 转换等级相同 | 显式转换或改接口 |
| 按值捕获大对象 | 复制开销 | 按 `const &` 捕获（注意生命期） |
| 返回按引用捕获的 lambda | 悬垂 | 按值捕获 |
| `[=]` 捕获 `this` 后对象销毁 | 悬垂 | 明确捕获需要的成员副本 |
| 递归没有基准情形 | 栈溢出 | 先写终止条件 |
| 默认实参重复定义 | 编译错误 | 只在声明写一次 |

## 4.13 语法卡片

| 术语 | 一句话定义 | 本章例子 |
|------|------------|----------|
| 声明 | 引入函数名和类型 | `int add(int, int);` |
| 定义 | 给出函数体 | `int add(...) { ... }` |
| 按值传递 | 复制实参 | `void f(int x);` |
| 按引用传递 | 直接操作实参 | `void f(int &x);` |
| `const` 引用 | 只读、可绑定临时对象 | `void f(const T &x);` |
| 默认实参 | 调用时可省略的实参 | `int power(int, int = 2);` |
| 重载 | 同名、不同参数列表 | `describe(int)`、`describe(double)` |
| 递归 | 函数调用自身 | `factorial(n - 1)` |
| lambda | 匿名函数对象 | `[x](int y) { return x + y; }` |
| 捕获 | lambda 保存外部变量的方式 | `[x]`、`[&x]` |
| 函数指针 | 指向函数的指针 | `int (*p)(int, int);` |
| `std::function` | 通用可调用对象包装器 | `std::function<int(int)>` |

## 4.14 练习

1. **参数传递**：写三个函数，分别按值、按引用、按 `const` 引用接收 `std::string`，在函数内打印地址和内容，比较是否复制。
2. **默认实参**：写一个 `repeat(std::string text, int times = 1)`，分别用默认值和显式值调用。
3. **重载**：为 `int`、`double`、`std::string` 写三个 `describe` 重载；再故意加一个只改返回类型的版本，读编译错误。
4. **递归**：分别用递归和迭代计算阶乘，比较 `n = 20` 时的结果（注意 `int` 溢出），改用 `unsigned long long`。
5. **lambda 捕获**：写一个按值捕获和按引用捕获的 lambda，在 lambda 创建后修改外部变量，观察两者看到的值。
6. **mutable**：写一个 `mutable` lambda，内部计数器每次调用加 1，外部变量不变。
7. **函数指针**：写一个 `apply` 函数，接收函数指针和两个整数，分别传入 `add`、`multiply` 和一个不捕获的 lambda。
8. **悬垂实验**：写一个返回按引用捕获局部变量的 lambda 的函数，**不要调用**它；用注释说明为什么危险。再改成按值捕获，验证安全。

## 4.15 下一章

**第 5 章 · 数组、指针与引用**

函数把行为组织起来，数组和指针把对象组织起来。下一章会讲原生数组、数组退化、指针算术、`const` 指针、指针的指针、数组指针、函数指针，以及 C++ 标准库提供的 `std::array` 和 `std::vector`。

届时，函数参数里的 `int values[]` 为什么等于 `int *values`，会有完整的答案。
