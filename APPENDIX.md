# 附录 A · C++20/23 速览

> 主线以 ISO C++17 为基线。本附录补齐几个 C++20/23 的重要特性。附录中的代码是示意，不进入 `make` 构建；这些特性需要支持对应标准的编译器和标准库，不能用本仓库的 C++17 配置验证。

C++20 是 C++11 之后最大的一次更新：concepts、ranges、coroutines、modules、三路比较、`constexpr` 扩展、`std::format`、`std::span`、`std::jthread` 等。C++23 在此基础上补了 `std::expected`、`std::print`、`std::mdspan`、`if consteval`、显式对象参数等。

本附录只给“这是什么、解决什么问题、代码长什么样”，不展开实现细节。

## A.1 Concepts：给模板加约束

C++17 的 SFINAE（第 9 章）能约束模板，但错误信息难读。C++20 的 **concept** 把约束写成可读的谓词：

```cpp
#include <concepts>

template <std::integral T>
T twice(T value)
{
    return value * 2;
}

template <typename T>
requires std::floating_point<T>
T half(T value)
{
    return value / 2;
}
```

也可以自定义 concept：

```cpp
template <typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::same_as<T>;
};

template <Addable T>
T add(T a, T b)
{
    return a + b;
}
```

`requires` 表达式检查“这些表达式是否合法”。concept 可以组合：

```cpp
template <typename T>
concept Number = std::integral<T> || std::floating_point<T>;
```

好处：

- 约束写在接口上，读者一眼能看出要求；
- 错误信息更短，直接说“T 不满足 std::integral”；
- 重载决议可以用 concept 排序，比 SFINAE 清晰；
- 可以和 `auto` 结合：`void f(std::integral auto x);`

## A.2 Ranges：把算法和视图组合起来

C++20 的 `<ranges>` 提供了范围版本的算法和惰性视图：

```cpp
#include <ranges>
#include <vector>
#include <algorithm>

std::vector<int> values = {5, 1, 4, 2, 3};

std::ranges::sort(values);

auto even = values
          | std::views::filter([](int x) { return x % 2 == 0; })
          | std::views::transform([](int x) { return x * x; });

for (int value : even) {
    std::cout << value << ' ';
}
```

`views` 是惰性的：`filter` 和 `transform` 不会立刻创建新容器，而是在迭代时计算。管道 `|` 让组合更接近自然语言。

常见视图：

| 视图 | 作用 |
|------|------|
| `views::filter` | 过滤元素 |
| `views::transform` | 转换元素 |
| `views::take` / `views::drop` | 取前 n 个 / 跳过前 n 个 |
| `views::reverse` | 反转 |
| `views::iota` | 生成整数序列 |
| `views::split` | 按分隔符切分 |
| `views::join` | 展平嵌套范围 |

注意视图的生命期：视图通常不拥有底层数据，底层容器销毁后不能继续使用。

## A.3 Coroutines：可暂停的函数

C++20 的协程允许函数在中途暂停和恢复：

```cpp
#include <coroutine>

Task<int> compute()
{
    int value = co_await read_async();
    co_return value * 2;
}
```

三个关键字：

- `co_await`：等待一个 awaitable；
- `co_yield`：产生一个值并暂停（生成器）；
- `co_return`：返回值并结束。

协程不是“自带调度器”的并发原语；它只提供暂停/恢复的机制，调度由库或框架负责。标准库在 C++20 只提供了底层设施（`coroutine_handle`、`suspend_always` 等），没有现成的 `Task` 或 `Generator`；C++23 的 `std::generator` 补了一个常用类型。

协程的难点在生命周期和分配：协程帧可能堆分配，`co_await` 的对象必须存活到恢复。初学阶段先理解“可暂停的函数”，不要把它当成线程。

## A.4 Modules：替代头文件的方向

C++20 的模块用 `import` 代替 `#include`：

```cpp
// math.cppm
export module math;

export int add(int a, int b)
{
    return a + b;
}
```

```cpp
// main.cpp
import math;
#include <iostream>

int main()
{
    std::cout << add(1, 2);
}
```

目标：

- 消除头文件的文本复制；
- 减少宏污染；
- 加快编译（模块接口可以被预编译）；
- 让“接口”和“实现”的边界更清晰。

现实：

- 编译器支持程度不一；
- 构建系统需要显式处理模块依赖；
- 与现有头文件库的互操作仍有摩擦；
- `import std;`（标准库模块）的支持更晚。

本教材仍然用头文件，因为它是 C++17 的基线，也是目前绝大多数项目的现实。

## A.5 三路比较 `<=>`

C++20 引入飞船运算符：

```cpp
#include <compare>

struct Point {
    int x;
    int y;

    auto operator<=>(const Point &) const = default;
};
```

编译器会自动生成 `==`、`!=`、`<`、`<=`、`>`、`>=`，按成员顺序比较。

返回类型：

- `std::strong_ordering`：可替换性（相等意味着不可区分）；
- `std::weak_ordering`：等价但可区分（例如忽略大小写的字符串）；
- `std::partial_ordering`：可能存在不可比较的值（例如浮点 NaN）。

```cpp
std::strong_ordering result = a <=> b;

if (result < 0) { /* a < b */ }
if (result == 0) { /* a == b */ }
```

`= default` 要求所有成员都支持 `<=>`。对浮点成员，默认生成的是 `partial_ordering`。

## A.6 指定初始化器

C++20 支持 C 风格的指定初始化：

```cpp
struct Config {
    int width;
    int height;
    bool fullscreen;
};

Config config{
    .width = 800,
    .height = 600,
    .fullscreen = false
};
```

规则：

