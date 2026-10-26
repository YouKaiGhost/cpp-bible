/*
 * ============================================================================
 *  C++ 语言圣经 · 第 9 章
 *  模板与泛型
 * ============================================================================
 *
 *  本文件观察：
 *
 *    1. 函数模板与类型推导
 *    2. 类模板与非类型模板参数
 *    3. 全特化与编译期计算
 *    4. 可变参数模板与折叠表达式（C++17）
 *    5. if constexpr（C++17）与类型特征
 *    6. SFINAE 的一瞥
 *
 *  编译：
 *      c++ -std=c++17 -Wall -Wextra -Wpedantic templates.cpp -o templates
 * ============================================================================
 */

#include <cstddef>
#include <iostream>
#include <string>
#include <type_traits>

namespace {

template <typename T>
T max_value(const T &left, const T &right)
{
    return left < right ? right : left;
}

template <typename T, std::size_t N>
class FixedArray {
public:
    constexpr std::size_t size() const
    {
        return N;
    }

    T &operator[](std::size_t index)
    {
        return data_[index];
    }

    const T &operator[](std::size_t index) const
    {
        return data_[index];
    }

    T *begin()
    {
        return data_;
    }

    T *end()
    {
        return data_ + N;
    }

    const T *begin() const
    {
        return data_;
    }

    const T *end() const
    {
        return data_ + N;
    }

private:
    T data_[N]{};
};

template <typename T>
struct TypeName {
    static const char *get()
    {
        return "unknown";
    }
};

template <>
struct TypeName<int> {
    static const char *get()
    {
        return "int";
    }
};

template <>
struct TypeName<double> {
    static const char *get()
    {
        return "double";
    }
};

template <>
struct TypeName<const char *> {
    static const char *get()
    {
        return "const char *";
    }
};

template <unsigned N>
struct Factorial {
    static constexpr unsigned long long value =
        static_cast<unsigned long long>(N) * Factorial<N - 1>::value;
};

template <>
struct Factorial<0> {
    static constexpr unsigned long long value = 1;
};

template <typename... Args>
void print_all(const Args &...args)
{
    ((std::cout << args << ' '), ...);
    std::cout << '\n';
}

template <typename... Args>
constexpr auto sum_all(Args... args)
{
    return (args + ...);
}

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

template <typename T>
std::enable_if_t<std::is_integral_v<T>, T> twice(T value)
{
    return static_cast<T>(value * 2);
}

template <typename Container>
void print_container(const Container &container)
{
    std::cout << "[";
    bool first = true;
    for (const auto &value : container) {
        if (!first) {
            std::cout << ", ";
        }
        std::cout << value;
        first = false;
    }
    std::cout << "]\n";
}

void show_function_templates()
{
    std::cout << "== function templates ==\n";
    std::cout << "max_value(3, 7) = " << max_value(3, 7) << '\n';
    std::cout << "max_value(2.5, 1.5) = " << max_value(2.5, 1.5) << '\n';
    std::cout << "max_value<std::string> = "
              << max_value(std::string("alpha"), std::string("beta")) << '\n';

    static_assert(std::is_same_v<decltype(max_value(3, 7)), int>);
    static_assert(std::is_same_v<decltype(max_value(2.5, 1.5)), double>);
}

void show_class_templates()
{
    FixedArray<int, 5> values{};
    for (std::size_t i = 0; i < values.size(); ++i) {
        values[i] = static_cast<int>(i * i);
    }

    std::cout << "\n== class templates ==\n";
    std::cout << "size = " << values.size() << '\n';
    print_container(values);

    const FixedArray<int, 5> &view = values;
    std::cout << "const view[3] = " << view[3] << '\n';
    static_assert(sizeof(values) >= sizeof(int) * 5);
}

void show_specialization()
{
    std::cout << "\n== specialization ==\n";
    std::cout << "TypeName<int>::get() = " << TypeName<int>::get() << '\n';
    std::cout << "TypeName<double>::get() = " << TypeName<double>::get()
              << '\n';
    std::cout << "TypeName<const char *>::get() = "
              << TypeName<const char *>::get() << '\n';
    std::cout << "TypeName<long>::get() = " << TypeName<long>::get() << '\n';
    std::cout << "Factorial<5>::value = " << Factorial<5>::value << '\n';
    static_assert(Factorial<5>::value == 120);
}

void show_variadic()
{
    std::cout << "\n== variadic templates ==\n";
    print_all(1, 2.5, "three", '4');
    print_all();
    std::cout << "sum_all(1, 2, 3) = " << sum_all(1, 2, 3) << '\n';
    std::cout << "sum_all(1.5, 2.5, 3.0) = "
              << sum_all(1.5, 2.5, 3.0) << '\n';

    static_assert(sum_all(1, 2, 3) == 6);
}

void show_type_traits()
{
    std::cout << "\n== type traits / if constexpr ==\n";
    std::cout << "classify(42) = " << classify(42) << '\n';
    std::cout << "classify(2.5) = " << classify(2.5) << '\n';
    std::cout << "classify(\"text\") = " << classify("text") << '\n';
    std::cout << "twice(21) = " << twice(21) << '\n';

    static_assert(std::is_integral_v<int>);
    static_assert(std::is_floating_point_v<double>);
    static_assert(!std::is_integral_v<std::string>);
}

} // namespace

int main()
{
    show_function_templates();
    show_class_templates();
    show_specialization();
    show_variadic();
    show_type_traits();
    return 0;
}
