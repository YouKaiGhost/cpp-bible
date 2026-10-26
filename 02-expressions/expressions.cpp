/*
 * ============================================================================
 *  C++ 语言圣经 · 第 2 章
 *  表达式、运算符与求值
 * ============================================================================
 *
 *  本文件把类型放进表达式里，观察：
 *
 *    1. 算术、关系、逻辑、位运算
 *    2. 值类别：lvalue、prvalue、xvalue
 *    3. 优先级与结合性
 *    4. 短路求值
 *    5. 自增、赋值与求值顺序
 *    6. 类型转换
 *
 *  编译：
 *      c++ -std=c++17 -Wall -Wextra -Wpedantic expressions.cpp -o expressions
 * ============================================================================
 */

#include <iomanip>
#include <iostream>
#include <type_traits>
#include <utility>

namespace {

int trace(const char *label, int value)
{
    std::cout << "[" << label << "] ";
    return value;
}

void show_arithmetic()
{
    int a = 7;
    int b = 3;
    double d = 3.9;

    std::cout << "== arithmetic ==\n";
    std::cout << "a + b = " << a + b << '\n';
    std::cout << "a - b = " << a - b << '\n';
    std::cout << "a * b = " << a * b << '\n';
    std::cout << "a / b = " << a / b << '\n';
    std::cout << "a % b = " << a % b << '\n';
    std::cout << "-7 / 3 = " << -7 / 3 << '\n';
    std::cout << "-7 % 3 = " << -7 % 3 << '\n';
    std::cout << "static_cast<double>(a) / b = "
              << static_cast<double>(a) / b << '\n';
    std::cout << "static_cast<int>(d) = " << static_cast<int>(d) << '\n';
    std::cout << "static_cast<unsigned char>(300) = "
              << static_cast<unsigned>(static_cast<unsigned char>(300))
              << '\n';
}

void show_compare_and_logic()
{
    int a = 3;
    int b = 5;
    bool side_effect = false;

    std::cout << "\n== compare and logic ==\n";
    std::cout << std::boolalpha;
    std::cout << "a < b  = " << (a < b) << '\n';
    std::cout << "a == b = " << (a == b) << '\n';
    std::cout << "a != b = " << (a != b) << '\n';
    std::cout << "!a     = " << !a << '\n';

    std::cout << "2 && 4 = " << (2 && 4) << '\n';
    std::cout << "2 & 4  = " << (2 & 4) << '\n';
    std::cout << "2 || 4 = " << (2 || 4) << '\n';
    std::cout << "2 | 4  = " << (2 | 4) << '\n';

    bool first = false && (side_effect = true);
    bool second = true || (side_effect = true);
    std::cout << "false && (...) = " << first << '\n';
    std::cout << "true  || (...) = " << second << '\n';
    std::cout << "side_effect    = " << side_effect << '\n';

    std::cout << std::noboolalpha;
}

void show_bitwise()
{
    unsigned x = 0x0Fu;
    unsigned y = 0x3Cu;

    std::cout << "\n== bitwise ==\n";
    std::cout << std::hex << std::showbase;
    std::cout << "x       = " << x << '\n';
    std::cout << "y       = " << y << '\n';
    std::cout << "x & y   = " << (x & y) << '\n';
    std::cout << "x | y   = " << (x | y) << '\n';
    std::cout << "x ^ y   = " << (x ^ y) << '\n';
    std::cout << "~x      = " << ~x << '\n';
    std::cout << "x << 2  = " << (x << 2) << '\n';
    std::cout << "y >> 2  = " << (y >> 2) << '\n';
    std::cout << std::dec << std::noshowbase;
}

void show_assignment()
{
    int i = 5;
    int j = ++i;
    int k = i++;
    int m = 10;

    std::cout << "\n== assignment ==\n";
    std::cout << "i = " << i << ", j = " << j << ", k = " << k << '\n';
    m += 5;
    std::cout << "m += 5  -> " << m << '\n';
    m -= 3;
    std::cout << "m -= 3  -> " << m << '\n';
    m *= 2;
    std::cout << "m *= 2  -> " << m << '\n';
    m /= 4;
    std::cout << "m /= 4  -> " << m << '\n';

    int p = 0;
    int q = 0;
    p = q = 7; // 赋值是右结合的
    std::cout << "p = q = 7 -> p = " << p << ", q = " << q << '\n';
}

void show_conditional_and_comma()
{
    int a = 12;
    int b = 30;

    std::cout << "\n== conditional / comma ==\n";
    std::cout << "(a > b ? a : b) = " << (a > b ? a : b) << '\n';

    int sequence = (trace("first", 1), trace("second", 2));
    std::cout << "comma result = " << sequence << '\n';
}

void show_value_categories()
{
    int x = 1;
    int y = 2;

    std::cout << "\n== value categories ==\n";
    std::cout << std::boolalpha;
    std::cout << "decltype((x)) is int&      : "
              << std::is_same_v<decltype((x)), int &> << '\n';
    std::cout << "decltype(x) is int         : "
              << std::is_same_v<decltype(x), int> << '\n';
    std::cout << "decltype(42) is int        : "
              << std::is_same_v<decltype(42), int> << '\n';
    std::cout << "decltype(std::move(x)) is int&& : "
              << std::is_same_v<decltype(std::move(x)), int &&> << '\n';
    std::cout << std::noboolalpha;

    int &alias = x;
    alias = 10;
    std::cout << "alias = 10 -> x = " << x << '\n';

    int &&rvalue_ref = y + 3;
    std::cout << "rvalue_ref = " << rvalue_ref << '\n';
}

void show_precedence()
{
    int a = 2;
    int b = 3;
    int c = 4;

    std::cout << "\n== precedence ==\n";
    std::cout << "a + b * c       = " << a + b * c << '\n';
    std::cout << "(a + b) * c     = " << (a + b) * c << '\n';
    std::cout << "a < b && b < c  = " << (a < b && b < c) << '\n';
    // + 比 << 结合得紧；这里显式加括号，避免 -Wparentheses 警告。
    std::cout << "a << (1 + 1)    = " << (a << (1 + 1)) << '\n';
    std::cout << "(a << 1) + 1    = " << ((a << 1) + 1) << '\n';
    // == 比 & 结合得紧；不写括号会变成 a & (1 == 0)。
    std::cout << "(a & 1) == 0    = " << ((a & 1) == 0) << '\n';
}

void show_evaluation_order()
{
    std::cout << "\n== evaluation order ==\n";
    std::cout << "chain: "
              << trace("left", 1)
              << trace("right", 2) << '\n';

    int value = 0;
    value = trace("rhs", 7);
    std::cout << "assignment result = " << value << '\n';
}

} // namespace

int main()
{
    show_arithmetic();
    show_compare_and_logic();
    show_bitwise();
    show_assignment();
    show_conditional_and_comma();
    show_value_categories();
    show_precedence();
    show_evaluation_order();
    return 0;
}
