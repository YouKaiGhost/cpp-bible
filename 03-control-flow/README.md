# 第 3 章 · 控制流

> **卷一 · 语言**　|　前置知识：第 2 章　|　建议用时：40 分钟

> 程序不是一条直线。控制流决定哪条线被走到，走多少次，什么时候离开。

表达式负责算出值，控制流负责决定“算哪一个”。这一章讲 `if`、`switch`、`while`、`do-while`、`for`、范围 `for`，以及 `break`、`continue`、`goto`。

C++ 的控制流表面上和 C 很像，但多了几样东西：C++17 的初始化语句、范围 `for`、`[[fallthrough]]` 属性，以及更严格的作用域规则。

## 3.1 本章语法地图

| 语法 | 本章出现的位置 |
|------|----------------|
| `if` / `else` | `if (value > 0) { ... }` |
| 带初始化的 `if` | `if (int n = value % 2; n == 0) { ... }` |
| `while` | `while (i < 3) { ... }` |
| `do-while` | `do { ... } while (j < 3);` |
| `for` | `for (int k = 0; k < 3; ++k) { ... }` |
| 范围 `for` | `for (int value : values) { ... }` |
| `switch` / `case` | `switch (command) { case ...: }` |
| `enum class` | `enum class Command { ... };` |
| `break` / `continue` | 跳出循环、跳过本轮 |
| `goto` / 标签 | `goto retry;` |
| 属性 | `[[fallthrough]]` |

## 3.2 `if`：让程序做选择

最简单的形式：

```cpp
if (condition) {
    // condition 为 true 时执行
}
```

带 `else`：

```cpp
if (value > 0) {
    std::cout << "positive\n";
} else if (value < 0) {
    std::cout << "negative\n";
} else {
    std::cout << "zero\n";
}
```

条件会被转换成 `bool`。整数 0 转成 `false`，非 0 转成 `true`；指针的 `nullptr` 转成 `false`，其他指针转成 `true`。这意味着下面两种写法都合法：

```cpp
if (pointer) { ... }        // pointer 非空
if (pointer != nullptr) { ... } // 更明确
```

C++ 允许在条件里声明变量，但它的作用域会泄漏到整个 `if` 语句：

```cpp
if (int n = compute(); n > 0) {
    // n 在这里可见
} else {
    // n 在这里也可见
}
// n 在这里不可见
```

C++17 进一步提供了**带初始化的 if**：

```cpp
if (int remainder = value % 2; remainder == 0) {
    // remainder 只在 if 和 else 里可见
} else {
    // remainder 在这里也可见
}
```

分号前是初始化语句，分号后是条件。它适合“先算一个值，再根据它判断”的场景，避免在外部作用域留下一个只用一次的变量。

### `if` 的常见错误

- **把 `=` 写成 `==` 的反面**：`if (x = 1)` 是赋值，条件恒为真。现代编译器通常警告，但不要依赖。
- **在条件后加分号**：`if (x > 0);` 是一个空语句，后面的块会无条件执行。
- **悬垂 `else`**：`else` 总是和最近的未配对 `if` 结合。必要时加花括号。
- **浮点相等比较**：`if (a == 0.1)` 通常不是你想要的行为。用容差比较。

## 3.3 `while`：先判断，再循环

```cpp
int i = 0;
while (i < 3) {
    std::cout << i << '\n';
    ++i;
}
```

执行顺序：

1. 判断条件；
2. 条件为真，执行循环体；
3. 回到步骤 1；
4. 条件为假，退出。

如果第一次条件就为假，循环体一次也不执行。

`while` 适合“不知道要循环多少次，只知道什么时候停”的场景，例如读取输入：

```cpp
int value;
while (std::cin >> value) {
    std::cout << value << '\n';
}
```

这里 `std::cin >> value` 返回流对象的引用，流在布尔上下文中会检查状态：读取成功为 `true`，失败或到达文件末尾为 `false`。

