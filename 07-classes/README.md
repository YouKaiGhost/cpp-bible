# 第 7 章 · 类、构造与资源管理

> **卷一 · 语言**　|　前置知识：第 1–6 章　|　建议用时：70 分钟

> 类的核心不是“把函数和数据包在一起”，而是让对象自己负责自己的生命期。

字符串的核心是“谁拥有字符、谁负责释放”。这一章会把这个问题提升到语言层面：类、构造函数、析构函数、RAII、拷贝、移动、继承和虚函数。

C++ 的类有四个特殊成员函数：默认构造、拷贝构造、拷贝赋值、析构。C++11 又加了移动构造和移动赋值。它们什么时候被编译器自动生成、什么时候必须手写、什么时候应该删除，是本章的主线。

## 7.1 本章语法地图

| 语法 | 本章出现的位置 |
|------|----------------|
| `class` / `struct` | `class Buffer { ... };` |
| 访问控制 | `public:`、`private:`、`protected:` |
| 构造函数 | `Buffer(std::size_t size);` |
| 成员初始化列表 | `: data_(...), size_(size)` |
| `explicit` | `explicit Buffer(std::size_t);` |
| 析构函数 | `~Buffer();` |
| `= default` / `= delete` | `Buffer(const Buffer &) = delete;` |
| 拷贝 / 移动 | 拷贝构造、移动构造、赋值 |
| `const` 成员函数 | `std::size_t size() const;` |
| `mutable` | `mutable int hits_;` |
| 运算符重载 | `operator+=`、`operator==`、`operator<<` |
| 继承 | `class Circle : public Shape` |
| 虚函数 | `virtual double area() const = 0;` |
| `override` / `final` | `double area() const override;` |
| `enum class` | `enum class Color { ... };` |

## 7.2 从 `struct` 到 `class`

C++ 的 `struct` 和 `class` 几乎一样，唯一区别是默认访问权限：

```cpp
struct Point {
    int x;   // 默认 public
    int y;
};

class Point {
    int x;   // 默认 private
    int y;
};
```

继承时的默认访问权限也一样：`struct` 默认 public 继承，`class` 默认 private 继承。习惯上：

- 只是把几个值放在一起，没有不变量：用 `struct`；
- 需要维护不变量、封装实现：用 `class`。

```cpp
struct Point {
    int x;
    int y;
};

Point p{1, 2};   // 聚合初始化
```

`Point` 是**聚合类型**：没有用户提供的构造函数，没有私有/受保护的非静态数据成员，没有虚函数，没有基类。聚合可以用花括号按成员顺序初始化。

一旦你给类写了构造函数，它就不再是聚合，花括号会走构造函数。

### 访问控制

- `public`：任何代码都能访问。
- `private`：只有类自己的成员函数和友元能访问。
- `protected`：类自己和派生类能访问。

```cpp
class Buffer {
public:
    std::size_t size() const;
private:
    std::unique_ptr<int[]> data_;
    std::size_t size_;
};
```

封装的意义不是“藏起来显得高级”，而是让类可以维护不变量。`Buffer` 的 `size_` 必须和 `data_` 的实际长度一致；如果外部能随便改 `size_`，这个不变量就守不住。

## 7.3 构造函数

构造函数在对象创建时运行，负责把对象带入有效状态：

```cpp
class Point {
public:
    Point(int x, int y) : x_(x), y_(y)
    {
    }

private:
    int x_;
    int y_;
};
```

构造函数的名字和类名相同，没有返回类型。成员初始化列表在函数体之前执行：

```cpp
Point(int x, int y) : x_(x), y_(y) { }
```

为什么用初始化列表而不是在函数体里赋值？因为成员在进入函数体之前就已经构造完成；在函数体里写 `x_ = x;` 是赋值，不是初始化。对 `const` 成员、引用成员和没有默认构造函数的成员，只能用初始化列表。

### 初始化顺序

成员按**声明顺序**初始化，不是按初始化列表里的书写顺序：

```cpp
class Example {
public:
    Example() : b_(1), a_(b_) { }  // 危险：a_ 先初始化，b_ 还没初始化
private:
    int a_;
    int b_;
};
```

编译器通常会给出 `-Wreorder` 警告。正确的做法是让初始化列表的顺序和声明顺序一致。

