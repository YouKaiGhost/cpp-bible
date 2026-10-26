# 第 6 章 · 字符、字符串与视图

> **卷一 · 语言**　|　前置知识：第 5 章　|　建议用时：50 分钟

> 字符串不是“一串字符”这么简单。它同时是内存布局、编码、所有权和接口约定的交叉点。

数组和指针是底层表示，字符串是它们最常见的应用。这一章讲 `char`、C 风格字符串、`std::string`、`std::string_view`、原始字符串、数值转换，以及 UTF-8 的字节视角。

C++ 有三套常用的字符串表示：

- **C 风格字符串**：以 `'\0'` 结尾的 `char` 数组，标准库和操作系统接口大量使用；
- **`std::string`**：拥有字符存储、自动管理生命期的类；
- **`std::string_view`**：只读、非拥有的字符视图，用来避免拷贝。

理解三者的边界，比背 API 更重要。

## 6.1 本章语法地图

| 语法 | 本章出现的位置 |
|------|----------------|
| `char` | `char c = 'A';` |
| 字符数组 | `char text[] = "hello";` |
| C 字符串函数 | `std::strlen`、`std::strcmp`、`std::strcpy` |
| `std::string` | `std::string text = "C++";` |
| 字符串视图 | `std::string_view view = text;` |
| 字符串字面量 | `"hello"`、`u8"你好"` |
| 原始字符串 | `R"(...)"` |
| 字面量拼接 | `"Hello, " "World!"` |
| 数值转换 | `std::to_string`、`std::stoi`、`std::stod` |
| 输入分割 | `std::getline`、`operator>>` |

## 6.2 `char`：最小的整数类型

`char` 是 C++ 的字符类型，也是整数类型：

```cpp
char letter = 'A';
char digit = '7';
char newline = '\n';
```

字符字面量用单引号，类型是 `char`（在 C++ 中，`sizeof('A') == 1`；C 里字符常量是 `int`）。`'A'` 的值是实现字符集里 `A` 的编码，常见是 65。

`char` 可以参与整数运算：

```cpp
char next = static_cast<char>('A' + 1);   // 'B'
```

但要注意：

- `char`、`signed char`、`unsigned char` 是三种不同的类型。
- 普通 `char` 是否有符号由实现决定。
- `char` 的范围不一定能容纳所有字节值；表示原始字节时，用 `unsigned char` 或 `std::byte`。
- 字符的“值”取决于执行字符集。常见实现用 ASCII 兼容的 UTF-8，但不是标准保证。

### 转义序列

| 写法 | 含义 |
|------|------|
| `'\n'` | 换行 |
| `'\t'` | 水平制表符 |
| `'\\'` | 反斜杠 |
| `'\''` | 单引号 |
| `'\"'` | 双引号 |
| `'\0'` | 空字符 |
| `'\x41'` | 十六进制 41 对应的字符 |
| `'\101'` | 八进制 101 对应的字符 |

`'\0'` 是 C 风格字符串的终止符。它不是字符串内容的一部分，但占了存储。

## 6.3 字符串字面量

```cpp
"hello"
```

字符串字面量的类型是 `const char[N]`，其中 `N` 包含末尾的 `'\0'`。所以 `"hello"` 的类型是 `const char[6]`，`sizeof("hello")` 是 6。

```cpp
std::cout << sizeof("hello");   // 6
std::cout << std::strlen("hello"); // 5
```

`std::strlen` 数到 `'\0'` 为止，不包括 `'\0'`；`sizeof` 包括。

### 字面量拼接

相邻的字符串字面量会在编译期拼接：

```cpp
const char *text = "Hello, "
                   "World!";
```

结果是 `"Hello, World!"`。这发生在翻译阶段，不是运行时 `+`。

### 原始字符串

原始字符串用 `R"(...)"`，里面的反斜杠和引号不需要转义：

```cpp
const char *path = R"(C:\code\cpp "quoted")";
```

原始字符串也可以带自定义分隔符，处理内容里出现 `)"` 的情况：

```cpp
const char *text = R"delim(raw )" content)delim";
```

### 编码前缀

| 前缀 | 类型（C++17） | 含义 |
|------|----------------|------|
| 无 | `const char[N]` | 普通字符集 |
| `u8` | `const char[N]` | UTF-8（C++20 改为 `const char8_t[N]`） |
| `u` | `const char16_t[N]` | UTF-16 |
| `U` | `const char32_t[N]` | UTF-32 |
| `L` | `const wchar_t[N]` | 宽字符 |

