# 第 1 章 · 对象、类型与声明

> **卷一 · 语言**　|　前置知识：第 0 章　|　建议用时：40 分钟

> 类型不是贴在数据上的标签，而是编译器理解内存的语法。

第 0 章让程序说出了第一句话。这一章要问一个更底层的问题：那句话里的 `"Hello, World!\n"`，在内存里到底占多大地方？`int` 又是什么？为什么同样是整数，`int`、`long`、`unsigned int` 的行为不一样？引用和指针到底差在哪里？

这一章只讲语言本身：对象、类型、声明、初始化、`auto`、`const`、`constexpr`、引用、`sizeof` 和转换。标准库的 `std::cout` 仍然只是我们的扬声器。

## 1.1 本章语法地图

| 语法 | 本章出现的位置 |
|------|----------------|
| 对象与变量 | `int x = 10;` |
| 声明与定义 | `extern int x;`、`int x;` |
| 基本类型 | `bool`、`char`、`short`、`int`、`long`、`long long`、`float`、`double`、`long double` |
| 定宽整数 | `std::int32_t`、`std::uint64_t` |
| `sizeof` / `alignof` | `sizeof(int)`、`alignof(double)` |
| 字面量 | `42`、`0x2A`、`0b101010`、`1'000'000`、`1e3`、`0.5f` |
| 初始化 | `int x = 10;`、`int y(20);`、`int z{30};`、`int w{};` |
| `auto` / `decltype` | `auto n = 42;`、`decltype(n) m = n;` |
| `const` / `constexpr` | `const int answer = 42;`、`constexpr int size = 4;` |
| 引用 | `int &ref = x;`、`const int &cref = x;` |
| 指针 | `int *ptr = &x;`、`nullptr` |
| 类型转换 | `static_cast<double>(7) / 2`、`static_cast<unsigned>(-1)` |
| 静态断言 | `static_assert(sizeof(char) == 1);` |

## 1.2 对象：内存里的一块地方

C++ 标准把**对象**（object）定义为一块存储区域（region of storage），它有类型和生命期。这个名字听起来抽象，但它就是“内存里的一块地方”。

一个对象有三个关键属性：

1. **存储**：它在内存里占多少字节，地址在哪里，对齐要求是多少。
2. **类型**：这些字节如何解释，能做哪些运算。
3. **生命期**：它从什么时候存在，到什么时候消失。

```cpp
int x = 42;
```

这行代码做的不只是“把 42 放进一个叫 x 的东西”。它声明了一个类型为 `int` 的对象 `x`，为它分配存储，并用值 `42` 初始化。类型决定了：

- `x` 占几个字节（由实现决定，常见是 4）；
- `x` 能表示哪些值（有符号还是无符号，范围多大）；
- 哪些运算对 `x` 有意义（加减、取地址、和指针互转等）；
- `x` 的地址需要按什么边界对齐。

**变量**通常指一个有名字的对象。但对象不一定有名字：动态分配的对象、临时对象、数组元素都可能是对象而没有自己的名字。名字是语法层面的东西，对象是运行时的存储。

C++ 里还有一个容易混淆的概念：**引用不是对象**。引用是已有对象的别名，它本身不占一块可以取地址的独立存储。第 1.7 节会展开。

## 1.3 声明与定义：名字和存储不是一回事

这是 C 和 C++ 都容易混淆的一对概念，但 C++ 的规则更严格。

- **声明**（declaration）告诉编译器：某个名字存在，它是什么类型。
- **定义**（definition）除了声明名字，还真正分配存储，或者给出函数体。

看几个例子：

```cpp
extern int x;      // 声明：x 在别处定义
int x;             // 定义：在 C++ 中，文件作用域的 int x; 就是定义
int x = 42;        // 定义：分配存储并初始化

int f();           // 函数声明
int f() {          // 函数定义：给出函数体
    return 1;
}
```

在块作用域里：

```cpp
void demo()
{
    int y;         // 定义：自动存储，未初始化
    extern int z;  // 声明：z 在别处定义
}
```