### `explicit`

单参数构造函数默认允许隐式转换：

```cpp
class Buffer {
public:
    Buffer(std::size_t size);   // 允许 Buffer b = 10;
};
```

加 `explicit` 可以禁止这种隐式转换：

```cpp
explicit Buffer(std::size_t size);   // 必须 Buffer b(10); 或 Buffer b{10};
```

大多数单参数构造函数都应该加 `explicit`，除非你确实想要隐式转换（例如 `std::string` 从 `const char *` 构造）。

### 默认构造、委托构造

```cpp
class Point {
public:
    Point() : Point(0, 0) { }   // 委托构造，C++11
    Point(int x, int y) : x_(x), y_(y) { }
private:
    int x_;
    int y_;
};
```

`= default` 让编译器生成默认实现：

```cpp
Point() = default;
```

`= delete` 明确删除某个函数：

```cpp
Point(const Point &) = delete;
```

删除比“不写”更强：不写可能被编译器自动生成，删除则明确禁止，并在调用时给出编译错误。

## 7.4 析构函数与 RAII

析构函数在对象生命期结束时运行：

```cpp
class Buffer {
public:
    ~Buffer()
    {
        // 释放资源
    }
};
```

C++ 的核心资源管理思想是 **RAII**（Resource Acquisition Is Initialization）：把资源的获取绑定到对象构造，把释放绑定到析构。对象离开作用域时，析构函数自动运行，无论是因为正常返回、异常还是提前退出（`goto` 除外的一些边界情况）。

标准库的 `std::string`、`std::vector`、`std::unique_ptr` 都是 RAII 类型。`Buffer` 用 `std::unique_ptr<int[]>` 管理数组，自己就不需要手写 `delete[]`：

```cpp
class Buffer {
public:
    explicit Buffer(std::size_t size)
        : data_(std::make_unique<int[]>(size)), size_(size)
    {
    }

private:
    std::unique_ptr<int[]> data_;
    std::size_t size_;
};
```

`unique_ptr` 的析构函数会自动 `delete[]`。即使 `Buffer` 的构造函数在分配后抛出异常，已经构造完成的成员也会被正确销毁。

### 析构顺序

对象销毁时，析构顺序与构造顺序相反：

- 成员按声明顺序的逆序销毁；
- 基类在派生类之后销毁；
- 局部对象按创建顺序的逆序销毁。

```cpp
{
    Buffer first(4);
    Buffer second(4);
}   // second 先销毁，first 后销毁
```

## 7.5 拷贝、移动与三五法则

### 拷贝

拷贝构造用同类型的另一个对象初始化新对象：

```cpp
Buffer copy = original;   // 拷贝构造
```

拷贝赋值把另一个对象的值赋给已有对象：

```cpp
copy = original;          // 拷贝赋值
```

编译器会自动生成拷贝构造和拷贝赋值，逐个成员拷贝。对 `Buffer` 这种拥有资源的类，逐成员拷贝 `unique_ptr` 是非法的（`unique_ptr` 不可拷贝），所以编译器会把拷贝操作定义为删除。

### 移动

移动构造和移动赋值从另一个对象“偷走”资源：

```cpp
Buffer moved = std::move(original);
```

移动之后，原对象仍然有效，但处于“已被移动”的状态；具体内容由类型决定。对 `Buffer`，`unique_ptr` 被移走，原对象不再拥有数组。

`std::move` 本身不移动，它只是把表达式转成右值引用，让编译器选择移动构造/赋值。第 2 章已经讲过值类别。

### 特殊成员函数生成规则

| 你声明了 | 编译器还会生成 | 备注 |
|----------|----------------|------|
| 默认构造 | 拷贝、移动、析构 | 只要没有其他构造函数 |
| 析构函数 | 拷贝构造、拷贝赋值 | 移动操作不再自动生成 |
| 拷贝构造 | 拷贝赋值（通常） | 移动操作不再自动生成 |
| 拷贝赋值 | 拷贝构造（通常） | 移动操作不再自动生成 |
| 移动构造 | 拷贝构造/赋值被删除 | 需要显式恢复 |
| 移动赋值 | 拷贝构造/赋值被删除 | 需要显式恢复 |