本书以 UTF-8 源文件和普通 `char` 字符串为主。`u8"..."` 在 C++17 里仍然是 `const char[]`，可以直接传给 `std::string`；C++20 换成 `char8_t`，需要转换。附录会提。

## 6.4 C 风格字符串与字符数组

C 风格字符串是“以 `'\0'` 结尾的 `char` 数组”：

```cpp
char text[] = "hello";   // 类型 char[6]，内容 h e l l o \0
const char *p = "hello"; // 指向字符串字面量，不应修改
```

常见操作来自 `<cstring>`：

```cpp
std::strlen(text);       // 长度，不含 '\0'
std::strcmp(a, b);       // 比较，0 表示相等
std::strcpy(dest, src);  // 复制，调用者保证 dest 足够大
std::strcat(dest, src);  // 拼接
```

这些函数不做边界检查，使用时要自己保证缓冲区足够大。`strcpy`、`strcat`、`sprintf` 是缓冲区溢出的历史常客；现代代码应优先用 `std::string`，必须用 C 接口时用带长度的版本（如 `strncpy`、`snprintf`，但注意它们的截断语义）。

```cpp
char buffer[16] = {};
std::strcpy(buffer, "hello");   // 安全：16 足够
```

`char text[] = "hello";` 是可修改的数组；`const char *p = "hello";` 指向字面量，通过 `const_cast` 去掉 `const` 再修改是 UB。字符串字面量通常放在只读段，写入可能直接崩溃。

## 6.5 `std::string`：拥有字符的字符串

`std::string` 定义在 `<string>`，是标准库的字符串类：

```cpp
#include <string>

std::string text = "C++";
text += " 语言";
text.append("圣经");
```

它拥有自己的字符存储，自动管理分配和释放。常用操作：

| 操作 | 含义 |
|------|------|
| `size()` / `length()` | 字符数（字节数） |
| `empty()` | 是否为空 |
| `capacity()` | 当前存储容量 |
| `reserve(n)` | 预分配至少 n 个字符的空间 |
| `clear()` | 清空内容，不释放容量 |
| `operator[]` | 不检查边界的访问 |
| `at(i)` | 检查边界，越界抛 `std::out_of_range` |
| `front()` / `back()` | 首/尾字符 |
| `c_str()` | 返回以 `'\0'` 结尾的 C 字符串 |
| `data()` | 返回底层字符指针（C++17 起可写） |
| `find()` / `rfind()` | 查找子串或字符 |
| `substr(pos, n)` | 返回子串 |
| `append()` / `insert()` / `erase()` / `replace()` | 修改内容 |
| `compare()` | 比较 |

`std::string` 的 `size()` 是**字节数**，不是“字符数”或“显示宽度”。对 UTF-8 中文，一个汉字通常占 3 个字节：

```cpp
std::string text = "你好";
text.size();   // 6，通常
```

`std::string` 可以和 C 风格字符串互操作：

```cpp
std::string text = "hello";
const char *c = text.c_str();   // 只读、以 '\0' 结尾
std::string copy(c);
```

`c_str()` 返回的指针在 `std::string` 被修改或销毁后可能失效。不要长期保存它。

### 小字符串优化

很多实现使用**小字符串优化**（SSO）：短字符串直接存在对象内部，不进行堆分配。`capacity()` 的初始值通常就是 SSO 容量，但具体数值由实现决定。教材不假设 SSO 一定存在，也不依赖它的容量。

## 6.6 `std::string_view`：只读、非拥有

`std::string_view` 是 C++17 引入的字符串视图：

```cpp
#include <string_view>

std::string text = "one,two,three";
std::string_view view = text;

view.size();          // 13
view.substr(0, 3);    // "one"
view.remove_prefix(4);
view.remove_suffix(6);
```

它由“指向字符的指针 + 长度”组成，**不拥有字符**，也不保证以 `'\0'` 结尾。它适合作为只读参数：

```cpp
void print(std::string_view text)
{
    std::cout << text << '\n';
}
```

调用者可以传 `std::string`、字符串字面量、`const char *`，都不需要构造临时 `std::string`：

```cpp
print("literal");
print(std::string("owned"));
```

### `string_view` 的生命期

`string_view` 不延长被查看对象的生命期：

```cpp
std::string_view bad()
{
    std::string local = "temporary";
    return local;   // 悬垂：local 已销毁
}
```

返回 `string_view` 时，必须保证底层字符在调用者使用期间仍然存在。常见安全用法：

- 参数：调用期间底层对象活着；
- 成员：底层对象比视图活得久（但要有清晰的所有权约定）；
- 解析：视图指向一个长期存在的输入缓冲区。

