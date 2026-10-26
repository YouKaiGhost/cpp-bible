# 第 11 章 · 综合练习：一个小型表达式求值器

> **卷一 · 语言**　|　前置知识：第 0–10 章　|　建议用时：70 分钟

> 这一章不引入新语法。它只把前面的东西拼起来，看看它们能否一起工作。

前面十章分别讲了类型、表达式、控制流、函数、数组与指针、字符串、类、存储期、模板和 UB。现在写一个真正能运行的小程序：一个支持 `+ - * /`、括号、一元正负号和十进制小数的表达式求值器。

它的核心是一个**递归下降解析器**（recursive descent parser）。这个算法直接对应文法，结构清晰，是学习“用 C++ 写解析器”的好起点。

## 11.1 程序目标

输入：

```text
1 + 2 * 3
(1 + 2) * 3
10 / 4
-(2 + 3) * 4
```

输出：

```text
1 + 2 * 3 = 7
(1 + 2) * 3 = 9
10 / 4 = 2.5
-(2 + 3) * 4 = -20
```

支持：

- `+`、`-`、`*`、`/`；
- 括号；
- 一元 `+` 和 `-`；
- 十进制小数（如 `3.14`、`.5`）；
- 空白字符。

不支持：

- 变量、赋值、函数调用；
- 指数记法（`1e3`）；
- 隐式乘法（`2(3)`）；
- `%`、幂运算、比较和逻辑运算；
- 注释。

## 11.2 文法

用 EBNF 写出来：

```text
expression = term { ("+" | "-") term } ;
term       = factor { ("*" | "/") factor } ;
factor     = number
           | "(" expression ")"
           | ("+" | "-") factor ;
number     = digit { digit } [ "." { digit } ]
           | "." digit { digit } ;
```

这个文法已经编码了优先级：

- `expression` 处理加减，优先级最低；
- `term` 处理乘除，优先级高于加减；
- `factor` 处理数字、括号和一元正负号，优先级最高。

左递归被改写成了循环（`{ ... }`），所以递归下降不会陷入无限递归。这是手写解析器的常见技巧。

## 11.3 递归下降

递归下降的核心思想：**每个非终结符对应一个函数**。

```text
expression() -> term() (('+' | '-') term())*
term()       -> factor() (('*' | '/') factor())*
factor()     -> number | '(' expression ')' | ('+' | '-') factor()
```

每个函数负责：

1. 调用下一层的函数，拿到一个值；
2. 如果当前记号是这一层的运算符，消费它，再调用下一层；
3. 把结果组合起来；
4. 出错时记录错误并返回。

因为 `expression` 调用 `term`，`term` 调用 `factor`，`factor` 在括号里又调用 `expression`，所以优先级自然体现在调用链里。

## 11.4 词法分析：把字符变成记号

解析器不应该直接看字符。先写一个**词法分析器**（lexer），把输入切成记号（token）：

```cpp
enum class TokenKind {
    Number,
    Plus,
    Minus,
    Star,
    Slash,
    LeftParen,
    RightParen,
    End,
    Invalid
};

struct Token {
    TokenKind kind = TokenKind::End;
    double number = 0.0;
    std::size_t offset = 0;
};
```

`Token` 只保存三样东西：

- 类型：这个记号是数字、加号、左括号，还是结束；
- 数值：只有 `Number` 有意义；
- 偏移：这个记号在输入中的位置，用于错误报告。

`Lexer` 持有一个 `std::string_view` 和当前位置：

```cpp
class Lexer {
public:
    explicit Lexer(std::string_view input);
    Token next();
private:
    void skip_space();
    Token number();
    std::string_view input_;
    std::size_t position_;
};
```

`std::string_view` 不拥有字符，只指向调用者的输入。只要输入在解析期间活着，视图就有效。这里输入是 `main` 的 `argv[i]` 或 `std::vector<std::string>` 里的字符串，生命期足够长。

### 跳过空白

```cpp
void Lexer::skip_space()
{
    while (position_ < input_.size() && is_space(input_[position_])) {
        ++position_;
    }
}
```

空白不产生记号，直接在 `next()` 开头跳过。

### 识别数字

数字可以以数字开头，也可以以 `.` 开头：

```text
123
3.14
.5
1.
```

解析逻辑：

1. 先读整数部分；
2. 如果遇到 `.`，读小数部分；
3. 用 `value = value * 10 + digit` 和 `scale *= 0.1` 累加。

这里没有用 `std::stod`，因为 `std::stod` 需要以 `'\0'` 结尾的字符串，而 `string_view` 不保证；另外，手工解析能让你看清“小数点前后”的状态。

### 识别单字符记号

`+ - * / ( )` 都是一字符记号，直接 `switch`：

```cpp
switch (character) {
case '+':
    return {TokenKind::Plus, 0.0, offset};
// ...
}
```