## 3.4 `do-while`：至少执行一次

```cpp
int j = 0;
do {
    std::cout << j << '\n';
    ++j;
} while (j < 3);
```

执行顺序：

1. 执行循环体；
2. 判断条件；
3. 条件为真，回到步骤 1；
4. 条件为假，退出。

循环体至少执行一次。注意 `while (...)` 后面有分号，这是 `do-while` 语法的一部分。

适合“先做一次，再决定要不要继续”的场景，例如菜单循环、输入重试。

## 3.5 `for`：把三件事写在一行

```cpp
for (int k = 0; k < 3; ++k) {
    std::cout << k << '\n';
}
```

三个部分：

1. **初始化**：`int k = 0`，只执行一次；
2. **条件**：`k < 3`，每轮开始前判断；
3. **增量**：`++k`，每轮结束后执行。

`k` 的作用域只在这个 `for` 语句里，循环结束后不可见。这比在外部声明 `int k;` 更干净。

三个部分都可以省略：

```cpp
for (;;) {
    // 无限循环，靠 break 退出
}
```

`for` 的初始化部分可以声明多个同类型变量：

```cpp
for (int i = 0, j = 10; i < j; ++i, --j) {
    // ...
}
```

但不建议在一行里塞太多逻辑。循环体才是重点。

## 3.6 范围 `for`：遍历一个序列

C++11 引入范围 `for`：

```cpp
int values[] = {10, 20, 30};
for (int value : values) {
    std::cout << value << '\n';
}
```

它等价于“对序列中的每个元素执行一次循环体”。编译器会把范围表达式拆成 `begin` 和 `end`，然后逐个解引用、递增。数组、`std::array`、`std::vector`、`std::string`、`std::initializer_list` 都支持。

几种常见写法：

```cpp
for (int value : values) { ... }          // 拷贝每个元素
for (int &value : values) { value *= 2; } // 引用，可修改
for (const int &value : values) { ... }   // const 引用，避免拷贝
for (const auto &value : values) { ... }  // 泛型写法
```

选择建议：

- 元素很小（`int`、`double`、指针）：按值即可；
- 元素较大（`std::string`、自定义类）：用 `const &`；
- 需要修改元素：用 `&`；
- 不确定时：`const auto &` 通常安全，除非你确实需要拷贝。

范围 `for` 也可以直接遍历花括号列表：

```cpp
for (int value : {1, 2, 3}) {
    std::cout << value << '\n';
}
```

这会创建一个 `std::initializer_list<int>`。它的底层数组生命期与范围 `for` 绑定，循环结束后销毁。

### 范围 `for` 的陷阱

- **不要在循环里增删容器元素**，除非你非常清楚迭代器失效规则。范围 `for` 底层持有迭代器，增删可能让它们失效。
- **不要返回循环变量的引用**，它每轮都是局部对象（除非你用引用绑定容器元素）。
- **注意临时对象的生命期**。`for (const auto &x : make_vector())` 在 C++17 里临时对象会延长到循环结束，但把范围表达式存成引用再返回就可能悬垂。

## 3.7 `break` 与 `continue`

- `break` 立即退出**最内层**的循环或 `switch`。
- `continue` 跳过本轮剩余部分，进入下一轮。

```cpp
for (int i = 0; i < 10; ++i) {
    if (i == 2) {
        continue;   // 跳过 2
    }
    if (i == 5) {
        break;      // 到 5 就停
    }
    std::cout << i << '\n';
}
```

输出 `0 1 3 4`。

`break` 只跳出最内层。要跳出多层循环，常见做法是：

- 用一个 `bool` 标志，外层循环检查它；
- 把循环封装成函数，用 `return` 退出；
- 用 `goto`（下一节）；
- C++20 的 `std::stop_token` 等机制（超出本章范围）。

`continue` 在 `while` 和 `do-while` 里要小心：它会直接跳到条件判断，可能跳过你原本想执行的增量。

