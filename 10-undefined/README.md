# 第 10 章 · 未定义行为与可移植性

> **卷一 · 语言**　|　前置知识：第 1–9 章　|　建议用时：60 分钟

> 标准不是“建议”，而是合同。UB 不是“结果不可预测”，而是“合同里根本没写这一条”。

模板让代码适配多种类型，但标准并不保证所有类型、所有平台、所有操作都有定义。这一章讲 C++ 里四种“没有唯一答案”的情况：未定义行为、未指定行为、实现定义行为，以及格式错误（ill-formed）。

这一章的目标不是让你背一份 UB 清单，而是建立一种阅读标准的方式：看到一条规则时，先问“这是保证、实现定义，还是完全没保证？”

## 10.1 本章语法地图

| 语法/工具 | 本章出现的位置 |
|-----------|----------------|
| `std::numeric_limits` | `min()`、`max()`、`epsilon()` |
| `alignof` / `alignas` | 对齐查询与指定 |
| `offsetof` | 结构体成员的偏移 |
| `std::memcpy` | 观察对象表示 |
| `std::uint32_t` | 固定宽度整数 |
| 浮点比较 | 容差比较 |
| `static_assert` | 编译期检查 |
| sanitizer | AddressSanitizer、UBSan |

## 10.2 四种“没有唯一答案”

### 未定义行为（undefined behavior, UB）

标准对某个操作**没有任何要求**。程序可能崩溃、产生垃圾值、看起来正常运行，或者做出任何事。编译器可以假设 UB 不会发生，并据此优化代码。

```cpp
int value = std::numeric_limits<int>::max();
value = value + 1;   // 有符号溢出：UB
```

注意：UB 不是“结果是随机的”。它是“标准不约束结果”，所以编译器可以删除代码、改变控制流，甚至让时间旅行式的优化发生（例如假设某个分支永远不会执行）。

### 未指定行为（unspecified behavior）

标准允许几种可能的行为，但不规定选哪一种，也不要求实现文档化。程序的行为仍然是“合法的”，只是每次执行/每个实现可能不同。

例子：

- 函数参数的求值顺序；
- `new` 失败时是否抛异常（默认 `new` 抛 `std::bad_alloc`，但 `nothrow` 版本返回 `nullptr`）；
- 某些标准库操作的复杂度/顺序。

### 实现定义行为（implementation-defined behavior）

标准允许实现自行选择，但要求实现**文档化**它的选择。程序仍然可移植，只是结果依赖实现。

例子：

- `sizeof(int)`；
- `char` 是否有符号；
- 有符号负数右移的结果；
- `std::numeric_limits<T>::is_iec559`。

### 格式错误（ill-formed）

程序不符合标准语法/语义，编译器必须给出诊断（有些情况不要求诊断，称为 ill-formed no diagnostic required）。这类问题通常在编译期发现。

```cpp
int x = "hello";   // 格式错误：const char* 不能初始化 int
```

## 10.3 UB 是一份契约

C++ 标准定义的是“什么程序有定义”，而不是“所有程序在真实机器上的行为”。当你写 UB 时，编译器有权假设它不会发生：

```cpp
int f(int x)
{
    return x + 1 > x;   // 编译器可能假设 x + 1 不会溢出
}
```

如果 `x == INT_MAX`，`x + 1` 是 UB；编译器可以据此假设 `x < INT_MAX`，甚至把整个条件优化成 `true`。这不是编译器“坏了”，而是它按合同办事。

常见的 UB 来源：

| 类别 | 例子 |
|------|------|
| 整数 | 有符号溢出、除零、移位越界、负数左移 |
| 内存 | 越界访问、悬垂指针、释放后使用、双重释放 |
| 初始化 | 读取未初始化的标量 |
| 指针 | 解引用空指针、不同数组指针相减、无效指针转换 |
| 对象生命期 | 在对象构造前/析构后使用、访问已结束生命期的对象 |
| 类型 | 违反严格别名、错误的 `reinterpret_cast` |
| 字符串 | 修改字符串字面量、越界写 `'\0'` |
| 并发 | 数据竞争、死锁（死锁本身不一定是 UB，但数据竞争是） |
| 库 | 违反标准库前置条件（如 `vector::operator[]` 越界） |

## 10.4 常见 UB 清单