这张表只抓主干，细节以标准为准。实践中的简单规则是：

- **Rule of Zero**：能不让类直接拥有资源，就不拥有；用 `std::string`、`std::vector`、`std::unique_ptr` 等 RAII 成员，编译器生成的特殊成员通常就够用。
- **Rule of Three**：如果你需要手写析构、拷贝构造、拷贝赋值中的任何一个，通常三个都需要。
- **Rule of Five**：C++11 之后，再加上移动构造和移动赋值。

`Buffer` 选择了“删除拷贝、默认移动”，因为数组资源应该独占。`Tracked` 则显式实现了拷贝和移动，用来观察计数。

### `noexcept`

移动构造和移动赋值通常应该标记 `noexcept`：

```cpp
Buffer(Buffer &&) noexcept = default;
```

`std::vector` 在扩容时，如果元素的移动构造是 `noexcept`，它会选择移动；否则可能选择拷贝，以保证强异常安全。移动操作原则上不应该抛异常。

## 7.6 `const` 成员与 `mutable`

`const` 成员函数承诺不修改对象的可观察状态：

```cpp
class Buffer {
public:
    std::size_t size() const { return size_; }
};
```

const 对象只能调用 const 成员函数：

```cpp
const Buffer buffer(4);
buffer.size();   // 可以
// buffer[0] = 1; // 不可以
```

`mutable` 允许某个成员在 const 成员函数里被修改，用于缓存、计数器、互斥量等“逻辑上不改变对象值”的状态：

```cpp
class Counter {
public:
    void hit() const { ++hits_; }
    int hits() const { return hits_; }
private:
    mutable int hits_ = 0;
};
```

`Counter` 的 `hit()` 是 const，但内部计数器可以变。这不算“欺骗 const”，而是把“物理状态”和“逻辑状态”分开；但不要滥用，否则 const 语义会变得不可信。

## 7.7 `this` 与运算符重载

在成员函数里，`this` 是指向当前对象的指针：

```cpp
Point &operator+=(const Point &other)
{
    x_ += other.x_;
    y_ += other.y_;
    return *this;
}
```

返回 `*this` 的引用，支持链式调用：

```cpp
point += Point(1, 2);   // 返回 point 的引用
```

运算符重载的规则：

- 至少有一个操作数是类类型或枚举类型；
- 不能改变运算符的优先级、结合性和元数；
- 不能重载 `::`、`.`、`.*`、`?:`、`sizeof` 等少数运算符；
- `=`、`[]`、`()`、`->` 必须是成员函数；
- 流插入 `<<` 通常写成非成员函数（或友元），因为左操作数是 `std::ostream`，不是你自己的类。

```cpp
friend std::ostream &operator<<(std::ostream &out, const Point &point)
{
    return out << '(' << point.x_ << ", " << point.y_ << ')';
}
```

`operator==` 可以写成成员或非成员。C++20 起可以用 `operator<=>` 自动生成比较；C++17 里需要手写。

不要为了“看起来像内置类型”而重载所有运算符。只重载语义自然的运算符，并保持和内置类型一致的行为。

## 7.8 继承、虚函数与多态

继承表达“是一种”（is-a）关系：

```cpp
class Shape {
public:
    virtual ~Shape() = default;
    virtual double area() const = 0;
    virtual const char *name() const = 0;
};

class Circle final : public Shape {
public:
    explicit Circle(double radius) : radius_(radius) { }

    double area() const override
    {
        return 3.14159265358979323846 * radius_ * radius_;
    }

    const char *name() const override
    {
        return "Circle";
    }

private:
    double radius_;
};
```

关键点：

- `virtual` 函数允许派生类覆盖，调用时根据**动态类型**选择实现。
- `= 0` 表示纯虚函数，类成为**抽象类**，不能直接实例化。
- `override` 让编译器检查你确实覆盖了基类虚函数，拼错名字会报错。
- `final` 禁止继续派生或覆盖。
- 基类的析构函数必须是 `virtual`，否则通过基类指针删除派生对象时，派生类的析构函数不会运行，资源泄漏或 UB。
- 派生类构造时先构造基类，再构造成员，最后执行派生类构造函数体；析构顺序相反。

### 多态与对象切片

通过基类指针或引用调用虚函数，才会发生动态绑定：