## 3.8 `switch`：多路分支

```cpp
switch (command) {
case Command::Add:
    return left + right;
case Command::Subtract:
    return left - right;
case Command::Quit:
    return 0;
}
```

规则：

- 条件必须是整数类型、枚举类型或能隐式转换到它们的类型；不能是字符串、浮点。
- `case` 标签必须是**常量表达式**，不能重复。
- 匹配成功后，从该标签开始**顺序执行**，直到遇到 `break`、`return`、`throw` 或 `switch` 结束。
- 如果没有任何 `case` 匹配，执行 `default`（如果有）。
- `default` 不必须在最后，但放在最后更清楚。

### 贯穿（fallthrough）

C++ 的 `switch` 默认会“贯穿”：

```cpp
switch (code) {
case 2:
    std::cout << "two\n";
    // 没有 break，继续执行 case 3
case 3:
    std::cout << "three\n";
    break;
}
```

这有时是有意的，有时是 bug。C++17 提供了 `[[fallthrough]]` 属性，明确告诉编译器“我知道会贯穿”：

```cpp
case 2:
    std::cout << "two\n";
    [[fallthrough]];
case 3:
    std::cout << "three\n";
    break;
```

`[[fallthrough]]` 必须出现在一条语句之后、下一个 `case` 之前。它不会改变行为，只是表达意图，并抑制编译器的贯穿警告。

### 带初始化的 `switch`

C++17 也支持：

```cpp
switch (int n = compute(); n) {
case 1:
    // ...
    break;
default:
    break;
}
```

`n` 的作用域是整个 `switch`。

### `switch` 的作用域

`case` 标签本身不创建作用域。如果在某个 `case` 里声明变量，它的作用域会延伸到整个 `switch`，可能和后面的 `case` 冲突：

```cpp
switch (x) {
case 1:
    int value = 10;   // 作用域从这里到 switch 结束
    break;
case 2:
    // 这里也能看到 value，但可能未初始化
    break;
}
```

解决办法是用花括号给 `case` 创建块作用域：

```cpp
case 1: {
    int value = 10;
    break;
}
```

## 3.9 `goto`：被误解的工具

`goto` 无条件跳转到同一函数内的标签：

```cpp
int attempts = 0;
retry:
++attempts;
if (attempts < 3) {
    goto retry;
}
```

`goto` 在现代 C++ 里很少用，但不是完全没用。它适合从多层嵌套循环中一次性退出，或在生成代码、状态机里表达跳转。

限制：

- 不能跳进一个变量（有非平凡初始化）的作用域；编译器会拒绝。
- 不能跨函数跳转。
- 不能跳过初始化。

```cpp
goto skip;
int value = 10; // 错误：跳过了 value 的初始化
skip:
```

大多数情况下，用函数、循环条件或标志位替代 `goto` 更清楚。不要因为“C 有 goto”就把它当成默认工具。

## 3.10 嵌套循环

嵌套循环是二维问题的自然写法：

```cpp
for (int row = 1; row <= 3; ++row) {
    for (int column = 1; column <= 3; ++column) {
        std::cout << row * column << ' ';
    }
    std::cout << '\n';
}
```

每一轮外层循环，内层循环都完整执行一遍。时间复杂度通常是两层循环次数的乘积。

常见陷阱：

- 内外层用了同一个变量名（`-Wshadow` 会提醒）；
- 内层 `break` 以为能跳出外层；
- 循环边界差一（off-by-one）；
- 内层修改了外层循环依赖的变量。

## 3.11 代码导览

[`control.cpp`](control.cpp) 分成五部分：

- `show_if()`：`if` / `else if` / `else`，以及 C++17 的初始化语句；
- `show_switch()`：`enum class`、`switch`、`[[fallthrough]]`、`switch` 初始化；
- `show_loops()`：`while`、`do-while`、`for`、范围 `for`、`initializer_list`；
- `show_jumps()`：`break`、`continue`、`goto`；
- `show_nested()`：嵌套循环与 `continue`。

