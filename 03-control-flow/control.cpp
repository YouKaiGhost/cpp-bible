/*
 * ============================================================================
 *  C++ 语言圣经 · 第 3 章
 *  控制流
 * ============================================================================
 *
 *  本文件观察：
 *
 *    1. if / else 与带初始化的 if（C++17）
 *    2. switch 与 enum class
 *    3. while、do-while、for
 *    4. 范围 for
 *    5. break、continue、goto
 *
 *  编译：
 *      c++ -std=c++17 -Wall -Wextra -Wpedantic control.cpp -o control
 * ============================================================================
 */

#include <iostream>
#include <initializer_list>

namespace {

enum class Command {
    Add,
    Subtract,
    Quit
};

const char *to_text(Command command)
{
    switch (command) {
    case Command::Add:
        return "Add";
    case Command::Subtract:
        return "Subtract";
    case Command::Quit:
        return "Quit";
    }
    return "Unknown";
}

int apply(Command command, int left, int right)
{
    switch (command) {
    case Command::Add:
        return left + right;
    case Command::Subtract:
        return left - right;
    case Command::Quit:
        return 0;
    }
    return 0;
}

void show_if()
{
    int value = 42;

    std::cout << "== if ==\n";
    if (value > 0) {
        std::cout << "positive\n";
    } else if (value < 0) {
        std::cout << "negative\n";
    } else {
        std::cout << "zero\n";
    }

    // C++17 的 if 初始化语句：condition 只在这个 if 的作用域里可见。
    if (int remainder = value % 2; remainder == 0) {
        std::cout << "even\n";
    } else {
        std::cout << "odd, remainder = " << remainder << '\n';
    }
}

void show_switch()
{
    std::cout << "\n== switch ==\n";

    Command commands[] = {Command::Add, Command::Subtract, Command::Quit};
    for (Command command : commands) {
        std::cout << to_text(command) << ": "
                  << apply(command, 7, 3) << '\n';
    }

    int code = 2;
    switch (code) {
    case 1:
        std::cout << "one\n";
        break;
    case 2:
        std::cout << "two\n";
        [[fallthrough]];
    case 3:
        std::cout << "three (fallthrough)\n";
        break;
    default:
        std::cout << "other\n";
        break;
    }

    // switch 也支持初始化语句。
    switch (int n = 3; n) {
    case 3:
        std::cout << "switch init: n == 3\n";
        break;
    default:
        break;
    }
}

void show_loops()
{
    std::cout << "\n== loops ==\n";

    int i = 0;
    while (i < 3) {
        std::cout << "while " << i << '\n';
        ++i;
    }

    int j = 0;
    do {
        std::cout << "do-while " << j << '\n';
        ++j;
    } while (j < 3);

    for (int k = 0; k < 3; ++k) {
        std::cout << "for " << k << '\n';
    }

    int values[] = {10, 20, 30};
    for (int value : values) {
        std::cout << "range-for " << value << '\n';
    }

    for (int value : {1, 2, 3}) {
        std::cout << "initializer_list " << value << '\n';
    }
}

void show_jumps()
{
    std::cout << "\n== break / continue / goto ==\n";

    for (int i = 0; i < 10; ++i) {
        if (i == 2) {
            continue;
        }
        if (i == 5) {
            break;
        }
        std::cout << "i = " << i << '\n';
    }

    int attempts = 0;
retry:
    ++attempts;
    if (attempts < 3) {
        goto retry;
    }
    std::cout << "attempts = " << attempts << '\n';
}

void show_nested()
{
    std::cout << "\n== nested loops ==\n";

    for (int row = 1; row <= 3; ++row) {
        for (int column = 1; column <= 3; ++column) {
            if (row * column > 4) {
                std::cout << "  ";
                continue;
            }
            std::cout << row * column << ' ';
        }
        std::cout << '\n';
    }
}

} // namespace

int main()
{
    show_if();
    show_switch();
    show_loops();
    show_jumps();
    show_nested();
    return 0;
}
