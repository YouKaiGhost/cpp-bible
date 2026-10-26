/*
 * ============================================================================
 *  C++ 语言圣经 · 第 4 章（附）
 *  main 的参数：命令行
 * ============================================================================
 *
 *  编译：
 *      c++ -std=c++17 -Wall -Wextra -Wpedantic args.cpp -o args
 *
 *  运行：
 *      ./args
 *      ./args alpha beta 42
 * ============================================================================
 */

#include <exception>
#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
    std::cout << "argc = " << argc << '\n';

    for (int i = 0; i < argc; ++i) {
        std::cout << "argv[" << i << "] = " << argv[i] << '\n';
    }

    if (argc == 1) {
        std::cout << "usage: args [arguments...]\n";
        return 0;
    }

    std::cout << "arguments:";
    for (int i = 1; i < argc; ++i) {
        std::cout << ' ' << argv[i];
    }
    std::cout << '\n';

    // 把最后一个参数当作整数；失败时不抛异常，而是返回 false。
    try {
        int value = std::stoi(argv[argc - 1]);
        std::cout << "last argument as int: " << value << '\n';
    } catch (const std::exception &error) {
        std::cout << "last argument is not an int: " << error.what() << '\n';
    }

    return 0;
}
