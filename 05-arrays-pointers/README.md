# 第 5 章 · 数组、指针与引用

> **卷一 · 语言**　|　前置知识：第 1–4 章　|　建议用时：55 分钟

> 数组是连续的对象，指针是对象的地址，引用是对象的别名。三者看起来都能“指向”东西，语义却完全不同。

函数把行为组织起来，数组和指针把对象组织起来。这一章讲原生数组、数组退化、指针算术、`const` 指针、指针的指针、数组指针、函数指针，以及 C++ 标准库提供的 `std::array` 和 `std::vector`。

C++ 继承了 C 的数组和指针，但又加了引用和容器。现代 C++ 的默认选择通常是 `std::array` 或 `std::vector`，而不是裸数组和裸指针；但理解底层表示仍然是必要的，否则你无法解释迭代器、字符串视图和 ABI。

## 5.1 本章语法地图

| 语法 | 本章出现的位置 |
|------|----------------|
| 原生数组 | `int values[5];` |
| 数组初始化 | `int values[] = {1, 2, 3};` |
| 数组长度 | `sizeof(values) / sizeof(values[0])` |
| `std::array` | `std::array<int, 5> values;` |
| `std::vector` | `std::vector<int> values;` |
| 数组退化 | 数组名转成指向首元素的指针 |
| 取地址 / 解引用 | `&x`、`*p` |
| 指针算术 | `p + 1`、`p - q` |
| `const` 指针 | `const int *`、`int *const` |
| 指针的指针 | `int **pp;` |
| 数组指针 | `int (*row)[3];` |
| 函数指针 | `int (*operation)(int, int);` |
| `nullptr` | `int *p = nullptr;` |
| 结构化绑定 | `auto [a, b] = ...;` |

## 5.2 数组：连续的一块对象

数组是一组**同类型、连续存放**的对象：

```cpp
int values[5] = {10, 20, 30, 40, 50};
```

它包含 5 个 `int`，在内存里依次排列。`values[0]` 是第一个，`values[4]` 是最后一个。数组的长度必须是**常量表达式**（C++ 没有 C 的变长数组）：

```cpp
constexpr std::size_t size = 5;
int values[size];   // 合法

int runtime = 5;
int bad[runtime];   // 标准 C++ 不允许
```

需要运行时长度时，用 `std::vector`。

数组的初始化方式：

```cpp
int a[5] = {1, 2, 3, 4, 5};   // 全部初始化
int b[5] = {1, 2};            // 其余元素值初始化为 0
int c[5] = {};                // 全部为 0
int d[] = {1, 2, 3};          // 长度推导为 3
```

数组不能直接赋值或比较：

```cpp
int x[3] = {1, 2, 3};
int y[3] = x;          // 错误
if (x == y) { ... }    // 比较的是指针，不是内容
```

要复制数组，用 `std::copy`、`std::array` 或 `std::vector`；要比较内容，用 `std::equal` 或容器。

### 数组的长度

```cpp
int values[5];
std::size_t count = sizeof(values) / sizeof(values[0]);
```

这是 C 时代的惯用法，只在**数组类型仍然可见**时有效。一旦数组退化为指针，`sizeof` 就只得到指针大小。C++17 提供了 `std::size(values)`（定义在 `<iterator>`），对原生数组也能工作，但同样要求数组类型可见。

## 5.3 数组名与退化

数组名在大多数表达式里会**退化**（decay）为指向首元素的指针：

```cpp
int values[5] = {1, 2, 3, 4, 5};
int *p = values;       // 等价于 &values[0]
```

退化发生在：

- 数组作为函数参数；
- 数组参与算术或比较；
- 数组被赋值给指针。

退化不发生在：

- `sizeof(values)`；
- `&values`（得到“指向整个数组的指针”，类型是 `int (*)[5]`）；
- 用引用来绑定数组：`int (&ref)[5] = values;`；
- `decltype(values)`（得到 `int[5]`）。

```cpp
void print(int values[5]);   // 参数里数组退化为 int *
void print(int *values);     // 与上一行完全等价
void print(int values[]);    // 也等价
```

所以函数内部无法通过 `sizeof(values)` 得到数组长度。必须额外传长度，或者用模板 / 容器：

```cpp
template <std::size_t N>
void print(const int (&values)[N])
{
    for (std::size_t i = 0; i < N; ++i) {
        std::cout << values[i] << '\n';
    }
}
```

`const int (&values)[N]` 是“数组的引用”，不会退化，长度 `N` 由模板推导。