必须强调一个 C 与 C++ 的差异：**C++ 没有 C 的“暂定定义”（tentative definition）**。在 C 里，文件作用域的 `int x;` 可以出现多次，链接阶段再合并；在 C++ 里，`int x;` 就是一个定义，同一个翻译单元里重复定义是错误，多个翻译单元重复定义会违反 ODR（一次定义规则），通常表现为链接错误。第 8 章会专门讲链接和 ODR。

`typedef` 也是声明，但它声明的不是对象，而是类型别名：

```cpp
typedef unsigned long ulong;   // C 风格
using ulong2 = unsigned long;  // C++11 起更常用的写法
```

现在只需要记住一个判断方法：**这句话有没有让编译器为某个对象留出存储，或者为某个函数提供函数体？** 有，就是定义；没有，通常只是声明。

## 1.4 基本类型：C++ 只保证下限

C++ 的类型系统继承自 C，同样围绕“实现”设计。标准规定每种类型**至少**能表示什么，但不规定它一定是多少位。你写的代码如果假设 `int` 永远是 32 位，就已经把可移植性押上去了。

### 整数类型

| 类型 | 标准保证的最小范围 | 常见宽度 |
|------|--------------------|----------|
| `signed char` | 至少 -127 .. 127 | 8 位 |
| `unsigned char` | 至少 0 .. 255 | 8 位 |
| `char` | 同 `signed char` 或 `unsigned char`，由实现决定 | 8 位 |
| `short` | 至少 -32767 .. 32767 | 16 位 |
| `unsigned short` | 至少 0 .. 65535 | 16 位 |
| `int` | 至少 -32767 .. 32767 | 32 位 |
| `unsigned int` | 至少 0 .. 65535 | 32 位 |
| `long` | 至少 -2147483647 .. 2147483647 | 32 或 64 位 |
| `unsigned long` | 至少 0 .. 4294967295 | 32 或 64 位 |
| `long long` | 至少 -9223372036854775807 .. 9223372036854775807 | 64 位 |
| `unsigned long long` | 至少 0 .. 18446744073709551615 | 64 位 |

几个关键点：

- `char`、`signed char`、`unsigned char` 是**三种不同的类型**。普通 `char` 到底有符号还是无符号，由实现决定。
- `sizeof(char) == 1` 永远成立。这里的“1”是**一个 C++ 字节**，不是“8 位”。一个字节有多少位由 `CHAR_BIT` 给出，标准只保证它至少为 8。
- `short`、`int`、`long`、`long long` 的宽度满足：
  `sizeof(short) <= sizeof(int) <= sizeof(long) <= sizeof(long long)`。
- 标准只保证最小范围，所以 `int` 在 16 位机器上可以是 16 位。需要精确宽度时，用 `<cstdint>` 里的 `std::int32_t`、`std::uint64_t` 等；但这些类型只有在实现确实支持该宽度时才存在。

### 布尔类型

C++ 的 `bool` 是真正的内置类型，不是 C 的 `_Bool` 加宏。它只有两个值：`false` 和 `true`，分别转换为整数 `0` 和 `1`。任何标量转成 `bool` 时，等于 0 得 `false`，不等于 0 得 `true`。

```cpp
bool flag = false;
flag = 42;        // flag 变成 true，不是 42
```

`sizeof(bool)` 由实现决定，常见是 1，但标准没有规定它必须等于 `sizeof(char)`。

### 浮点类型

| 类型 | 常见宽度 | 说明 |
|------|----------|------|
| `float` | 32 位 | 单精度，至少 6 位十进制有效数字 |
| `double` | 64 位 | 双精度，至少 10 位十进制有效数字 |
| `long double` | 80/128 位 | 扩展精度，具体由实现决定 |

浮点数不是“带小数点的精确数”。`0.1` 在二进制浮点里通常不能精确表示，所以不要用 `==` 比较两个浮点结果。需要比较时，通常看差值是否小于一个容差；这是数值计算的话题，本章只建立“浮点是近似”的直觉。

浮点类型还可能是实现定义的格式（例如是否支持 IEEE 754、是否有无穷大和 NaN）。`<limits>` 里的 `std::numeric_limits<float>::digits10`、`::epsilon()`、`::is_iec559` 等可以告诉你当前实现的精度和格式。

