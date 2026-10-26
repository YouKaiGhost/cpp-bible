# 第 9 章 · 模板与泛型

> **卷一 · 语言**　|　前置知识：第 1–8 章　|　建议用时：65 分钟

> 模板不是“带类型的宏”，而是编译器在编译期执行的一套类型推导与代码生成规则。

链接解决“多个翻译单元如何拼在一起”，模板解决“同一段代码如何适配多个类型”。这一章讲函数模板、类模板、类型推导、特化、可变参数模板、折叠表达式、`if constexpr` 和 SFINAE。

模板是 C++ 最强大也最复杂的语言特性之一。本章不追求覆盖所有角落，而是建立一套能读懂标准库、能写出简单泛型代码的直觉。

## 9.1 本章语法地图

| 语法 | 本章出现的位置 |
|------|----------------|
| 函数模板 | `template <typename T> T max_value(T, T);` |
| 类模板 | `template <typename T, std::size_t N> class FixedArray;` |
| 非类型模板参数 | `FixedArray<int, 5>`、`Factorial<5>` |
| 模板参数推导 | `max_value(3, 7)` 推导 `T = int` |
| 全特化 | `template <> struct TypeName<int>` |
| 可变参数模板 | `template <typename... Args>` |
| 折叠表达式 | `(args + ...)`、`((std::cout << args), ...)` |
| `if constexpr` | 编译期分支 |
| 类型特征 | `std::is_integral_v<T>` |
| SFINAE | `std::enable_if_t<...>` |
| `static_assert` | 编译期断言 |

## 9.2 为什么需要模板

假设你要写一个“取两个值中较大者”的函数。C 里常见做法是宏：

```cpp
#define MAX(a, b) ((a) > (b) ? (a) : (b))
```

宏有几个问题：

- 没有类型检查；
- 参数可能被求值多次；
- 调试器看不到；
- 名字没有作用域。

C++ 的函数模板：

```cpp
template <typename T>
T max_value(const T &left, const T &right)
{
    return left < right ? right : left;
}
```

调用：

```cpp
max_value(3, 7);       // T = int
max_value(2.5, 1.5);   // T = double
```

编译器会为 `int` 和 `double` 各生成一份函数，分别做类型检查。这就是**编译期多态**：同一份源码，针对不同类型生成不同的机器码。

模板和运行期多态（虚函数）的区别：

| 维度 | 模板 | 虚函数 |
|------|------|--------|
| 绑定时机 | 编译期 | 运行期 |
| 生成代码 | 每个类型一份 | 通常一份 |
| 类型要求 | 编译期可见 | 通过基类接口 |
| 运行时开销 | 通常无 | 虚表/间接调用 |
| 适用场景 | 泛型算法、容器 | 运行时多态对象 |

## 9.3 函数模板与类型推导

```cpp
template <typename T>
T max_value(const T &left, const T &right)
{
    return left < right ? right : left;
}
```

`template <typename T>` 声明一个类型参数。`typename` 和 `class` 在这里等价；`typename` 更明确表示“这是一个类型”。

调用时，编译器从实参推导 `T`：

```cpp
max_value(3, 7);       // T = int
max_value(2.5, 1.5);   // T = double
```

推导规则和 `auto` 类似：

- 按值参数会丢掉顶层 `const` 和引用；
- `const T &` 会保留被引用对象的 const；
- 数组实参会退化为指针（按值）或绑定到数组引用（`const T &` 可能推导出数组类型）。

如果两个实参推导出的 `T` 不一致，推导失败：

```cpp
max_value(3, 7.5);   // 错误：T 同时被推导为 int 和 double
```

可以显式指定模板参数：

```cpp
max_value<double>(3, 7.5);   // T = double，3 转成 3.0
```

或者把模板设计成接受两个类型：

```cpp
template <typename T, typename U>
auto max_value(T left, U right)
{
    return left < right ? right : left;
}
```

返回类型用 `auto`，让编译器推导共同类型。注意 `left < right ? right : left` 的条件运算符也会做类型转换，结果类型可能不是你以为的那个。

### 模板的定义必须可见

与普通函数不同，模板的定义通常必须放在头文件里，或者在使用它的翻译单元里可见。原因是编译器在实例化时需要看到完整定义，而不能只看到声明。

```cpp
// max.h
template <typename T>
T max_value(const T &, const T &)
{
    // 定义必须在这里
}
```

如果把模板定义放在 `.cpp` 里，另一个 `.cpp` 调用它时，编译器只能看到声明，无法实例化，链接时可能报 `undefined reference`。解决办法：