### 有符号整数溢出

```cpp
int x = INT_MAX;
x = x + 1;   // UB
```

无符号整数溢出是**有定义的**：

```cpp
unsigned x = UINT_MAX;
x = x + 1;   // 0
```

需要检测溢出时，用更宽的类型、`__builtin_add_overflow`（编译器扩展）或 C++20 的 `std::add_overflow` 系列（如果可用）。

### 除零

```cpp
int x = 1 / 0;      // UB
double y = 1.0 / 0; // IEEE 754 下是 inf，不是整数除零
```

### 移位

```cpp
1 << 32;       // UB：移位量 >= 位宽
1 << -1;       // UB：负移位量
-1 << 1;       // UB：左移负数
```

无符号左移在结果不超出范围时有定义；有符号左移的规则更严格。移位量要小于类型的位宽。

### 读取未初始化标量

```cpp
int x;
std::cout << x;   // UB：x 是未初始化的自动变量
```

`int x{};` 会值初始化为 0。类类型如果有默认构造函数，会初始化成员；但内置类型成员不一定。

### 悬垂指针/引用

```cpp
int *p;
{
    int local = 42;
    p = &local;
}
std::cout << *p;   // UB：local 已销毁
```

返回局部变量的引用、保存 `c_str()` 后修改字符串、`vector` 扩容后继续用旧指针，都是同类问题。

### 越界访问

```cpp
int values[3] = {1, 2, 3};
std::cout << values[3];   // UB
```

`std::vector::operator[]` 越界也是 UB；`at()` 会抛异常。

### 空指针解引用

```cpp
int *p = nullptr;
std::cout << *p;   // UB
```

### 释放后使用/双重释放

```cpp
int *p = new int(42);
delete p;
delete p;   // UB
std::cout << *p; // UB
```

用 `std::unique_ptr` 可以消除大部分手动 `delete` 问题。

### 严格别名

```cpp
int value = 42;
float *p = reinterpret_cast<float *>(&value);
std::cout << *p;   // UB：通过 float* 访问 int 对象
```

例外：`char`、`unsigned char`、`std::byte` 可以别名任意对象。需要类型转换时用 `std::memcpy`（C++17）或 `std::bit_cast`（C++20）。

### 修改字符串字面量

```cpp
char *p = const_cast<char *>("hello");
p[0] = 'H';   // UB：字面量通常只读
```

### 数据竞争

两个线程同时访问同一内存位置，至少一个写，且没有同步，就是数据竞争，行为未定义。用 `std::atomic`、互斥量或线程间同步。

## 10.5 实现定义与未指定

### 实现定义的例子

```cpp
std::cout << sizeof(long);       // 4 或 8
std::cout << std::is_signed_v<char>; // true 或 false
```

有符号负数右移：

```cpp
int x = -8;
int y = x >> 1;   // 实现定义：可能是 -4，也可能是很大的正数
```

浮点到整数的越界转换不是实现定义，而是 UB：截断后的值无法用目标类型表示时，标准不约束结果。

```cpp
double d = 1e100;
int i = static_cast<int>(d);   // UB：1e100 不能用 int 表示
```

标准库的很多行为也依赖实现，例如 `std::string` 的 SSO 容量、`std::vector` 的扩容因子、`std::hash` 的具体值。

### 未指定的例子

```cpp
int f();
int g();
int x = f() + g();   // f 和 g 的顺序未指定
```

不同编译器、不同优化级别可能不同。程序仍然是合法的，但你不应该依赖某个顺序。

## 10.6 对象表示、填充与对齐

对象在内存里的字节称为**对象表示**（object representation）。对 `int`、`float` 这类类型，表示由实现决定；对类类型，还包括填充字节。

```cpp
struct Layout {
    char tag;
    int value;
    char tail;
};
```

在常见平台上：

- `tag` 在偏移 0；
- 为了对齐 `int`，`value` 在偏移 4；
- `tail` 在偏移 8；
- `sizeof(Layout)` 可能是 12，末尾还有 3 字节填充。

填充字节的值是**不确定的**，不要用 `memcmp` 比较两个结构体是否相等：

```cpp
Layout a{};
Layout b{};
std::memcmp(&a, &b, sizeof a);   // 可能非 0，因为填充字节不同
```