运行：

```sh
make 03-control-flow/control
./03-control-flow/control
```

`enum class` 是第 7 章的主题之一，这里先用它给 `switch` 提供一组有名字的整数常量。`[[fallthrough]]` 是 C++17 属性，编译时需要 `-std=c++17`。

## 3.12 常见陷阱

| 写法 | 问题 | 建议 |
|------|------|------|
| `if (x = 1)` | 赋值，条件恒真 | 写 `x == 1`，并打开警告 |
| `if (x > 0);` | 空语句，后面的块无条件执行 | 删除多余分号 |
| `while (i < n);` | 空循环体，可能死循环 | 检查分号 |
| `for (int i = 0; i < n; ++i)` 里改 `i` | 循环变量失控 | 用别的变量 |
| 范围 `for` 里增删元素 | 迭代器可能失效 | 先收集，再修改 |
| `switch` 忘记 `break` | 意外贯穿 | 用 `[[fallthrough]]` 表达有意贯穿 |
| `case` 里定义变量 | 作用域覆盖整个 `switch` | 用花括号创建块作用域 |
| 浮点做 `switch` | 不允许 | 用整数或枚举 |
| `do-while` 漏分号 | 编译错误 | 记住 `while (...);` |
| `goto` 跳过初始化 | 编译错误 | 调整作用域或不用 `goto` |

## 3.13 语法卡片

| 术语 | 一句话定义 | 本章例子 |
|------|------------|----------|
| 条件语句 | 根据布尔值选择执行路径 | `if (x > 0) { ... }` |
| 初始化语句 | C++17 在条件前声明变量 | `if (int n = f(); n > 0)` |
| 循环 | 重复执行一段代码 | `while`、`for` |
| 范围 `for` | 遍历序列的每个元素 | `for (int x : values)` |
| 贯穿 | `switch` 匹配后继续执行下一个标签 | 缺少 `break` |
| `[[fallthrough]]` | 明确标记有意贯穿 | `[[fallthrough]];` |
| `break` | 退出最内层循环或 `switch` | `break;` |
| `continue` | 跳过本轮剩余部分 | `continue;` |
| 标签 | `goto` 的跳转目标 | `retry:` |
| `enum class` | 强类型枚举 | `enum class Command { ... };` |

## 3.14 练习

1. **符号判断**：读入一个整数，用 `if` / `else if` / `else` 输出正数、负数或零。
2. **阶乘表**：用 `for` 打印 1 到 10 的阶乘，注意 `int` 会溢出，改用 `long long`。
3. **猜数字**：用 `while` 和 `std::cin` 写一个猜数字循环，输入非法时退出。
4. **菜单**：用 `do-while` 和 `switch` 实现一个简单菜单，输入 `q` 退出。
5. **范围 `for`**：定义一个 `std::vector<int>`，分别用按值、引用、`const` 引用遍历，修改元素并观察结果。
6. **贯穿**：写一个 `switch`，让 `case 1` 贯穿到 `case 2`，加上 `[[fallthrough]]`，再用 `-Wimplicit-fallthrough` 编译，观察警告是否消失。
7. **多层退出**：用两层循环查找一个值，分别用 `break` + 标志、`return` 函数、`goto` 三种方式退出，比较可读性。
8. **破坏实验**：删掉 `switch` 里的一个 `break`，运行程序，观察输出；再删掉 `do-while` 的分号，读编译错误。

## 3.15 下一章

**第 4 章 · 函数、重载与 lambda**

控制流把语句组织成路径，函数把路径组织成可复用的单元。下一章会讲声明与定义、参数传递、返回值、递归、重载、默认实参、函数指针，以及 C++11 的 lambda。

届时，`switch` 里的每个分支都可能变成一个函数，而函数本身也可以像值一样被传递。