### `void`、`std::size_t` 和 `std::ptrdiff_t`

- `void` 是**不完整类型**，不能定义对象。它只用于函数返回类型（表示不返回值）和 `void *` 这种通用对象指针。
- `std::size_t` 是 `sizeof` 的结果类型，也是表示对象大小的无符号整数类型。它定义在 `<cstddef>` 等头文件里，具体宽度由实现决定。
- `std::ptrdiff_t` 是两个指针相减的结果类型，是有符号整数类型，也定义在 `<cstddef>`。

`std::size_t` 是无符号的，这一点会在比较和减法里制造陷阱。C++ 的 iostream 会自动按类型输出，不需要 `%zu`；但如果你用 C 的 `printf`，打印它要用 `%zu`，打印 `std::ptrdiff_t` 用 `%td`。

### `std::byte`

C++17 引入了 `std::byte`，定义在 `<cstddef>`：

```cpp
std::byte b{0x2A};
```

它是 `enum class`，不是整数类型，不能直接做算术。它的用途是明确表示“这里存的是原始字节，不是字符，也不是数字”。需要转换时用 `std::to_integer<int>(b)`。第 10 章会再遇到它。

## 1.5 初始化：四种写法，不同规则

C++ 的初始化语法比 C 多，也更容易踩坑。最常见的四种：

```cpp
int a = 10;    // 复制初始化
int b(20);     // 直接初始化
int c{30};     // 列表初始化（C++11）
int d{};       // 值初始化：d == 0
```

它们不是完全等价的：

- **复制初始化**用 `=`，允许隐式转换。`int a = 3.9;` 合法，`a` 变成 3。
- **直接初始化**用括号，也允许隐式转换。它还会参与重载决议，可能选到奇怪的构造函数。
- **列表初始化**用花括号，**拒绝窄化转换**。`int c{3.9};` 是编译错误，`char c{300};` 也是。
- **空花括号** `{}` 对内置类型做值初始化，结果是 0；对类类型会调用默认构造函数。

列表初始化还有一个重要用途：防止“最令人烦恼的解析”（most vexing parse）：

```cpp
int value(10);   // 定义变量 value，初值 10
int value2();    // 声明一个函数，返回 int，不接受参数！不是变量
int value3{};    // 定义变量，值初始化为 0
```

`int value2();` 不是变量定义，而是函数声明。这是 C++ 语法继承自 C 的歧义；用 `{}` 就能避开。

列表初始化也不是万能的。对类类型，如果类有 `std::initializer_list` 构造函数，花括号会优先选它，可能导致意外的重载结果。第 7 章会讲构造函数时再展开。

## 1.6 `auto`、`decltype`、`const` 与 `constexpr`

### `auto`：让编译器推导类型

```cpp
auto n = 42;      // int
auto d = 1.5;     // double
auto p = &n;      // int *
```

`auto` 的推导规则接近模板参数推导：

- 顶层 `const` 和引用会被丢掉：`const int x = 1; auto y = x;` 中 `y` 是 `int`。
- 如果你要保留引用，写 `auto &`：`auto &r = x;`。
- 如果你要保留 `const`，写 `const auto`。
- `auto` 不会推导出数组类型；数组会退化为指针。

`auto` 不是“动态类型”。变量的类型在编译期就确定了，只是你让编译器替你写出来。

### `decltype`：查询表达式的类型

`decltype(expr)` 给出表达式的类型，规则比 `auto` 更细：

```cpp
int x = 1;
decltype(x) a = 2;      // int
decltype((x)) b = x;    // int &：加括号变成左值表达式
decltype(1 + 2) c = 3;  // int
```

一个常见的组合是 `decltype(auto)`，它用 `decltype` 的规则做返回类型推导，常用于完美转发和泛型代码。第 9 章会再提到。

### `const`：运行期不变

```cpp
const int answer = 42;
```

`const` 表示这个对象在初始化后不能再被修改。它可以在运行时初始化：

```cpp
int input = 7;
const int value = input * 2;
```

`const` 对象必须初始化，因为之后没有机会再赋值。对指针来说，`const` 的位置很重要：