需要逐成员比较，或让类自己实现 `operator==`。

### 对齐

`alignof(T)` 查询类型的对齐要求，`alignas(N)` 指定更严格的对齐：

```cpp
struct alignas(16) Aligned {
    char bytes[16];
};

static_assert(alignof(Aligned) == 16);
```

对齐影响：

- 结构体布局和大小；
- 动态分配的对齐（C++17 的 `new` 支持过对齐类型）；
- 缓存行、SIMD、硬件访问要求；
- 跨平台二进制接口（ABI）。

### 用 `memcpy` 观察对象表示

```cpp
float value = 3.5f;
std::uint32_t bits = 0;
std::memcpy(&bits, &value, sizeof bits);
```

`std::memcpy` 复制的是字节，不违反严格别名。前提是源/目标类型是**可平凡复制**（trivially copyable），并且大小匹配。

C++20 的 `std::bit_cast` 更直接：

```cpp
float value = 3.5f;
std::uint32_t bits = std::bit_cast<std::uint32_t>(value);
```

它要求源/目标大小相同、可平凡复制，并且是 `constexpr` 友好的。

## 10.7 可移植性清单

写可移植代码时，逐项检查：

1. **整数宽度**：不要假设 `int` 是 32 位；需要固定宽度用 `<cstdint>`。
2. **`char` 符号**：不要假设 `char` 有符号；表示字节用 `unsigned char` 或 `std::byte`。
3. **字节序**：不要假设小端；网络协议和文件格式要显式转换。
4. **对齐**：不要假设结构体没有填充；不要用 `memcmp` 比较含填充的结构体。
5. **浮点**：不要假设 IEEE 754；用 `std::numeric_limits` 查询。
6. **求值顺序**：不要依赖未指定的顺序；拆成多条语句。
7. **有符号溢出**：不要依赖回绕；用无符号或显式检查。
8. **右移负数**：不要依赖算术右移；用除法或显式处理。
9. **对象表示**：不要用 `reinterpret_cast` 做类型双关；用 `memcpy` / `bit_cast`。
10. **ABI**：不要假设不同编译器/标准库之间的二进制兼容。
11. **未定义行为**：用警告、sanitizer、静态分析工具提前发现。
12. **语言版本**：明确使用 C++17 还是 C++20；不同版本规则不同。

## 10.8 工具：让 UB 现形

### 编译警告

```sh
c++ -std=c++17 -Wall -Wextra -Wpedantic -Werror ...
```

`-Werror` 把警告变成错误，适合 CI。常见有用警告：

- `-Wsign-compare`：有符号/无符号比较；
- `-Wconversion`：隐式窄化；
- `-Wshadow`：变量遮蔽；
- `-Wreorder`：成员初始化顺序；
- `-Wimplicit-fallthrough`：`switch` 贯穿；
- `-Wnull-dereference`：空指针解引用（GCC）。

### Sanitizer

AddressSanitizer（ASan）检测越界、悬垂、重复释放、泄漏；UndefinedBehaviorSanitizer（UBSan）检测有符号溢出、移位、空指针等：

```sh
c++ -std=c++17 -g -fsanitize=address,undefined portable.cpp -o portable
./portable
```

Sanitizer 有运行时开销，适合测试和调试，不适合最终发布。

### 静态分析

Clang-Tidy、Cppcheck、编译器自带的静态分析器可以在不运行程序的情况下发现部分问题。它们不能证明程序没有 UB，但能覆盖常见模式。

### Debug 与 Release

优化器会利用 UB 做假设，所以某些 UB 在 `-O0` 下“看起来正常”，在 `-O2` 下崩溃或产生奇怪结果。**不要用“Debug 能跑”证明程序正确。**

## 10.9 代码导览

[`portable.cpp`](portable.cpp) 只演示**有定义**的行为，并在注释里标出邻近的 UB 边界：

- `show_limits()`：`std::numeric_limits` 的范围、精度、IEEE 754 标志；
- `show_char_signedness()`：`char` 的符号性；
- `show_layout()`：结构体填充、`offsetof`、`alignof`、`alignas`；
- `show_object_representation()`：用 `memcpy` 观察 `float` 的位模式；
- `show_float_comparison()`：`0.1 + 0.2` 与容差比较；
- `show_defined_behavior()`：尾后指针、数组边界、`nullptr` 检查，以及被注释掉的 UB 示例。