## 5.4 `std::array`：有长度的数组

`std::array` 定义在 `<array>`，是固定长度的容器：

```cpp
std::array<int, 5> values = {5, 1, 4, 2, 3};

values.size();      // 5
values[0];          // 第一个元素
values.front();     // 第一个
values.back();      // 最后一个
values.data();      // 指向首元素的指针
```

它和原生数组的区别：

- 知道自己有多长；
- 可以直接赋值和比较（逐元素）；
- 支持 `begin()` / `end()`，可用于范围 `for` 和标准算法；
- 不会退化为指针；
- 作为函数参数时按值传递会复制整个数组。

`std::array` 通常是聚合类型，可以用花括号初始化：

```cpp
std::array<int, 3> point = {1, 2, 3};
auto [x, y, z] = point;   // C++17 结构化绑定
```

`std::array` 的大小是编译期常量，通常没有额外开销；它就是把原生数组包了一层。

## 5.5 `std::vector`：运行时长度

`std::vector` 定义在 `<vector>`，是动态数组：

```cpp
std::vector<int> values = {3, 1, 4};
values.push_back(1);
values.push_back(5);

values.size();       // 5
values.capacity();   // 至少 5，通常更大
values[0];           // 不检查边界
values.at(0);        // 检查边界，越界抛 std::out_of_range
values.data();       // 指向连续存储
```

关键点：

- 元素连续存放，`data()` 可以传给需要指针的 C 接口；
- `size()` 是元素个数，`capacity()` 是当前分配的存储能容纳的元素个数；
- `push_back` 可能重新分配，导致所有指针、引用、迭代器失效；
- `reserve(n)` 预分配容量，避免多次重新分配；
- `[]` 不检查边界，`at()` 检查并抛异常。

现代 C++ 里，需要动态长度数组时，默认用 `std::vector`；需要固定长度时，默认用 `std::array`。裸数组通常只用于底层接口、静态存储或性能极端敏感的场合。

## 5.6 指针：对象的地址

指针保存一个对象的地址：

```cpp
int value = 42;
int *pointer = &value;   // & 取地址
std::cout << *pointer;   // * 解引用，得到 value
```

`&` 和 `*` 是一对互逆操作：

```cpp
*&value == value
&*pointer == pointer   // 前提是 pointer 有效
```

指针的类型决定了“解引用时读多少字节、如何解释”：

- `int *` 指向 `int`，`*p` 得到一个 `int`；
- `double *` 指向 `double`，`*p` 得到一个 `double`；
- `void *` 可以保存任意对象地址，但不能直接解引用。

指针本身也是一个对象，占内存，可以取地址，也可以被 `const` 修饰。

## 5.7 指针算术

指针算术只在**同一个数组**（或一个对象视为长度 1 的数组）内有定义：

```cpp
int values[5] = {10, 20, 30, 40, 50};
int *p = values;

p + 1;      // 指向 values[1]
*(p + 2);   // values[2]
p[3];       // 等价于 *(p + 3)
p - values; // 5，类型是 std::ptrdiff_t
```

规则：

- `p + n` 指向“向后第 n 个元素”，不是第 n 个字节。实际地址偏移是 `n * sizeof(*p)`。
- 指针可以指向数组的**尾后位置**（one past the end），即 `values + 5`；可以形成和比较，但不能解引用。
- 两个指针相减，只有指向同一数组（或尾后）时才有定义。
- 越界指针算术是 UB，即使不解引用。
- `void *` 不能做算术；`char *` 的算术单位是 1 字节。

```cpp
int *end = values + 5;
for (int *cursor = values; cursor != end; ++cursor) {
    std::cout << *cursor << '\n';
}
```

用指针遍历数组是合法的，但现代 C++ 更推荐迭代器或范围 `for`，因为它们更不容易越界。

## 5.8 `const` 与指针、引用

`const` 和指针组合有四种情况：

```cpp
int value = 10;

const int *p1 = &value;        // 指向 const int：不能通过 p1 改值
int *const p2 = &value;        // const 指针：不能改指向
const int *const p3 = &value;  // 两者都不能改
int *p4 = &value;              // 都可以改
```

记忆方法：看 `const` 在 `*` 的哪一边。

- `const int *`：`const` 修饰 `int`，值不可改。
- `int *const`：`const` 修饰指针本身，指向不可改。
- `const int *const`：两者都不可改。

引用也有类似的区别，但引用本身不能重新绑定，所以只有“引用的对象是否 const”：