```cpp
int value = 10;
const int *p1 = &value;        // 指向 const int 的指针：不能通过 p1 改值
int *const p2 = &value;        // const 指针：不能改指向，但可以改值
const int *const p3 = &value;  // 两者都不能改
```

### `constexpr`：编译期常量

```cpp
constexpr int size = 4;
int buffer[size];     // 合法：size 是编译期常量
```

`constexpr` 蕴含 `const`，但要求更强：对象或函数必须能在编译期求值。C++17 的 `constexpr` 函数可以包含循环、局部变量和分支，不再局限于 C++11 的“单条 return”。

```cpp
constexpr int square(int value)
{
    return value * value;
}

static_assert(square(5) == 25);
```

`constexpr` 函数也可以接受运行时参数，在运行时调用；只是当它用于常量表达式时，编译器必须在编译期算出结果。

### `const` 与 `constexpr` 的区别

| 写法 | 初始化时机 | 能否用于数组长度 / 模板参数 |
|------|------------|------------------------------|
| `const int x = 42;` | 可以是运行期 | 只有常量表达式才行 |
| `constexpr int x = 42;` | 必须编译期 | 可以 |

在 C++17 里，文件作用域的 `const` 变量默认是**内部链接**；`constexpr` 变量也一样。如果你要在头文件里定义一个被多个翻译单元共享的常量，C++17 可以用 `inline constexpr`，或者放在类里的 `static constexpr` 成员。第 8 章会讲链接。

## 1.7 引用：对象的别名

引用是 C++ 相对 C 最重要的新增之一：

```cpp
int value = 10;
int &ref = value;   // ref 是 value 的别名

ref = 20;           // value 也变成 20
```

引用有几条硬规则：

1. **必须初始化**，不能先声明后绑定。
2. **不能重新绑定**。`ref = other;` 是把 `other` 的值赋给 `value`，不是让 `ref` 改指向 `other`。
3. **没有空引用**。引用必须绑定到一个有效对象。
4. **引用不是对象**。`sizeof(ref)` 得到的是被引用对象的大小，`&ref` 得到的是被引用对象的地址。
5. **不能有引用的引用、引用的数组、指向引用的指针**（但可以有数组的引用、指针的引用）。

```cpp
int a = 1;
int b = 2;
int &ref = a;
ref = b;            // a 变成 2，ref 仍然绑定 a
int *ptr = &ref;    // ptr 指向 a
```

引用的常见用途：

- 函数参数：避免拷贝，同时允许修改实参。
- 范围 for：`for (const auto &item : container)`。
- 运算符重载：返回 `*this` 的引用，支持链式调用。
- 别名：给长类型或长表达式起短名字。

### 引用与指针的对比

| 维度 | 引用 | 指针 |
|------|------|------|
| 是否必须绑定 | 是 | 否，可以是 `nullptr` |
| 能否重新绑定 | 否 | 是 |
| 是否占独立存储 | 通常不（实现可以优化掉） | 是 |
| 能否做算术 | 否 | 是 |
| 能否有空值 | 否 | 是 |
| 解引用语法 | 直接用 | `*p` |
| 典型用途 | 参数、别名、返回值 | 可空关系、动态对象、底层操作 |

引用更适合表达“这里一定有一个对象”；指针更适合表达“可能没有”“需要重新指向”“需要做地址运算”。第 5 章会展开。

## 1.8 `sizeof` 与 `alignof`

`sizeof` 是运算符，不是函数。它返回一个对象或类型占用的字节数，结果的类型是 `std::size_t`。

```cpp
std::cout << sizeof(int);    // 类型：必须加括号
std::cout << sizeof value;   // 对象：可以不加括号
```

几个必须建立的直觉：

1. `sizeof(char) == 1` 永远成立。
2. `sizeof` 的结果单位是 C++ 字节，不是 bit。要知道一个字节几位，看 `CHAR_BIT`。
3. `sizeof` 在大多数情况下是编译期常量。C++ 没有 C 的变长数组（VLA）；数组长度必须是常量表达式（或者用 `std::vector` 在运行时分配）。
4. `sizeof` 的操作数**通常不会被求值**：

```cpp
int counter = 0;
std::size_t n = sizeof(counter++);
// counter 仍然是 0，因为 sizeof 不需要真的执行 counter++
```