```cpp
std::vector<std::unique_ptr<Shape>> shapes;
shapes.push_back(std::make_unique<Circle>(2.0));
shapes.push_back(std::make_unique<Rectangle>(3.0, 4.0));

for (const auto &shape : shapes) {
    std::cout << shape->name() << ' ' << shape->area() << '\n';
}
```

如果按值把派生类赋给基类对象，会发生**对象切片**（object slicing）：派生类特有的部分被切掉，只剩基类子对象。虚函数也不再按派生类动态绑定：

```cpp
struct Base {
    virtual void identify() const { std::cout << "Base\n"; }
};

struct Derived : Base {
    void identify() const override { std::cout << "Derived\n"; }
    int extra = 42;
};

Derived derived;
Base sliced = derived;   // 切片：extra 丢失，identify() 调用 Base 版本
```

多态对象通常通过指针或引用传递，而不是按值传递基类。

### 继承与名字隐藏

派生类里的同名函数会隐藏基类的所有同名重载，而不是只隐藏签名相同的那一个：

```cpp
class Base {
public:
    void f(int);
    void f(double);
};

class Derived : public Base {
public:
    void f(const char *);
};

Derived d;
d.f(42);   // 错误：Base::f(int) 被隐藏
```

需要时用 `using Base::f;` 把基类重载引入派生类作用域。

### 多重继承

C++ 支持多重继承，也带来菱形继承、二义性和布局复杂性问题。日常设计通常优先组合而不是多重继承；接口式多重继承（纯虚基类）相对安全。本章不展开。

## 7.9 `enum class` 与 `union`

### `enum class`

C++11 的强类型枚举：

```cpp
enum class Color {
    Red,
    Green,
    Blue
};

Color color = Color::Red;
// int value = color;          // 错误：不能隐式转 int
int value = static_cast<int>(color);
```

优点：

- 枚举名在枚举作用域内，不污染外层；
- 不会隐式转换成整数；
- 可以指定底层类型：`enum class Color : unsigned char { ... };`
- 前向声明更容易。

C 风格 `enum` 仍然存在，但会隐式转整数，枚举名泄漏到外层作用域。新代码优先用 `enum class`。

### `union`

`union` 的成员共享同一块存储，同一时刻只有一个成员是“活跃”的：

```cpp
union Value {
    int integer;
    double real;
};
```

写入一个成员，再读另一个成员，通常是 UB（取决于类型和公共初始序列规则）。需要“带标签的联合”时，可以自己加一个枚举标签，或用 C++17 的 `std::variant`：

```cpp
std::variant<int, double> value = 42;
```

`std::variant` 知道当前哪个类型是活跃的，访问更安全。它属于标准库，本章只提名字。

## 7.10 代码导览

[`classes.cpp`](classes.cpp) 分成六部分：

- `show_raii()`：`Buffer` 用 `unique_ptr` 管理数组，删除拷贝，允许移动；
- `show_special_members()`：`Tracked` 统计拷贝和移动次数；
- `show_const_members()`：`Counter` 的 `mutable` 计数器；
- `show_operators()`：`Point` 的 `+=`、`==`、`<<`；
- `show_polymorphism()`：`Shape`、`Circle`、`Rectangle` 和 `unique_ptr` 容器；
- `show_enum()`：`enum class Color`。

运行：

```sh
make 07-classes/classes
./07-classes/classes
```

`Tracked` 的输出会显示 `vector` 在 `reserve(3)` 后 `push_back` 的拷贝/移动次数。具体数字依赖标准库实现，但“移动比拷贝多”这个趋势是本章想让你看到的。

## 7.11 常见陷阱