- 指示符必须按成员声明顺序出现；
- 不能跳过成员后再回头；
- 不能和普通的位置初始化混用；
- 只适用于聚合类型。

它让配置结构体的初始化更可读，尤其是成员很多、类型相同的时候。

## A.7 `consteval` 与 `constinit`

`consteval` 强制函数在编译期求值：

```cpp
consteval int square(int value)
{
    return value * value;
}

constexpr int a = square(5);   // 可以
int b = square(runtime_value); // 错误：必须在编译期
```

`constinit` 保证变量在编译期初始化，但允许运行期修改：

```cpp
constinit int counter = 0;

int main()
{
    ++counter;   // 可以
}
```

它解决的是“静态初始化顺序问题”：`constinit` 变量不会参与动态初始化，因此在 `main` 之前就已经处于确定状态。

## A.8 `std::span`：连续内存的视图

`std::span<T>` 是 C++20 的连续内存视图，类似 `std::string_view` 但面向任意类型：

```cpp
#include <span>

void print(std::span<const int> values)
{
    for (int value : values) {
        std::cout << value << ' ';
    }
}

int array[] = {1, 2, 3};
std::vector<int> vector = {4, 5, 6};

print(array);    // 不需要传长度
print(vector);   // vector 隐式转换
```

特点：

- 不拥有数据；
- 知道长度；
- 可以表示数组、`std::vector`、`std::array` 的连续区间；
- 可以是有界的 `std::span<T, N>` 或动态长度的 `std::span<T>`。

它填补了“函数参数需要指针 + 长度”的空白，比裸指针安全，比 `vector` 轻量。

## A.9 `char8_t` 与 UTF-8

C++20 引入 `char8_t`，让 UTF-8 字符串有独立类型：

```cpp
const char8_t *text = u8"你好";
std::u8string s = u8"你好";
```

C++17 的 `u8"..."` 是 `const char[]`，C++20 变成 `const char8_t[]`。这提高了类型安全，但也带来兼容性问题：

- `std::u8string` 不能直接传给 `std::string`；
- C 接口通常要 `const char *`；
- 需要显式转换（`reinterpret_cast` 或逐字节复制）。

如果项目以 C++17 为主，继续用 `std::string` 存 UTF-8 是常见做法；升级到 C++20 时要评估接口边界。

## A.10 `std::format`

C++20 的 `std::format` 提供类型安全的格式化：

```cpp
#include <format>
#include <iostream>

std::cout << std::format("{} + {} = {}\n", 1, 2, 3);
std::cout << std::format("{:08.2f}\n", 3.14159);
```

它结合了 Python 风格的 `{}` 和 C++ 的类型安全：

- 不需要 `%d` / `%s` 手动匹配类型；
- 支持位置参数、宽度、精度、进制；
- 编译期检查格式串（实现支持时）；
- 可以和用户自定义类型集成（`std::formatter`）。

C++23 的 `std::print` 进一步简化输出：

```cpp
std::print("{} + {} = {}\n", 1, 2, 3);
```

在 `std::format` 广泛可用之前，`std::ostringstream` 仍然是 C++17 的可移植选择。

## A.11 C++23 速览

C++23 是一个“补丁 + 实用工具”版本，重点包括：

### `std::expected`

用类型表达“成功值或错误”：

```cpp
std::expected<int, std::string> parse(std::string_view text);
```

它比异常更轻，比 `optional` 多了错误信息。适合错误是“预期内”的接口。

### `std::print` / `std::println`

```cpp
std::println("hello, {}", name);
```

基于 `std::format`，直接输出到 `stdout`，不需要先构造字符串。

### `std::mdspan`

多维数组视图：

```cpp
std::mdspan<double, std::extents<std::size_t, 3, 4>> matrix(data);
matrix[1, 2] = 3.14;
```

它不拥有数据，只提供多维索引，适合数值计算。

### `if consteval`

在 `constexpr` 函数里判断“当前是否在编译期求值”：

```cpp
constexpr int f(int x)
{
    if consteval {
        return x * 2;   // 编译期路径
    } else {
        return compute(x); // 运行期路径
    }
}
```

### 显式对象参数（deducing this）

```cpp
struct Widget {
    template <typename Self>
    void process(this Self &&self);
};
```

它允许成员函数显式接收对象类型，简化 CRTP、转发和 const/非 const 重载。

### 多维下标

```cpp
matrix[1, 2] = 3.14;
```

C++23 允许 `operator[]` 接收多个参数，配合 `std::mdspan` 使用。

### 其他

- `std::flat_map` / `std::flat_set`：基于连续存储的关联容器；
- `std::generator`：协程生成器；
- `std::stacktrace`：栈回溯；
- `std::byteswap`：字节序转换；
- `std::to_underlying`：枚举转底层类型；
- `std::unreachable`：标记不可达代码。

C++23 的库支持比语言特性更晚，使用前检查编译器和标准库版本。

## A.12 从 C++17 往哪走

如果你已经掌握本教材的 C++17 内容，下一步可以按这个顺序扩展：

1. **标准库**：容器、算法、迭代器、智能指针、文件系统、正则、并发；
2. **C++20**：concepts、ranges、`std::span`、`std::format`、协程；
3. **工程**：CMake、包管理、测试框架、静态分析、Sanitizer；
4. **底层**：ABI、对象布局、异常实现、内存模型、无锁编程；
5. **领域**：图形、网络、嵌入式、数值计算、编译器。

C++ 的边界很宽，但语言本身的核心规则并不多。把第 0–11 章的对象模型、值类别、RAII、拷贝/移动、模板和 UB 握稳，后面的库和框架都只是这些规则的不同组合。

> 标准不是建议，是合同。读得懂合同，才谈得上自由。