```cpp
int value = 10;
int &ref = value;             // 可修改 value
const int &cref = value;      // 不能通过 cref 修改
```

`const` 引用可以绑定临时对象，并延长其生命期到引用的作用域结束：

```cpp
const int &ref = 42;   // 合法
int &bad = 42;         // 错误
```

## 5.9 指针的指针、数组指针、指针数组

### 指针的指针

```cpp
int value = 7;
int *p = &value;
int **pp = &p;

**pp = 8;   // 修改 value
```

`pp` 指向“指向 int 的指针”。多级指针在 C 接口里常见（例如 `char **argv`），但在现代 C++ 里应尽量用引用或容器替代。

### 数组指针

```cpp
int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
int (*row)[3] = matrix;   // 指向“含 3 个 int 的数组”

row[1][2];   // 6
```

注意 `int (*row)[3]` 和 `int *row[3]` 的区别：

- `int (*row)[3]`：一个指针，指向长度为 3 的 `int` 数组。
- `int *row[3]`：一个数组，含 3 个 `int *`。

括号决定结合方式。`int *row[3]` 是“指针的数组”，`int (*row)[3]` 是“数组的指针”。

### 指针数组

```cpp
const char *names[] = {"alpha", "beta", "gamma"};
```

`names` 是一个数组，每个元素是 `const char *`。它常用于字符串表、命令行参数。

## 5.10 函数指针

函数也有地址，可以存进函数指针：

```cpp
int add(int left, int right)
{
    return left + right;
}

int (*operation)(int, int) = add;
int result = operation(2, 3);
```

函数指针的类型包含返回类型和参数列表：

```cpp
int (*p)(int, int);       // 指向“接收两个 int、返回 int”的函数
void (*q)();              // 指向“无参数、无返回值”的函数
```

函数指针可以作为参数，实现回调：

```cpp
int apply(int (*operation)(int, int), int left, int right)
{
    return operation(left, right);
}
```

不捕获任何变量的 lambda 可以转换成函数指针：

```cpp
int (*multiply)(int, int) = [](int left, int right) {
    return left * right;
};
```

捕获了变量的 lambda 不能转换，因为它有状态。需要保存状态时，用 `std::function` 或模板参数。

## 5.11 引用与指针的对比

| 维度 | 引用 | 指针 |
|------|------|------|
| 是否必须绑定 | 是 | 否，可为 `nullptr` |
| 能否重新绑定 | 否 | 是 |
| 是否占独立存储 | 通常不 | 是 |
| 能否做算术 | 否 | 是 |
| 能否有空值 | 否 | 是 |
| 解引用语法 | 直接用 | `*p` |
| 典型用途 | 参数、别名、返回值 | 可空关系、动态对象、底层操作 |

引用更像“对象的另一个名字”，指针更像“对象的地址值”。C++ 的很多接口用引用表达“必须有一个对象”，用指针表达“可能没有”。

## 5.12 `nullptr` 与 `void *`

`nullptr` 是 C++11 引入的空指针字面量，类型是 `std::nullptr_t`：

```cpp
int *p = nullptr;
if (p == nullptr) { ... }
```

它比 C 的 `NULL` 更安全：

- `nullptr` 有独立类型，不会和整数 `0` 混淆；
- 重载决议时，`f(nullptr)` 选 `f(std::nullptr_t)`，而不是 `f(int)`；
- 不能隐式转换成整数（除了 `bool`）。

`void *` 可以保存任意对象指针，但不能直接解引用：

```cpp
int value = 42;
void *raw = &value;
int *p = static_cast<int *>(raw);
std::cout << *p;
```

`void *` 常用于 C 接口、内存分配和类型擦除。它放弃了类型信息，使用时要格外小心。

## 5.13 二维数组

```cpp
int matrix[3][4] = {
    {1, 2, 3, 4},
    {5, 6, 7, 8},
    {9, 10, 11, 12},
};
```

二维数组在内存里仍然是连续的：先放第 0 行，再放第 1 行，以此类推。`matrix[i][j]` 等价于 `*(*(matrix + i) + j)`。

`matrix` 退化为指向第一行的指针，类型是 `int (*)[4]`，不是 `int **`。这是一个常见误解：

```cpp
void f(int matrix[3][4]);   // 参数类型是 int (*)[4]
void g(int **matrix);       // 完全不同，不能接收二维数组
```

需要动态二维数组时，可以用 `std::vector<std::vector<int>>`、一维 `std::vector` 加索引计算，或 `std::array` 的嵌套。