其他字符返回 `Invalid`，由解析器统一报错。

## 11.5 解析器状态

`Parser` 持有一个 `Lexer`、当前记号、错误信息和错误位置：

```cpp
class Parser {
public:
    explicit Parser(std::string_view input);
    std::optional<double> parse();
    const std::string &error() const;
    std::size_t error_offset() const;

private:
    double expression();
    double term();
    double factor();
    double primary();
    void advance();
    void fail(std::string message, std::size_t offset);

    Lexer lexer_;
    Token current_;
    std::string error_;
    std::size_t error_offset_ = 0;
};
```

`parse()` 返回 `std::optional<double>`：

- 成功时有值；
- 失败时为空，具体错误从 `error()` 和 `error_offset()` 取。

为什么不用异常？解析错误在这里是“预期内的失败”，用 `optional` + 错误信息更直接，也避免异常带来的控制流跳跃。真实编译器通常有专门的诊断对象，这里简化成一个字符串和偏移。

### 推进与错误

```cpp
void Parser::advance()
{
    current_ = lexer_.next();
}

void Parser::fail(std::string message, std::size_t offset)
{
    if (error_.empty()) {
        error_ = std::move(message);
        error_offset_ = offset;
    }
}
```

只记录**第一个错误**。后面的错误往往是第一个错误造成的连锁反应；只报第一个更接近真实原因。

## 11.6 逐层解析

### `expression()`：加减

```cpp
double Parser::expression()
{
    double value = term();

    while (error_.empty() &&
           (current_.kind == TokenKind::Plus ||
            current_.kind == TokenKind::Minus)) {
        TokenKind operation = current_.kind;
        advance();

        double right = term();
        if (!error_.empty()) {
            return 0.0;
        }

        if (operation == TokenKind::Plus) {
            value += right;
        } else {
            value -= right;
        }
    }

    return value;
}
```

流程：

1. 先解析一个 `term`；
2. 只要当前记号是 `+` 或 `-`，消费它，再解析右边的 `term`；
3. 把结果加/减到 `value`；
4. 如果下层出错，立即返回。

左结合由循环自然实现：`1 - 2 - 3` 会先算 `1 - 2`，再减 `3`。

### `term()`：乘除

```cpp
double Parser::term()
{
    double value = factor();

    while (error_.empty() &&
           (current_.kind == TokenKind::Star ||
            current_.kind == TokenKind::Slash)) {
        TokenKind operation = current_.kind;
        std::size_t operation_offset = current_.offset;
        advance();

        double right = factor();
        if (!error_.empty()) {
            return 0.0;
        }

        if (operation == TokenKind::Star) {
            value *= right;
        } else {
            if (right == 0.0) {
                fail("division by zero", operation_offset);
                return 0.0;
            }
            value /= right;
        }
    }

    return value;
}
```

除零检查放在这里，因为只有 `/` 这一层知道除法的位置。错误位置记录的是 `/` 的偏移，而不是右操作数的偏移；报告时指向运算符更直观。

### `factor()`：一元正负号

```cpp
double Parser::factor()
{
    if (current_.kind == TokenKind::Plus) {
        advance();
        return factor();
    }

    if (current_.kind == TokenKind::Minus) {
        advance();
        return -factor();
    }

    return primary();
}
```

一元 `+` 和 `-` 是右结合的，并且可以叠加：`--5` 是 `-(-5)`，结果是 `5`。递归调用 `factor()` 自然支持。

### `primary()`：数字与括号

```cpp
double Parser::primary()
{
    if (current_.kind == TokenKind::Number) {
        double value = current_.number;
        advance();
        return value;
    }

    if (current_.kind == TokenKind::LeftParen) {
        advance();
        double value = expression();
        if (!error_.empty()) {
            return 0.0;
        }
        if (current_.kind != TokenKind::RightParen) {
            fail("expected ')'", current_.offset);
            return 0.0;
        }
        advance();
        return value;
    }

    fail("expected number or '('", current_.offset);
    return 0.0;
}
```

括号里的 `expression()` 会递归回最高层，所以括号能改变优先级。

## 11.7 入口与错误报告

`evaluate()` 把 lexer、parser 和结果包起来：

```cpp
Evaluation evaluate(std::string_view text)
{
    Parser parser(text);
    Evaluation result;
    result.value = parser.parse();

    if (!result.value.has_value()) {
        result.error = parser.error();
        result.error_offset = parser.error_offset();
    }

    return result;
}
```

`report()` 负责打印。成功时输出结果；失败时输出错误信息和插入符：

```text
1 / 0 = error: division by zero at offset 2
  1 / 0
    ^
```

插入符的位置是 `error_offset`。如果错误发生在输入末尾（例如 `2 +`），偏移等于输入长度，插入符会出现在最后一个字符之后。