5. `sizeof` 数组得到整个数组的字节数，`sizeof` 指针得到指针本身的字节数。数组和指针在这一点上完全不同，第 5 章会展开。
6. `sizeof` 引用得到被引用对象的大小，不是引用本身的大小。

`alignof` 查询类型的对齐要求：

```cpp
std::cout << alignof(double);
```

对齐是“对象地址必须是某个值的整数倍”的约束。它影响结构体布局、缓存和底层编程；第 7 章讲类的布局，第 10 章讲填充与可移植性。

`sizeof` 返回无符号的 `std::size_t`，所以它和负整数比较时会出问题：

```cpp
if (sizeof(int) > -1) {
    // 这个条件为假！
}
```

`-1` 会被转换成很大的无符号数，于是 `4 > 一个巨大的数` 为假。编译器通常会用 `-Wsign-compare` 提醒你，但最好的办法是不要写这种比较。

## 1.9 类型转换

C++ 的转换比 C 多，也更显式。先看最基本的两种：

```cpp
int i = 3;
double d = i;              // 隐式转换：int -> double
int j = static_cast<int>(3.9); // 显式转换：截断为 3
```

C++ 提供四种命名的显式转换：

| 转换 | 用途 | 例子 |
|------|------|------|
| `static_cast` | 相关类型之间的转换 | `static_cast<double>(7) / 2` |
| `dynamic_cast` | 多态类型的安全向下转换 | `dynamic_cast<Derived *>(base)` |
| `const_cast` | 去掉或加上 `const` | `const_cast<int &>(cref)` |
| `reinterpret_cast` | 重新解释位模式（危险） | `reinterpret_cast<std::uintptr_t>(p)` |

本章只需要 `static_cast`。另外三种会在后续章节或底层专题里出现；日常代码应尽量少用 `reinterpret_cast` 和 `const_cast`。

### 整数提升与寻常算术转换

小整数类型（`bool`、`char`、`short` 等）参与算术运算时，会先**提升**为 `int`（或 `unsigned int`）：

```cpp
char a = 100;
char b = 100;
int sum = a + b;   // 先提升为 int，结果是 200，不会溢出 char
char c = a + b;    // 200 超出 signed char 范围，结果由实现定义
```

两个不同类型的整数一起运算时，会进行**寻常算术转换**：通常向更宽、无符号的一方转换。这导致经典的陷阱：

```cpp
int negative = -1;
unsigned int one = 1;
if (static_cast<unsigned int>(negative) < one) {
    // -1 被转换成 UINT_MAX，条件为假
}
```

无符号整数的回绕是**有定义的**：`0u - 1u` 得到 `UINT_MAX`。有符号整数的溢出是**未定义行为**，第 10 章会讲。

### 窄化转换

列表初始化拒绝窄化：

```cpp
int a{3.9};       // 错误：double -> int 是窄化
char b{300};      // 错误：300 超出 char 范围
int c{300};       // 合法
```

“窄化”不只是“变小”，还包括丢失精度、改变符号等。编译器会在编译期检查常量表达式；对于运行期值，某些窄化只能在运行时检测。

## 1.10 代码导览

[`types.cpp`](types.cpp) 分成五个部分：

- `show_sizes()`：打印常见类型的大小和对齐；
- `show_ranges()`：用 `std::numeric_limits` 打印范围和浮点精度；
- `show_literals()`：展示十进制、十六进制、八进制、二进制、数字分隔符和字符字面量；
- `show_initialization()`：展示四种初始化、`auto`、`decltype`、引用和静态断言；
- `show_conversions()`：展示截断、无符号回绕和整数除法。

运行：

```sh
make 01-types/types
./01-types/types
```

输出中的具体宽度和对齐依赖平台。教材不假设 `int` 一定是 4 字节，也不假设 `char` 一定有符号；代码只用标准保证的性质做断言。

## 1.11 常见陷阱