## 5.14 代码导览

本章有两个程序：

- [`arrays.cpp`](arrays.cpp)：原生数组、数组退化、`std::array`、`std::vector`、二维数组、结构化绑定。
- [`pointers.cpp`](pointers.cpp)：取地址、解引用、指针算术、`const` 指针、指针的指针、数组指针、函数指针、`nullptr`。

运行：

```sh
make 05-arrays-pointers/arrays 05-arrays-pointers/pointers
./05-arrays-pointers/arrays
./05-arrays-pointers/pointers
```

`arrays.cpp` 里的 `length` 模板用数组引用推导长度，避免退化；`pointers.cpp` 里的 `end - begin` 和字节距离展示了指针算术的单位差异。

## 5.15 常见陷阱

| 写法 | 问题 | 建议 |
|------|------|------|
| `sizeof(array)` 在函数参数里 | 数组已退化为指针 | 传长度或用 `std::array` |
| `int a[5]; a[5]` | 越界，UB | 用 `at()` 或检查索引 |
| 指针越界后再比较 | 只有同数组（含尾后）才有定义 | 保持在数组范围内 |
| `int *p = 0;` | 可读性差，重载可能选错 | 用 `nullptr` |
| `int *p; *p = 1;` | 未初始化指针，UB | 初始化为 `nullptr` 或有效地址 |
| 返回局部数组的指针 | 悬垂 | 返回容器或动态对象 |
| `int **p = matrix;` | 二维数组不是 `int **` | 用 `int (*p)[N]` |
| `int *a[3]` 与 `int (*a)[3]` 混淆 | 结合方式不同 | 用 `using` 别名拆开 |
| `vector` 扩容后继续用旧指针 | 迭代器/指针失效 | 扩容后重新取 `data()` |
| 对 `void *` 解引用 | 不允许 | 先转回具体类型 |

## 5.16 语法卡片

| 术语 | 一句话定义 | 本章例子 |
|------|------------|----------|
| 数组 | 同类型、连续存放的对象 | `int a[5];` |
| 退化 | 数组名转成指向首元素的指针 | `int *p = a;` |
| `std::array` | 固定长度、知道大小的容器 | `std::array<int, 5>` |
| `std::vector` | 运行时长度、连续存储的容器 | `std::vector<int>` |
| 指针 | 保存对象地址的对象 | `int *p = &x;` |
| 解引用 | 通过指针访问对象 | `*p` |
| 指针算术 | 按元素大小移动指针 | `p + 1` |
| `const` 指针 | 指向或本身不可修改的指针 | `const int *`、`int *const` |
| 数组指针 | 指向整个数组的指针 | `int (*p)[3]` |
| 函数指针 | 指向函数的指针 | `int (*p)(int, int)` |
| `nullptr` | 类型安全的空指针字面量 | `int *p = nullptr;` |
| 结构化绑定 | 把聚合/容器拆成多个名字 | `auto [a, b] = pair;` |

## 5.17 练习

1. **数组长度**：写一个模板函数，接收数组引用并返回长度；再用指针参数版本对比，观察 `sizeof` 的差异。
2. **数组复制**：用 `std::copy` 把一个原生数组复制到另一个；再用 `std::array` 直接赋值，比较两种写法。
3. **指针遍历**：用指针和范围 `for` 分别遍历同一个数组，输出相同的结果。
4. **`const` 练习**：声明 `const int *`、`int *const`、`const int *const`，分别尝试改值和改指向，记录哪些编译错误。
5. **二维数组**：写一个函数接收 `int (*matrix)[4]`，打印所有元素；再尝试用 `int **` 接收，读编译错误。
6. **函数指针**：写 `apply`，分别传入普通函数、不捕获的 lambda、捕获的 lambda（用 `std::function`），比较三种写法。
7. **容器失效**：创建一个 `std::vector<int>`，保存 `data()` 指针，`push_back` 多次直到扩容，观察指针是否变化（不要解引用旧指针）。
8. **破坏实验**：把 `int values[5]` 改成 `int values[5]{}`，再越界访问 `values[5]`，用 AddressSanitizer 运行，读诊断。

## 5.18 下一章

**第 6 章 · 字符、字符串与视图**

数组和指针是底层表示，字符串是它们最常见的应用。下一章会讲 `char`、C 风格字符串、`std::string`、`std::string_view`、原始字符串、数值转换和 UTF-8。

届时，`std::string` 为什么能自动管理内存、`std::string_view` 为什么不能随便返回，会有完整的答案。
