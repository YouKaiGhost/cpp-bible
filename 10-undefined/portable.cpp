/*
 * ============================================================================
 *  C++ 语言圣经 · 第 10 章
 *  未定义行为与可移植性
 * ============================================================================
 *
 *  本文件只演示“有定义”的行为，并指出邻近的 UB 边界：
 *
 *    1. 数值范围与浮点表示
 *    2. char 的有符号性
 *    3. 填充、对齐与布局
 *    4. 用 memcpy 观察对象表示
 *    5. 浮点比较
 *
 *  编译：
 *      c++ -std=c++17 -Wall -Wextra -Wpedantic portable.cpp -o portable
 *
 *  可选：
 *      c++ -std=c++17 -g -fsanitize=address,undefined portable.cpp -o portable
 * ============================================================================
 */

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <limits>
#include <type_traits>

namespace {

struct Layout {
    char tag;
    int value;
    char tail;
};

struct alignas(16) Aligned {
    char bytes[16];
};

void show_limits()
{
    std::cout << "== limits ==\n";
    std::cout << "int min      : " << std::numeric_limits<int>::min() << '\n';
    std::cout << "int max      : " << std::numeric_limits<int>::max() << '\n';
    std::cout << "unsigned max : " << std::numeric_limits<unsigned>::max()
              << '\n';
    std::cout << "double digits: " << std::numeric_limits<double>::digits10
              << '\n';
    std::cout << "double eps   : " << std::setprecision(17)
              << std::numeric_limits<double>::epsilon() << std::setprecision(6)
              << '\n';
    std::cout << "iec559       : " << std::boolalpha
              << std::numeric_limits<double>::is_iec559 << std::noboolalpha
              << '\n';
}

void show_char_signedness()
{
    char value = static_cast<char>(255);
    std::cout << "\n== char signedness ==\n";
    std::cout << "is_signed<char> = " << std::boolalpha
              << std::is_signed_v<char> << std::noboolalpha << '\n';
    std::cout << "static_cast<int>(char(255)) = "
              << static_cast<int>(value) << '\n';
}

void show_layout()
{
    std::cout << "\n== layout ==\n";
    std::cout << "sizeof(Layout) = " << sizeof(Layout) << '\n';
    std::cout << "offsetof(tag) = " << offsetof(Layout, tag) << '\n';
    std::cout << "offsetof(value) = " << offsetof(Layout, value) << '\n';
    std::cout << "offsetof(tail) = " << offsetof(Layout, tail) << '\n';
    std::cout << "alignof(Layout) = " << alignof(Layout) << '\n';

    std::cout << "sizeof(Aligned) = " << sizeof(Aligned) << '\n';
    std::cout << "alignof(Aligned) = " << alignof(Aligned) << '\n';
}

void show_object_representation()
{
    float value = 3.5f;
    std::uint32_t bits = 0;
    static_assert(sizeof(float) >= sizeof(bits));

    std::memcpy(&bits, &value, sizeof(bits));

    std::cout << "\n== object representation ==\n";
    std::cout << "float value = " << value << '\n';
    std::cout << "bits (hex)  = " << std::hex << bits << std::dec << '\n';
    std::cout << "sizeof(float) = " << sizeof(float) << '\n';
}

void show_float_comparison()
{
    double sum = 0.1 + 0.2;
    double difference = std::abs(sum - 0.3);

    std::cout << "\n== floating point ==\n";
    std::cout << std::setprecision(17);
    std::cout << "0.1 + 0.2 = " << sum << '\n';
    std::cout << "0.3       = " << 0.3 << '\n';
    std::cout << "sum == 0.3: " << std::boolalpha << (sum == 0.3)
              << std::noboolalpha << '\n';
    std::cout << "difference = " << difference << '\n';
    std::cout << "close enough: " << std::boolalpha
              << (difference < std::numeric_limits<double>::epsilon() * 8)
              << std::noboolalpha << std::setprecision(6) << '\n';
}

void show_defined_behavior()
{
    int values[3] = {10, 20, 30};
    int *begin = values;
    int *end = values + 3; // 尾后指针可以形成和比较，不能解引用

    std::cout << "\n== defined boundaries ==\n";
    std::cout << "end - begin = " << (end - begin) << '\n';
    std::cout << "values[2] = " << values[2] << '\n';
    std::cout << "null check: " << std::boolalpha
              << (begin != nullptr) << std::noboolalpha << '\n';

    // 下面这些都是 UB，不要在程序里执行：
    // int overflow = std::numeric_limits<int>::max() + 1;
    // int uninitialized;
    // std::cout << uninitialized;
    // std::cout << values[3];
    // int *dangling = nullptr;
    // std::cout << *dangling;
}

} // namespace

int main()
{
    show_limits();
    show_char_signedness();
    show_layout();
    show_object_representation();
    show_float_comparison();
    show_defined_behavior();
    return 0;
}
