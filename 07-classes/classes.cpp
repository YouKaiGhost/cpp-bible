/*
 * ============================================================================
 *  C++ 语言圣经 · 第 7 章
 *  类、构造与资源管理
 * ============================================================================
 *
 *  本文件观察：
 *
 *    1. class 与 struct 的访问控制
 *    2. 构造函数、成员初始化列表、析构函数
 *    3. RAII 与 unique_ptr
 *    4. 拷贝、移动与三五法则
 *    5. const 成员函数与 mutable
 *    6. 运算符重载
 *    7. 继承、虚函数与多态
 *
 *  编译：
 *      c++ -std=c++17 -Wall -Wextra -Wpedantic classes.cpp -o classes
 * ============================================================================
 */

#include <cstddef>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

namespace {

class Buffer {
public:
    explicit Buffer(std::size_t size)
        : data_(std::make_unique<int[]>(size)), size_(size)
    {
        for (std::size_t i = 0; i < size_; ++i) {
            data_[i] = static_cast<int>(i);
        }
    }

    ~Buffer() = default;

    // 这个类拥有独占资源，因此拷贝被删除，移动被允许。
    Buffer(const Buffer &) = delete;
    Buffer &operator=(const Buffer &) = delete;
    Buffer(Buffer &&) noexcept = default;
    Buffer &operator=(Buffer &&) noexcept = default;

    std::size_t size() const
    {
        return size_;
    }

    int &operator[](std::size_t index)
    {
        return data_[index];
    }

    const int &operator[](std::size_t index) const
    {
        return data_[index];
    }

private:
    std::unique_ptr<int[]> data_;
    std::size_t size_;
};

class Tracked {
public:
    explicit Tracked(int value) : value_(value)
    {
    }

    Tracked(const Tracked &other) : value_(other.value_)
    {
        ++copies_;
    }

    Tracked(Tracked &&other) noexcept : value_(other.value_)
    {
        ++moves_;
    }

    Tracked &operator=(const Tracked &other)
    {
        value_ = other.value_;
        ++copies_;
        return *this;
    }

    Tracked &operator=(Tracked &&other) noexcept
    {
        value_ = other.value_;
        ++moves_;
        return *this;
    }

    ~Tracked() = default;

    int value() const
    {
        return value_;
    }

    static int copies()
    {
        return copies_;
    }

    static int moves()
    {
        return moves_;
    }

private:
    int value_;
    static int copies_;
    static int moves_;
};

int Tracked::copies_ = 0;
int Tracked::moves_ = 0;

class Counter {
public:
    void hit() const
    {
        ++hits_; // mutable 成员可以在 const 成员函数里修改
    }

    int hits() const
    {
        return hits_;
    }

private:
    mutable int hits_ = 0;
};

class Point {
public:
    Point(int x, int y) : x_(x), y_(y)
    {
    }

    int x() const
    {
        return x_;
    }

    int y() const
    {
        return y_;
    }

    Point &operator+=(const Point &other)
    {
        x_ += other.x_;
        y_ += other.y_;
        return *this;
    }

    friend bool operator==(const Point &left, const Point &right)
    {
        return left.x_ == right.x_ && left.y_ == right.y_;
    }

    friend std::ostream &operator<<(std::ostream &out, const Point &point)
    {
        return out << '(' << point.x_ << ", " << point.y_ << ')';
    }

private:
    int x_;
    int y_;
};

class Shape {
public:
    virtual ~Shape() = default;
    virtual double area() const = 0;
    virtual const char *name() const = 0;
};

class Circle final : public Shape {
public:
    explicit Circle(double radius) : radius_(radius)
    {
    }

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

class Rectangle final : public Shape {
public:
    Rectangle(double width, double height)
        : width_(width), height_(height)
    {
    }

    double area() const override
    {
        return width_ * height_;
    }

    const char *name() const override
    {
        return "Rectangle";
    }

private:
    double width_;
    double height_;
};

enum class Color {
    Red,
    Green,
    Blue
};

const char *to_text(Color color)
{
    switch (color) {
    case Color::Red:
        return "Red";
    case Color::Green:
        return "Green";
    case Color::Blue:
        return "Blue";
    }
    return "Unknown";
}

void show_raii()
{
    std::cout << "== RAII / move ==\n";

    Buffer first(4);
    std::cout << "first:";
    for (std::size_t i = 0; i < first.size(); ++i) {
        std::cout << ' ' << first[i];
    }
    std::cout << '\n';

    Buffer second = std::move(first);
    second[2] = 99;
    std::cout << "second[2] = " << second[2] << '\n';
    std::cout << "second.size() = " << second.size() << '\n';

    // Buffer copy = second; // 错误：拷贝构造被删除
    std::cout << "resource released at scope exit\n";
}

void show_special_members()
{
    std::cout << "\n== copy / move ==\n";

    {
        std::vector<Tracked> items;
        items.reserve(3);
        items.push_back(Tracked(1));
        items.push_back(Tracked(2));
        items.push_back(Tracked(3));
        items.push_back(items[0]);
        std::cout << "after pushes: copies = " << Tracked::copies()
                  << ", moves = " << Tracked::moves() << '\n';
    }

    std::cout << "after scope: copies = " << Tracked::copies()
              << ", moves = " << Tracked::moves() << '\n';
}

void show_const_members()
{
    const Counter counter;
    counter.hit();
    counter.hit();

    std::cout << "\n== const member / mutable ==\n";
    std::cout << "hits = " << counter.hits() << '\n';
}

void show_operators()
{
    Point point(1, 2);
    point += Point(3, 4);

    std::cout << "\n== operators ==\n";
    std::cout << "point = " << point << '\n';
    std::cout << "point == Point(4, 6): " << std::boolalpha
              << (point == Point(4, 6)) << std::noboolalpha << '\n';
}

void show_polymorphism()
{
    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.push_back(std::make_unique<Circle>(2.0));
    shapes.push_back(std::make_unique<Rectangle>(3.0, 4.0));

    std::cout << "\n== polymorphism ==\n";
    double total = 0.0;
    for (const auto &shape : shapes) {
        std::cout << shape->name() << " area = " << shape->area() << '\n';
        total += shape->area();
    }
    std::cout << "total area = " << total << '\n';
}

void show_enum()
{
    Color colors[] = {Color::Red, Color::Green, Color::Blue};

    std::cout << "\n== enum class ==\n";
    for (Color color : colors) {
        std::cout << to_text(color) << ' ';
    }
    std::cout << '\n';
    std::cout << "sizeof(Color) = " << sizeof(Color) << '\n';
}

} // namespace

int main()
{
    show_raii();
    show_special_members();
    show_const_members();
    show_operators();
    show_polymorphism();
    show_enum();
    return 0;
}