- 把定义放头文件；
- 在 `.cpp` 里显式实例化：`template int max_value<int>(const int &, const int &);`
- 使用 C++20 的模块（modules）。

## 9.4 类模板与非类型参数

类模板把类型参数化：

```cpp
template <typename T, std::size_t N>
class FixedArray {
public:
    constexpr std::size_t size() const { return N; }
    T &operator[](std::size_t index) { return data_[index]; }
    const T &operator[](std::size_t index) const { return data_[index]; }

private:
    T data_[N]{};
};
```

使用：

```cpp
FixedArray<int, 5> values;
FixedArray<double, 3> coordinates;
```

`N` 是**非类型模板参数**，必须是编译期常量。C++17 允许的非类型参数包括：

- 整数类型；
- 枚举类型；
- 指针/引用（指向具有静态存储期、具有内部或外部链接的对象/函数）；
- `std::nullptr_t`；
- 某些类型的 `constexpr` 值。

C++20 又允许浮点数和某些类类型作为非类型参数。

非类型参数让模板可以表达“长度是类型的一部分”：

```cpp
template <int N>
struct Factorial {
    static constexpr unsigned long long value =
        static_cast<unsigned long long>(N) * Factorial<N - 1>::value;
};

template <>
struct Factorial<0> {
    static constexpr unsigned long long value = 1;
};

static_assert(Factorial<5>::value == 120);
```

`FixedArray` 的大小在编译期确定，通常没有动态分配，也没有额外开销。

### 类模板的成员函数

类模板的成员函数在类外定义时，要重复模板参数：

```cpp
template <typename T, std::size_t N>
constexpr std::size_t FixedArray<T, N>::size() const
{
    return N;
}
```

如果成员函数返回依赖模板参数的类型，需要用 `typename` 告诉编译器“这是一个类型”：

```cpp
template <typename T>
typename T::value_type first(const T &container)
{
    return *container.begin();
}
```

没有 `typename`，编译器默认把 `T::value_type` 当成静态成员，解析失败。这是“依赖名”（dependent name）的经典坑。

### CTAD（C++17）

类模板参数推导（Class Template Argument Deduction）允许省略模板参数：

```cpp
std::pair p{1, 2};              // std::pair<int, int>
std::vector v{1, 2, 3};         // std::vector<int>
std::array a{1, 2, 3};          // std::array<int, 3>
```

编译器从构造函数实参推导模板参数。自定义类模板也可以提供**推导指引**（deduction guide）：

```cpp
template <typename T>
FixedArray(T, T) -> FixedArray<T, 2>;
```

CTAD 让代码更简洁，但也可能让类型变得不直观。库代码通常显式写模板参数，教学代码可以适当使用。

## 9.5 特化

模板特化是针对某个具体类型给出不同实现。

### 全特化

```cpp
template <typename T>
struct TypeName {
    static const char *get() { return "unknown"; }
};

template <>
struct TypeName<int> {
    static const char *get() { return "int"; }
};
```

`TypeName<int>` 使用特化版本，其他类型使用主模板。

### 偏特化

偏特化只针对部分模板参数：

```cpp
template <typename T>
struct TypeName<T *> {
    static const char *get() { return "pointer"; }
};
```

`TypeName<int *>` 会选偏特化。偏特化只能用于类模板，不能用于函数模板；函数模板可以通过重载实现类似效果。

### 特化的匹配顺序

- 全特化优先于偏特化；
- 偏特化优先于主模板；
- 多个偏特化都可能匹配时，选“更特殊”的那个；无法区分则二义。

特化是编译期分派的一种手段，但容易让代码分散。现代 C++ 更常用 `if constexpr`、概念（concepts）和重载来替代部分特化。

## 9.6 可变参数模板与折叠表达式

可变参数模板接受任意数量的类型参数：

```cpp
template <typename... Args>
void print_all(const Args &...args)
{
    ((std::cout << args << ' '), ...);
    std::cout << '\n';
}
```

- `typename... Args` 是**参数包**（parameter pack）。
- `const Args &...args` 是函数参数包。
- `sizeof...(Args)` 得到参数个数。
- `(expr, ...)` 是 C++17 的**折叠表达式**，把参数包展开成一个表达式。

调用：

```cpp
print_all(1, 2.5, "three", '4');
print_all();   // 空包也合法
```

### 折叠表达式的四种形式

| 形式 | 含义 |
|------|------|
| `(pack op ...)` | 一元右折叠：`a op (b op (c op d))` |
| `(... op pack)` | 一元左折叠：`((a op b) op c) op d` |
| `(init op ... op pack)` | 二元右折叠 |
| `(pack op ... op init)` | 二元左折叠 |