### `string_view` 与 C 接口

`string_view` 不保证 `'\0'` 结尾，不能直接传给 `strlen`、`printf("%s")` 等 C 函数。需要时先构造 `std::string`：

```cpp
std::string owned(view);
```

### `string_view` 的修改

`string_view` 本身可以重新绑定到别的字符范围，但不能通过它修改字符（它始终是 `const` 视图）：

```cpp
std::string text = "hello";
std::string_view view = text;
// view[0] = 'H';   // 错误：只读
```

需要修改时用 `std::string &` 或 `std::span<char>`（C++20）。

## 6.7 数值与字符串互转

### 数字转字符串

```cpp
std::string text = std::to_string(42);
std::string real = std::to_string(3.5);
```

`std::to_string` 定义在 `<string>`，支持整数和浮点。它的格式化规则比较固定，不适合需要精确控制小数位或本地化的场景；那种场景用 `std::ostringstream` 或 C++20 的 `std::format`。

### 字符串转数字

```cpp
int value = std::stoi("123");
double real = std::stod("2.5");
```

`std::stoi`、`std::stol`、`std::stod` 等定义在 `<string>`，失败时抛异常：

- `std::invalid_argument`：没有可转换的字符；
- `std::out_of_range`：结果超出目标类型范围。

它们可以接收一个 `pos` 指针，报告转换结束的位置：

```cpp
std::size_t pos = 0;
int value = std::stoi("123abc", &pos);   // value == 123, pos == 3
```

不抛异常的替代方案是 C++17 的 `std::from_chars`（整数版本）：

```cpp
#include <charconv>

int value = 0;
const char *begin = text.data();
const char *end = begin + text.size();
auto result = std::from_chars(begin, end, value);
if (result.ec == std::errc{}) {
    // 转换成功
}
```

`from_chars` 不分配内存、不抛异常，适合高性能解析。浮点版本的广泛支持要更晚一些，使用前查编译器文档。

## 6.8 输入、分割与拼接

### 读取一行

```cpp
std::string line;
std::getline(std::cin, line);
```

`std::getline` 读取到换行符为止，丢弃换行符。它和 `operator>>` 的行为不同：

```cpp
std::string word;
std::cin >> word;   // 跳过前导空白，读到下一个空白
```

混用两者时要注意缓冲区里残留的换行符：

```cpp
int value;
std::cin >> value;
std::string line;
std::getline(std::cin, line);   // 可能读到空行
```

先调用 `std::cin.ignore()` 清掉换行，或者统一用 `getline` 再解析。

### 按分隔符分割

```cpp
std::istringstream input("alpha,beta,gamma");
std::string field;
while (std::getline(input, field, ',')) {
    // 处理 field
}
```

`std::getline` 的第三个参数是分隔符。它不会返回空字段的“尾随分隔符”问题，但连续分隔符会产生空字段，需要自己处理。

### 拼接

```cpp
std::string result = "prefix:" + text;
result += "suffix";
```

`std::string` 支持 `+`、`+=`、`append`。注意 `"a" + "b"` 是指针相加，不是字符串拼接；至少有一个操作数必须是 `std::string`：

```cpp
std::string text = std::string("a") + "b";
```

C++14 起有 `operator""s`：

```cpp
using namespace std::string_literals;
std::string text = "hello"s + " world";
```

本书仍然建议显式写 `std::string`，少用 `using namespace`。

## 6.9 UTF-8、宽字符与编码

C++ 的 `char` 是字节，不是 Unicode 码点。UTF-8 是一种变长编码：

- ASCII 字符占 1 字节；
- 拉丁扩展、希腊字母等占 2 字节；
- 常用汉字占 3 字节；
- 部分表情符号占 4 字节。

```cpp
std::string text = u8"你好，世界";
std::cout << text.size();   // 通常是 15
```

`std::string::size()` 返回字节数，不是“字符数”。要按 Unicode 码点遍历，需要专门的库（如 ICU）或 C++20 的 `char8_t` 加手工解码；标准库在 C++17 里没有完整的 Unicode 文本处理。

`wchar_t` 的宽度由实现决定（Windows 通常 16 位，Linux 通常 32 位），不适合作为跨平台 Unicode 表示。现代跨平台代码通常以 UTF-8 的 `std::string` 为存储，在边界处转换。

C++20 引入了 `char8_t` 和 `std::u8string`，试图把 UTF-8 和普通字节区分开；代价是和现有 `std::string` 接口不直接兼容。本书以 C++17 为基线，附录会提。

## 6.10 代码导览

[`strings.cpp`](strings.cpp) 分成六部分：

