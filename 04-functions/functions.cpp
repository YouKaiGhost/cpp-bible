/*
 * ============================================================================
 *  C++ 语言圣经 · 第 4 章
 *  函数、重载与 lambda
 * ============================================================================
 *
 *  本文件观察：
 *
 *    1. 声明、定义与调用
 *    2. 按值、按引用、按 const 引用传参
 *    3. 重载与默认实参
 *    4. 递归与 constexpr
 *    5. 函数指针、lambda 与 std::function
 *
 *  编译：
 *      c++ -std=c++17 -Wall -Wextra -Wpedantic functions.cpp -o functions
 * ============================================================================
 */

#include <functional>
#include <iostream>
#include <string>

namespace {

int add(int left, int right)
{
    return left + right;
}

int multiply(int left, int right)
{
    return left * right;
}

const char *describe(int)
{
    return "int";
}

const char *describe(double)
{
    return "double";
}

const char *describe(const char *)
{
    return "const char *";
}

const char *describe(const std::string &)
{
    return "std::string";
}

int power(int base, int exponent = 2)
{
    int result = 1;
    for (int i = 0; i < exponent; ++i) {
        result *= base;
    }
    return result;
}

void by_value(int value)
{
    value = 999;
    std::cout << "  inside by_value: " << value << '\n';
}

void by_reference(int &value)
{
    value = 999;
    std::cout << "  inside by_reference: " << value << '\n';
}

void by_const_reference(const int &value)
{
    std::cout << "  inside by_const_reference: " << value << '\n';
}

constexpr int factorial(int n)
{
    return n <= 1 ? 1 : n * factorial(n - 1);
}

int fibonacci(int n)
{
    return n < 2 ? n : fibonacci(n - 1) + fibonacci(n - 2);
}

int &static_counter()
{
    static int value = 0;
    return value;
}

int apply(int (*operation)(int, int), int left, int right)
{
    return operation(left, right);
}

void show_calls()
{
    int value = 10;

    std::cout << "== calls ==\n";
    std::cout << "add(2, 3) = " << add(2, 3) << '\n';
    std::cout << "power(3) = " << power(3) << '\n';
    std::cout << "power(3, 4) = " << power(3, 4) << '\n';

    std::cout << "by value:\n";
    by_value(value);
    std::cout << "after by_value: " << value << '\n';

    std::cout << "by reference:\n";
    by_reference(value);
    std::cout << "after by_reference: " << value << '\n';

    std::cout << "by const reference:\n";
    by_const_reference(value);
    std::cout << "after by_const_reference: " << value << '\n';
}

void show_overloads()
{
    std::cout << "\n== overloads ==\n";
    std::cout << "describe(42)          -> " << describe(42) << '\n';
    std::cout << "describe(3.14)        -> " << describe(3.14) << '\n';
    std::cout << "describe(\"text\")      -> " << describe("text") << '\n';
    std::cout << "describe(std::string) -> "
              << describe(std::string("text")) << '\n';
}

void show_recursion()
{
    std::cout << "\n== recursion ==\n";
    std::cout << "factorial(6) = " << factorial(6) << '\n';
    static_assert(factorial(5) == 120);
    std::cout << "fibonacci(10) = " << fibonacci(10) << '\n';

    static_counter() = 41;
    ++static_counter();
    std::cout << "static_counter() = " << static_counter() << '\n';
}

void show_function_objects()
{
    std::cout << "\n== function pointers / lambdas ==\n";

    int (*operation)(int, int) = add;
    std::cout << "function pointer: " << operation(4, 5) << '\n';
    std::cout << "apply(add): " << apply(add, 4, 5) << '\n';
    std::cout << "apply(multiply): " << apply(multiply, 4, 5) << '\n';

    int offset = 10;
    auto add_offset = [offset](int value) {
        return value + offset;
    };
    std::cout << "capture by value: " << add_offset(5) << '\n';

    auto add_offset_ref = [&offset](int value) {
        return value + offset;
    };
    offset = 20;
    std::cout << "capture by reference: " << add_offset_ref(5) << '\n';

    int call_count = 0;
    auto counted = [call_count]() mutable {
        ++call_count;
        return call_count;
    };
    std::cout << "mutable lambda: " << counted() << ", " << counted() << '\n';
    std::cout << "outer call_count: " << call_count << '\n';

    auto twice = [](auto value) {
        return value + value;
    };
    std::cout << "generic lambda int: " << twice(21) << '\n';
    std::cout << "generic lambda double: " << twice(1.5) << '\n';

    std::function<int(int, int)> stored = multiply;
    std::cout << "std::function: " << stored(6, 7) << '\n';
}

} // namespace

int main()
{
    show_calls();
    show_overloads();
    show_recursion();
    show_function_objects();
    return 0;
}