`main()` 支持两种模式：

- 不带参数：运行内置样例；
- 带参数：每个参数当作一个表达式求值。

```sh
./calc
./calc "1 + 2 * 3"
./calc "2 * (3 + 4) - 5" "1 / 0"
```

## 11.8 编译与运行

```sh
make 11-capstone/calc
./11-capstone/calc
```

或者手动：

```sh
c++ -std=c++17 -Wall -Wextra -Wpedantic calc.cpp -o calc
```

预期输出：

```text
== expression evaluator ==
1 + 2 * 3 = 7
(1 + 2) * 3 = 9
10 / 4 = 2.5
-3 + 5 = 2
2 * (3 + 4) - 5 = 9
0.1 + 0.2 = 0.3
1 / 0 = error: division by zero at offset 2
  1 / 0
    ^
2 + = error: expected number or '(' at offset 3
  2 +
     ^

usage: calc "1 + 2 * 3"
```

`0.1 + 0.2` 用默认精度打印成 `0.3`；如果提高精度，会看到 `0.30000000000000004`。这是浮点表示，不是解析器的错误。

## 11.9 已知限制

这个求值器是教学程序，不是生产级计算器：

- 只支持十进制小数，不支持指数记法；
- 数字解析没有处理溢出/下溢；
- 没有 AST，直接求值，无法做变量、函数或优化；
- 错误恢复只报第一个错误；
- 没有运算符优先级表，优先级写死在函数调用链里；
- 空白只支持 ASCII 空格、制表、换行、回车；
- 不支持 `%`、幂、比较、逻辑、赋值；
- 浮点结果没有做容差处理，`0.1 + 0.2` 不会“自动等于 0.3”。

这些限制是故意的：每增加一个特性，程序就多一层复杂度。先把最小闭环走通，再扩展。

## 11.10 这个程序用到了什么

| 语言特性 | 在程序里的位置 |
|----------|----------------|
| `enum class` | `TokenKind` |
| 类与封装 | `Lexer`、`Parser` |
| 构造函数 | `Lexer(std::string_view)` |
| `const` 成员函数 | `error()`、`error_offset()` |
| `std::string_view` | 输入与词法分析 |
| `std::optional` | 解析结果 |
| `std::string` | 错误信息 |
| `std::vector` | 内置样例、命令行参数 |
| 引用 | `const std::string &` 循环变量 |
| 递归 | 表达式、括号、一元运算 |
| 异常安全 | 解析失败不抛出，返回 `optional` |
| `[[nodiscard]]` 思路 | 结果必须被检查（这里用 `optional` 的接口提醒） |

没有用到的：模板、继承、虚函数、智能指针、移动语义。不是因为它们不重要，而是这个程序不需要。**能用简单工具解决的问题，不要为了展示语言特性而复杂化。**

## 11.11 练习与扩展

1. **加 `%`**：在 `term()` 里增加 `%`，注意除零检查。
2. **加幂运算**：增加 `^`，它是右结合的，优先级高于乘除；想想为什么不能简单地在 `term()` 里加一个循环。
3. **加变量**：支持 `x = 3` 和后续表达式引用 `x`；需要一个符号表（`std::unordered_map<std::string, double>`）。
4. **加函数**：支持 `sqrt(2)`、`sin(0.5)`；需要在 `primary()` 里识别标识符和 `(`。
5. **AST**：把“直接求值”改成“先建语法树，再求值”；比较两种设计的优缺点。
6. **错误恢复**：一次报告多个错误，而不是只报第一个。
7. **容差输出**：让 `0.1 + 0.2` 显示为 `0.3`，同时保留 `1e-15` 级别的小量；设计一个合理的格式化策略。
8. **测试**：为解析器写一组表驱动测试，覆盖优先级、结合性、括号、错误输入。

## 11.12 结束语

这个程序不大，但它把前面十章的东西串成了一条线：

- 类型和值类别决定了 `Token`、`optional`、`string_view` 怎么用；
- 表达式和控制流决定了 lexer 和 parser 的分支；
- 函数和递归决定了递归下降的结构；
- 数组、指针和字符串决定了输入和错误报告的底层表示；
- 类和 RAII 决定了 `Lexer`、`Parser` 的生命期；
- 存储期和链接决定了多文件程序如何组织；
- 模板和 UB 提醒我们：泛型不是免费的，边界之外没有保证。

如果这套教材只让你记住一句话，那应该是：**先问标准保证了什么，再问编译器实际做了什么。** C++ 的自由很大，但自由的前提是知道边界在哪里。

下一站，附录 A 会把 C++20/23 的新特性摊开：concepts、ranges、coroutines、modules、`<=>`、`constexpr` 的扩展。它们不是另一门语言，而是同一套规则在新版本里的延伸。