- `show_c_strings()`：字符数组、`strlen`、`strcpy`、`strcmp`；
- `show_std_string()`：`std::string` 的拼接、查找、替换、`c_str`；
- `show_string_view()`：视图的 `substr`、`remove_prefix`、`remove_suffix`；
- `show_literals()`：原始字符串、字面量拼接、`u8` 字符串；
- `show_conversions()`：`to_string`、`stoi`、`stod`；
- `show_split()`：`getline` 分割和 `operator>>` 分词。

运行：

```sh
make 06-strings/strings
./06-strings/strings
```

代码里打印了 `text.back()` 的字节值，而不是字符，因为 `std::string` 的最后一个字节可能只是某个 UTF-8 汉字的一部分。这个细节会在输出中直接看到。

## 6.11 常见陷阱

| 写法 | 问题 | 建议 |
|------|------|------|
| `char *p = const_cast<char *>("text"); p[0] = 'T';` | 修改字面量，UB | 用 `const char *` 或 `std::string` |
| `strcpy` 到不够大的缓冲区 | 缓冲区溢出 | 用 `std::string` 或带长度的接口 |
| `std::string::size()` 当作字符数 | 返回字节数 | 需要 Unicode 处理时用专门库 |
| 返回局部 `std::string` 的 `string_view` | 悬垂 | 返回值或确保底层对象存活 |
| `string_view` 传给 `printf("%s")` | 不保证 `'\0'` 结尾 | 先构造 `std::string` |
| 保存 `c_str()` 后修改原字符串 | 指针可能失效 | 需要时重新取 |
| `"a" + "b"` | 指针相加，编译错误或 UB | 至少一个操作数是 `std::string` |
| `stoi` 不处理异常 | 输入非法时抛异常 | 捕获 `std::exception` 或用 `from_chars` |
| `cin >>` 后直接 `getline` | 残留换行 | `ignore()` 或统一用 `getline` |
| `string_view` 修改内容 | 只读视图 | 用 `std::string` 或 `std::span<char>` |

## 6.12 语法卡片

| 术语 | 一句话定义 | 本章例子 |
|------|------------|----------|
| `char` | 字符/最小整数类型 | `char c = 'A';` |
| C 风格字符串 | 以 `'\0'` 结尾的 `char` 数组 | `char text[] = "hi";` |
| 字符串字面量 | 编译期常量字符数组 | `"hello"` |
| 原始字符串 | 不需要转义的字符串字面量 | `R"(C:\code)"` |
| `std::string` | 拥有字符存储的字符串类 | `std::string text;` |
| `std::string_view` | 只读、非拥有的字符视图 | `std::string_view view;` |
| SSO | 短字符串存在对象内部 | 实现定义 |
| `c_str()` | 返回以 `'\0'` 结尾的指针 | `text.c_str()` |
| `std::getline` | 按行或分隔符读取 | `getline(cin, line);` |
| UTF-8 | 变长 Unicode 编码 | `u8"你好"` |

## 6.13 练习

1. **C 与 C++**：分别用 `char[]` + `strlen` 和 `std::string` + `size()` 计算 `"hello"` 的长度，比较结果和代码量。
2. **原始字符串**：用原始字符串输出一段包含反斜杠、双引号和换行的 Windows 路径。
3. **`string_view`**：写一个函数 `count_vowels(std::string_view)`，分别传入字面量和 `std::string`，观察不需要构造临时字符串。
4. **悬垂**：写一个返回 `std::string_view` 的函数，让它指向局部 `std::string`，**不要调用**；用注释解释悬垂原因，再改成返回值。
5. **数值转换**：用 `std::stoi` 解析 `"123abc"`，打印转换结果和结束位置；再用 `std::from_chars` 做一遍。
6. **分割**：读入一行逗号分隔的字符串，用 `std::getline` 分割并打印每个字段；处理连续逗号产生的空字段。
7. **UTF-8**：输出 `u8"你好，世界"` 的 `size()` 和每个字节的十六进制值，验证一个汉字占 3 字节。
8. **破坏实验**：把 `const char *p = "hello";` 改成 `char *p = const_cast<char *>("hello");`，尝试修改 `p[0]`，用 AddressSanitizer 运行，读诊断。

## 6.14 下一章

**第 7 章 · 类、构造与资源管理**

字符串的核心是“谁拥有字符、谁负责释放”。下一章会把这个问题提升到语言层面：类、构造函数、析构函数、RAII、拷贝、移动、继承和虚函数。

届时，`std::string` 为什么能自动管理内存、为什么拷贝和移动的代价不同，会有完整的答案。