例子：

```cpp
template <typename... Args>
auto sum_all(Args... args)
{
    return (args + ...);   // 一元右折叠
}

sum_all(1, 2, 3);          // 6
```

空包的 `(args + ...)` 不合法（没有初始值），需要写成 `(args + ... + 0)`。`print_all` 用的是逗号折叠，空包是合法的。

### 递归展开

C++17 之前，可变参数模板通常用递归展开：

```cpp
void print_all() { }

template <typename T, typename... Rest>
void print_all(const T &first, const Rest &...rest)
{
    std::cout << first << ' ';
    print_all(rest...);
}
```

C++17 的折叠表达式让很多场景不再需要递归。但递归在类型处理、元编程里仍然常见。

## 9.7 `if constexpr` 与类型特征

`if constexpr` 是 C++17 的编译期分支：

```cpp
template <typename T>
std::string classify(T)
{
    if constexpr (std::is_integral_v<T>) {
        return "integral";
    } else if constexpr (std::is_floating_point_v<T>) {
        return "floating point";
    } else {
        return "other";
    }
}
```

和普通 `if` 的区别：

- 条件必须是编译期常量表达式；
- 未被选中的分支**不会被实例化**；
- 因此可以在不同分支里写只对某些类型合法的代码。

```cpp
template <typename T>
void print(const T &value)
{
    if constexpr (std::is_pointer_v<T>) {
        if (value != nullptr) {
            std::cout << *value;
        }
    } else {
        std::cout << value;
    }
}
```

如果这里用普通 `if`，`*value` 对非指针类型也会参与编译，导致错误。`if constexpr` 把不合法分支丢弃。

### 常用类型特征

| 特征 | 含义 |
|------|------|
| `std::is_integral_v<T>` | 是否整数类型 |
| `std::is_floating_point_v<T>` | 是否浮点类型 |
| `std::is_pointer_v<T>` | 是否指针 |
| `std::is_reference_v<T>` | 是否引用 |
| `std::is_const_v<T>` | 是否 const |
| `std::is_same_v<T, U>` | 两个类型是否相同 |
| `std::is_base_of_v<Base, Derived>` | 是否继承关系 |
| `std::is_constructible_v<T, Args...>` | 是否可由 Args 构造 |
| `std::remove_reference_t<T>` | 去掉引用 |
| `std::decay_t<T>` | 退化（去引用、去 const、数组转指针） |

类型特征本身是模板，`_v` 后缀是 C++17 的变量模板，等价于 `::value`。

## 9.8 SFINAE 与 `enable_if`

SFINAE 是 “Substitution Failure Is Not An Error” 的缩写：模板参数替换失败时，编译器不报错，而是把这个候选从重载集合里去掉。

经典用法：

```cpp
template <typename T>
std::enable_if_t<std::is_integral_v<T>, T> twice(T value)
{
    return static_cast<T>(value * 2);
}
```

- 如果 `T` 是整数，`std::enable_if_t<true, T>` 是 `T`，函数参与重载；
- 如果 `T` 不是整数，`std::enable_if_t<false, T>` 替换失败，函数被移除；
- 调用 `twice(2.5)` 会找不到匹配的函数。

`std::enable_if` 还可以放在返回类型、参数类型或模板参数里。它可读性一般，错误信息也很长。

C++20 的 **concepts** 提供了更直接的写法：

```cpp
template <std::integral T>
T twice(T value)
{
    return value * 2;
}
```

本章以 C++17 为基线，只要求能读懂 `enable_if`；附录会介绍 concepts。

## 9.9 模板的错误信息

模板错误通常很长，因为编译器会打印：

- 实例化栈：哪个模板、哪组参数、哪一行触发；
- 候选函数和推导失败原因；
- 最终的错误位置。

阅读顺序：

1. 先找 `required from here` / `in instantiation of`，定位真正触发实例化的代码；
2. 再看第一条 `error`，而不是最后一条；
3. 看模板参数被推导成了什么；
4. 如果涉及 `enable_if`，看哪个约束失败。

一个实用技巧：在模板里加 `static_assert`，把错误提前到更可读的位置：

```cpp
template <typename T>
void process(T value)
{
    static_assert(std::is_copy_constructible_v<T>,
                  "T must be copy constructible");
    // ...
}
```

`static_assert` 的条件必须是编译期常量，第二个参数是错误信息。

## 9.10 代码导览