| 写法 | 问题 | 建议 |
|------|------|------|
| `int x;` 后直接读 `x` | 自动存储期的内置类型未初始化，读取是 UB | 用 `int x{};` 或立即赋值 |
| `int x = 3.9;` | 隐式窄化，`x` 是 3 | 用 `static_cast<int>` 明确意图，或用 `{}` 让编译器拒绝 |
| `int x();` | 这是函数声明，不是变量 | 用 `int x{};` 或 `int x(0);` |
| `sizeof(int) > -1` | 有符号/无符号比较，结果反直觉 | 避免混合比较，或显式转换 |
| `const int x;` | 未初始化，编译错误 | 定义时初始化 |
| `int &r;` | 引用未绑定，编译错误 | 定义时绑定 |
| `auto &r = 42;` | 不能绑定非 const 左值引用到临时对象 | 用 `const auto &r = 42;` |
| `char c = 300;` | 超出范围，结果由实现定义 | 用 `static_cast<char>` 并确认范围，或改用更宽类型 |
| `long` 当作 64 位 | Windows 上 `long` 是 32 位 | 需要固定宽度时用 `<cstdint>` |

## 1.12 语法卡片

| 术语 | 一句话定义 | 本章例子 |
|------|------------|----------|
| 对象 | 一块有类型和生命期的存储区域 | `int x = 42;` |
| 声明 | 引入名字和类型，不一定分配存储 | `extern int x;` |
| 定义 | 分配存储或给出函数体 | `int x = 42;` |
| 引用 | 已有对象的别名，不是独立对象 | `int &ref = x;` |
| `const` | 初始化后不可修改 | `const int answer = 42;` |
| `constexpr` | 必须能在编译期求值 | `constexpr int n = 4;` |
| `auto` | 让编译器从初始化器推导类型 | `auto n = 42;` |
| `decltype` | 查询表达式的类型 | `decltype(x) y = x;` |
| `sizeof` | 对象或类型占用的字节数 | `sizeof(int)` |
| `alignof` | 类型的对齐要求 | `alignof(double)` |
| 窄化转换 | 可能丢失信息的隐式转换 | `int x{3.9};`（被拒绝） |
| ODR | 一个实体在程序中只能有一个定义（有例外） | 第 8 章展开 |

## 1.13 练习

1. **量一量**：写一个程序，打印你机器上 `bool`、`char`、`short`、`int`、`long`、`long long`、`float`、`double`、`long double`、`void *` 的 `sizeof` 和 `alignof`。和本章输出对比。
2. **范围**：用 `std::numeric_limits` 打印 `int`、`unsigned`、`long long` 的最小值和最大值。解释为什么 `std::numeric_limits<int>::min()` 不是 `-std::numeric_limits<int>::max()`。
3. **初始化**：分别用 `=`、`()`、`{}` 初始化 `int`、`double`、`char`。尝试把 `3.9` 放进 `int` 的花括号，读编译错误。
4. **auto 与引用**：定义 `int x = 1; const int cx = 2;`，分别用 `auto`、`auto &`、`const auto &`、`decltype` 声明变量，打印 `sizeof` 和值，验证类型推导规则。
5. **引用不是对象**：定义 `int value = 10; int &ref = value;`，比较 `&value`、`&ref`、`sizeof(ref)`，并尝试写 `int &ref2 = ref;`，观察它绑定到谁。
6. **转换**：验证 `static_cast<unsigned>(-1)` 的值，以及 `0u - 1u` 的结果。再写一个 `int` 溢出的例子，但**不要运行**，只查标准说明它为什么是 UB。
7. **`constexpr`**：写一个 `constexpr` 函数计算阶乘，用 `static_assert` 验证 `factorial(5) == 120`，再在运行时打印 `factorial(6)`。
8. **破坏实验**：把 `const int answer = 42;` 改成 `const int answer;`，编译并读诊断；再把 `int &ref = value;` 改成 `int &ref;`，比较两条错误信息。

## 1.14 下一章

**第 2 章 · 表达式、值类别与求值**

下一章我们会进入表达式内部：一个表达式是左值、纯右值还是将亡值？`i++` 和 `++i` 的区别在哪里？`a = b` 的求值顺序在 C++17 里保证了什么？为什么 `std::cout << f() << g()` 的输出顺序是确定的，而 `f() + g()` 不是？

类型给了对象形状，表达式则让对象开始运动。