编译与运行：

```sh
make 10-undefined/portable
./10-undefined/portable
```

再用 sanitizer 跑一遍：

```sh
c++ -std=c++17 -g -fsanitize=address,undefined portable.cpp -o portable-san
./portable-san
```

代码里没有执行任何 UB；被注释掉的例子是为了让你看到“边界在哪里”，而不是让你复制到生产代码。

## 10.10 常见误区

| 误区 | 事实 |
|------|------|
| “UB 就是结果随机” | 标准不约束结果，编译器可以做任何假设 |
| “Debug 能跑就没问题” | 优化器可能利用 UB，Release 才暴露 |
| “`int` 一定是 32 位” | 标准只保证最小范围 |
| “`char` 一定是 8 位有符号” | `CHAR_BIT` 至少 8，符号性由实现决定 |
| “无符号溢出也是 UB” | 无符号回绕有定义 |
| “`reinterpret_cast` 可以随便转” | 严格别名和生命周期规则仍然适用 |
| “`memcpy` 可以比较任意结构体” | 填充字节不确定，结果不可靠 |
| “`vector::operator[]` 会检查边界” | 不会；用 `at()` |
| “字符串字面量可以修改” | 修改是 UB |
| “两个线程只读同一变量也安全” | 只读通常安全；一旦有写且无同步就是数据竞争 |
| “`delete` 后指针自动置空” | 不会；置空需要手动或用智能指针 |

## 10.11 语法卡片

| 术语 | 一句话定义 | 本章例子 |
|------|------------|----------|
| 未定义行为 | 标准不约束结果 | 有符号溢出、越界 |
| 未指定行为 | 标准允许多种结果，不规定选哪个 | 参数求值顺序 |
| 实现定义 | 实现必须文档化其选择 | `sizeof(int)` |
| 格式错误 | 不符合标准，通常编译期诊断 | 类型不匹配 |
| 对象表示 | 对象在内存中的字节 | `memcpy` 观察 |
| 填充 | 为了对齐插入的不确定字节 | 结构体尾部 |
| 对齐 | 地址必须是某个值的整数倍 | `alignof`、`alignas` |
| 严格别名 | 通过不兼容类型访问对象 | `reinterpret_cast` 陷阱 |
| 数据竞争 | 多线程无同步的读写 | UB |
| Sanitizer | 运行时检测 UB 的工具 | ASan、UBSan |

## 10.12 练习

1. **范围**：用 `std::numeric_limits` 打印 `int`、`unsigned`、`long long`、`float`、`double` 的范围和精度。
2. **有符号溢出**：写一个函数检测 `int` 加法溢出（不执行 UB），用 `long long` 或边界判断实现。
3. **填充**：定义几个结构体，打印 `sizeof`、`alignof`、`offsetof`，画出内存布局。
4. **对齐**：用 `alignas(16)` 定义一个结构体，验证 `alignof` 和 `sizeof`；再尝试 `alignas(1)` 降低 `int` 的对齐，读编译错误。
5. **对象表示**：用 `memcpy` 打印 `float`、`double`、`int` 的字节（十六进制），比较大小端。
6. **浮点**：验证 `0.1 + 0.2 != 0.3`，用容差写一个 `almost_equal`。
7. **严格别名**：写一个 `reinterpret_cast` 的类型双关例子，**不要运行**；再用 `memcpy` 写安全版本。
8. **Sanitizer**：故意写一个越界访问，用 ASan 运行，读报告；修复后确认报告消失。
9. **数据竞争**：用两个 `std::thread` 同时递增一个普通 `int`，用 ThreadSanitizer 运行，读报告；再用 `std::atomic` 修复。

## 10.13 下一章

**第 11 章 · 综合练习：一个小型表达式求值器**

前面十章分别讲了类型、表达式、控制流、函数、数组与指针、字符串、类、存储期、模板和 UB。下一章不再引入新语法，而是把它们拼成一个真正能运行的小程序：一个支持 `+ - * /`、括号、一元正负号和十进制小数的表达式求值器。

届时，词法分析、递归下降、错误报告、`std::string_view`、`std::optional` 和类的前后关系，会第一次同时出现在一个程序里。