| 写法 | 问题 | 建议 |
|------|------|------|
| 拥有资源的类不写析构/拷贝/移动 | 双重释放、泄漏 | Rule of Zero/Three/Five |
| 基类析构函数不是 `virtual` | 通过基类指针删除时 UB | 多态基类加 `virtual ~Base() = default;` |
| 按值传递多态对象 | 对象切片 | 用指针或引用 |
| 构造函数体里赋值成员 | 不是初始化，可能多一次赋值 | 用成员初始化列表 |
| 初始化列表顺序与声明顺序不一致 | 实际按声明顺序初始化 | 保持一致，开 `-Wreorder` |
| 单参数构造函数不加 `explicit` | 意外隐式转换 | 默认加 `explicit` |
| `std::move` 后继续使用对象 | 状态有效但未指定 | 只做赋值/销毁，或重新初始化 |
| 移动构造没标 `noexcept` | `vector` 可能选择拷贝 | 移动操作标 `noexcept` |
| 在 const 函数里随意 `mutable` | 破坏 const 语义 | 只用于缓存/计数等逻辑状态 |
| 重载 `&&`、`||`、`,` | 失去短路/顺序语义 | 不要重载这些运算符 |
| `enum` 隐式转整数 | 类型混淆 | 用 `enum class` |
| 读 `union` 的非活跃成员 | UB | 用标签或 `std::variant` |

## 7.12 语法卡片

| 术语 | 一句话定义 | 本章例子 |
|------|------------|----------|
| 类 | 用户定义的类型，包含数据和行为 | `class Buffer { ... };` |
| 构造函数 | 对象创建时运行，建立有效状态 | `Buffer(std::size_t);` |
| 析构函数 | 对象销毁时运行，释放资源 | `~Buffer();` |
| 成员初始化列表 | 在函数体前初始化成员 | `: data_(...), size_(...)` |
| RAII | 用对象生命期管理资源 | `unique_ptr` 成员 |
| 拷贝构造 | 用同类型对象初始化新对象 | `Buffer(const Buffer &);` |
| 移动构造 | 从右值对象转移资源 | `Buffer(Buffer &&) noexcept;` |
| Rule of Zero | 尽量不手写特殊成员 | 用 RAII 成员 |
| Rule of Five | 析构、拷贝、移动五件套 | 手写资源管理类 |
| `const` 成员函数 | 不修改对象可观察状态 | `size() const` |
| `mutable` | 允许 const 函数修改的成员 | `mutable int hits_;` |
| 虚函数 | 运行期动态绑定的成员函数 | `virtual double area() const;` |
| 纯虚函数 | 没有实现，派生类必须覆盖 | `= 0` |
| 对象切片 | 派生类按值赋给基类，丢失派生部分 | 避免按值传多态对象 |
| `enum class` | 强类型、作用域受限的枚举 | `enum class Color` |

## 7.13 练习

1. **Point**：写一个 `Point` 类，包含 `x`、`y`、`+=`、`==`、`<<`，并实现 `operator-` 返回两点之差。
2. **Buffer**：写一个拥有 `int` 数组的类，用 `unique_ptr<int[]>` 管理；删除拷贝，默认移动；实现 `size()` 和 `operator[]`。
3. **Tracked**：写一个类统计拷贝/移动次数，放进 `std::vector`，分别 `reserve` 和不 `reserve`，比较输出。
4. **Rule of Five**：给 `Buffer` 手写析构、拷贝构造、拷贝赋值、移动构造、移动赋值，用日志验证调用顺序。
5. **const**：给 `Buffer` 加一个 `const` 成员函数和一个 `mutable` 缓存，验证 const 对象能调用哪些函数。
6. **继承**：写 `Shape`、`Circle`、`Rectangle`，用 `vector<unique_ptr<Shape>>` 存放，计算总面积；再故意去掉 `virtual` 析构，观察警告或泄漏。
7. **切片**：写一个按值接收 `Shape` 的函数（先让 `Shape` 不是抽象类），传入 `Circle`，观察派生部分丢失。
8. **`enum class`**：定义 `enum class Color : unsigned char`，打印 `sizeof` 和 `static_cast<int>` 的值。
9. **破坏实验**：把 `Buffer` 的拷贝删除去掉，尝试拷贝，读编译错误；再把移动构造的 `noexcept` 去掉，观察 `vector` 的行为是否变化。

## 7.14 下一章

**第 8 章 · 存储期、链接与对象模型**

类让单个对象有了生命期，链接让多个翻译单元拼成一个程序。下一章会讲自动/静态/线程/动态存储期、内部/外部链接、ODR、`static`、`extern`、`inline` 变量，以及头文件与源文件的分工。

届时，为什么类的成员函数定义在头文件里要加 `inline`，为什么全局 `const` 默认是内部链接，会有完整的答案。