[`templates.cpp`](templates.cpp) 分成五部分：

- `show_function_templates()`：函数模板、类型推导、显式模板参数；
- `show_class_templates()`：`FixedArray<T, N>`、非类型参数、范围遍历；
- `show_specialization()`：`TypeName` 的全特化、`Factorial` 的编译期递归；
- `show_variadic()`：可变参数模板、逗号折叠、加法折叠；
- `show_type_traits()`：`if constexpr`、类型特征、`enable_if`。

运行：

```sh
make 09-templates/templates
./09-templates/templates
```

`FixedArray` 的 `data_[N]{}` 会把元素值初始化；`print_container` 是另一个函数模板，接受任何支持范围遍历的容器。

## 9.11 常见陷阱

| 写法 | 问题 | 建议 |
|------|------|------|
| 模板定义放在 `.cpp` | 其他翻译单元无法实例化 | 放头文件或显式实例化 |
| `max_value(3, 7.5)` | 两个实参推导出不同 `T` | 显式指定 `T` 或使用两个类型参数 |
| 依赖名不加 `typename` | 编译器不知道它是类型 | `typename T::value_type` |
| 模板里用普通 `if` 处理类型分支 | 不合法分支仍参与编译 | 用 `if constexpr` |
| `enable_if` 条件写反 | 函数被意外移除 | 用 `static_assert` 或 concepts 辅助 |
| 特化与主模板行为不一致 | 难以维护 | 保持语义契约 |
| 空参数包做 `(args + ...)` | 没有初始值，编译错误 | `(args + ... + 0)` |
| 模板错误只看最后一行 | 真正原因在实例化栈上方 | 从第一条 `error` 和 `required from` 看 |
| 在头文件里写非 `inline` 函数模板的特化 | 可能重复定义 | 用 `inline` 或放 `.cpp` |

## 9.12 语法卡片

| 术语 | 一句话定义 | 本章例子 |
|------|------------|----------|
| 函数模板 | 参数化类型的函数 | `template <typename T> T f(T);` |
| 类模板 | 参数化类型的类 | `FixedArray<T, N>` |
| 模板参数推导 | 从实参推断模板参数 | `max_value(3, 7)` |
| 非类型参数 | 编译期值作为模板参数 | `FixedArray<int, 5>` |
| 全特化 | 为具体类型提供实现 | `template <> struct TypeName<int>` |
| 偏特化 | 为部分参数模式提供实现 | `TypeName<T *>` |
| 参数包 | 任意数量的类型/值 | `typename... Args` |
| 折叠表达式 | 把参数包折叠成一个表达式 | `(args + ...)` |
| `if constexpr` | 编译期分支 | `if constexpr (is_integral_v<T>)` |
| 类型特征 | 编译期查询类型属性 | `std::is_pointer_v<T>` |
| SFINAE | 替换失败不算错误 | `enable_if_t<...>` |
| CTAD | 类模板参数推导 | `std::pair p{1, 2};` |

## 9.13 练习

1. **max**：写一个函数模板 `max_value`，支持 `int`、`double`、`std::string`；再写一个接受两个不同类型的版本。
2. **FixedArray**：给 `FixedArray` 加 `begin()` / `end()`，用范围 `for` 遍历；再加一个 `fill(value)`。
3. **特化**：为 `TypeName<bool>`、`TypeName<std::string>` 写全特化；为 `TypeName<T *>` 写偏特化，打印“pointer”。
4. **编译期阶乘**：用 `Factorial<N>` 计算 0 到 10 的阶乘，用 `static_assert` 验证几个值。
5. **可变参数**：写 `sum_all`，支持空包（返回 0）和混合类型；再写 `print_all`，用逗号折叠输出每个参数。
6. **`if constexpr`**：写一个 `describe` 模板，对整数、浮点、指针、其他类型返回不同字符串。
7. **SFINAE**：用 `enable_if` 写一个只接受整数类型的 `twice`；尝试传入 `double`，读错误信息。
8. **错误阅读**：故意在 `FixedArray` 里写一个类型错误，记录编译器输出的前三条信息，找出真正触发实例化的位置。

## 9.14 下一章

**第 10 章 · 未定义行为与可移植性**

模板让代码适配多种类型，但标准并不保证所有类型、所有平台、所有操作都有定义。下一章会讲未定义行为、实现定义、未指定行为、对象表示、对齐、严格别名和可移植性清单。

届时，为什么有符号溢出是 UB、为什么 `memcpy` 可以观察对象表示、为什么 `reinterpret_cast` 容易踩坑，会有完整的答案。
