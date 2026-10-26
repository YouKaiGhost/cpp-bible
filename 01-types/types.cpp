/*
 * ============================================================================
 *  C++ 语言圣经 · 第 1 章
 *  对象、类型与声明
 * ============================================================================
 *
 *  本文件回答五个问题：
 *
 *    1. 一个“对象”是什么？
 *    2. C++ 的基本类型都保证了什么？
 *    3. auto、const、constexpr 分别做了什么？
 *    4. 引用和指针差在哪里？
 *    5. sizeof 与 alignof 测量了什么？
 *
 *  编译：
 *      c++ -std=c++17 -Wall -Wextra -Wpedantic types.cpp -o types
 * ============================================================================
 */

#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <type_traits>

namespace {

void show_sizes()
{
    std::cout << "== sizeof / alignof ==\n";
    std::cout << "bool          : " << sizeof(bool)
              << " / " << alignof(bool) << '\n';
    std::cout << "char          : " << sizeof(char)
              << " / " << alignof(char) << '\n';
    std::cout << "short         : " << sizeof(short)
              << " / " << alignof(short) << '\n';
    std::cout << "int           : " << sizeof(int)
              << " / " << alignof(int) << '\n';
    std::cout << "long          : " << sizeof(long)
              << " / " << alignof(long) << '\n';
    std::cout << "long long     : " << sizeof(long long)
              << " / " << alignof(long long) << '\n';
    std::cout << "float         : " << sizeof(float)
              << " / " << alignof(float) << '\n';
    std::cout << "double        : " << sizeof(double)
              << " / " << alignof(double) << '\n';
    std::cout << "long double   : " << sizeof(long double)
              << " / " << alignof(long double) << '\n';
    std::cout << "void *        : " << sizeof(void *)
              << " / " << alignof(void *) << '\n';
    std::cout << "std::size_t   : " << sizeof(std::size_t)
              << " / " << alignof(std::size_t) << '\n';
    std::cout << "std::ptrdiff_t: " << sizeof(std::ptrdiff_t)
              << " / " << alignof(std::ptrdiff_t) << '\n';
    std::cout << "std::byte     : " << sizeof(std::byte)
              << " / " << alignof(std::byte) << '\n';
}

void show_ranges()
{
    std::cout << "\n== ranges ==\n";
    std::cout << "char      : "
              << static_cast<int>(std::numeric_limits<char>::min()) << " .. "
              << static_cast<int>(std::numeric_limits<char>::max()) << '\n';
    std::cout << "int       : " << std::numeric_limits<int>::min() << " .. "
              << std::numeric_limits<int>::max() << '\n';
    std::cout << "unsigned  : 0 .. " << std::numeric_limits<unsigned>::max()
              << '\n';
    std::cout << "long long : " << std::numeric_limits<long long>::min()
              << " .. " << std::numeric_limits<long long>::max() << '\n';
    std::cout << "float dig : " << std::numeric_limits<float>::digits10 << '\n';
    std::cout << "double dig: " << std::numeric_limits<double>::digits10 << '\n';
    std::cout << "double eps: " << std::setprecision(17)
              << std::numeric_limits<double>::epsilon() << std::setprecision(6)
              << '\n';
    std::cout << "iec559    : " << std::boolalpha
              << std::numeric_limits<double>::is_iec559 << std::noboolalpha
              << '\n';
}

void show_literals()
{
    std::cout << "\n== literals ==\n";
    constexpr int decimal = 42;
    constexpr int hexadecimal = 0x2A;
    constexpr int octal = 052;
    constexpr int binary = 0b101010;
    constexpr long long separated = 1'000'000;
    constexpr double scientific = 1e3;
    constexpr float single = 0.5f;

    std::cout << "42        : " << decimal << '\n';
    std::cout << "0x2A      : " << hexadecimal << '\n';
    std::cout << "052       : " << octal << '\n';
    std::cout << "0b101010  : " << binary << '\n';
    std::cout << "1'000'000 : " << separated << '\n';
    std::cout << "1e3       : " << scientific << '\n';
    std::cout << "0.5f      : " << single << '\n';
    std::cout << "'A'       : " << 'A' << '\n';
    std::cout << "sizeof('A'): " << sizeof('A') << '\n';
    std::cout << "u8\"C++\"    : " << u8"C++" << '\n';
}

void show_initialization()
{
    std::cout << "\n== initialization ==\n";

    int a = 10;         // 复制初始化
    int b(20);          // 直接初始化
    int c{30};          // 列表初始化（C++11）
    int d{};            // 值初始化：d == 0
    auto e = 40;        // 推导为 int
    auto f = 1.5;       // 推导为 double
    constexpr int g = 50;
    decltype(g) h = 60; // h 的类型与 g 相同：const int

    std::cout << "a..h = " << a << ' ' << b << ' ' << c << ' ' << d << ' '
              << e << ' ' << f << ' ' << g << ' ' << h << '\n';

    const int answer = 42;
    const int &ref = answer; // 引用必须绑定到对象，且不能重新绑定
    const int *ptr = &answer;

    std::cout << "answer = " << answer << ", ref = " << ref
              << ", *ptr = " << *ptr << '\n';
    std::cout << "sizeof(ref) = " << sizeof(ref) << '\n';
    std::cout << "&ref == &answer: " << std::boolalpha
              << (&ref == &answer) << std::noboolalpha << '\n';

    // 列表初始化会拒绝窄化转换；下面两行无法通过编译：
    // int narrowed{1.5};
    // char too_big{300};

    static_assert(sizeof(char) == 1, "sizeof(char) is always 1");
    static_assert(sizeof(int) >= sizeof(short), "int is at least as wide");
    static_assert(std::is_same_v<decltype(e), int>);
    static_assert(std::is_same_v<decltype(f), double>);
    static_assert(std::is_same_v<decltype(g), const int>);
    std::cout << "static assertions passed\n";
}

void show_conversions()
{
    std::cout << "\n== conversions ==\n";

    double d = 3.9;
    int truncated = static_cast<int>(d);
    unsigned wrapped = 0u - 1u;
    int negative = -1;
    unsigned converted = static_cast<unsigned>(negative);

    std::cout << "static_cast<int>(3.9) = " << truncated << '\n';
    std::cout << "0u - 1u               = " << wrapped << '\n';
    std::cout << "(unsigned)-1          = " << converted << '\n';
    std::cout << "(double)7 / 2         = "
              << static_cast<double>(7) / 2 << '\n';
    std::cout << "7 / 2                 = " << 7 / 2 << '\n';

    bool flag = 42;
    std::cout << "bool flag = 42        : " << flag << '\n';
}

} // namespace

int main()
{
    show_sizes();
    show_ranges();
    show_literals();
    show_initialization();
    show_conversions();
    return 0;
}
