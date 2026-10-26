/*
 * ============================================================================
 *  C++ 语言圣经 · 第 8 章
 *  存储期、链接与对象模型
 * ============================================================================
 *
 *  本文件与 counter.cpp 一起编译，观察：
 *
 *    1. 自动存储、静态存储、线程存储
 *    2. 内部链接与外部链接
 *    3. static / extern / inline 的不同含义
 *    4. 文件作用域 const 的链接属性
 *    5. 静态局部变量与 ODR
 *
 *  编译：
 *      c++ -std=c++17 -Wall -Wextra -Wpedantic storage.cpp counter.cpp -o storage
 * ============================================================================
 */

#include <iostream>

#include "counter.h"

int file_scope = 100;          // 外部链接
static int file_static = 200;  // 内部链接
extern const int external_const = 42; // extern 让它获得外部链接
const int internal_const = 7;  // 没有 extern：内部链接
int zero_initialized;          // 静态存储期，零初始化
thread_local int tls_value = 0;

static inline int square(int value)
{
    return value * value;
}

static int next_local()
{
    static int value = 0; // 第一次执行时初始化，之后一直存在
    return ++value;
}

int read_file_scope()
{
    return file_scope;
}

int read_inline_shared()
{
    return inline_shared;
}

int main()
{
    std::cout << "== external linkage ==\n";
    std::cout << "file_scope = " << file_scope
              << ", external_const = " << external_const << '\n';
    std::cout << "read_file_scope() = " << read_file_scope() << '\n';
    std::cout << "read_external_const() = " << read_external_const() << '\n';
    std::cout << "inline_shared = " << inline_shared
              << ", read_inline_shared() = " << read_inline_shared() << '\n';

    std::cout << "shared_counter = " << shared_counter << '\n';
    bump();
    bump();
    std::cout << "after two bumps: shared_counter = " << shared_counter
              << '\n';
    std::cout << "read_counter() = " << read_counter()
              << ", internal_bumps = " << internal_bump_count() << '\n';

    std::cout << "\n== internal linkage ==\n";
    std::cout << "file_static = " << file_static
              << ", internal_const = " << internal_const << '\n';
    std::cout << "square(7) = " << square(7) << '\n';

    std::cout << "\n== static local ==\n";
    std::cout << "next_local: " << next_local() << ", " << next_local()
              << ", " << next_local() << '\n';

    std::cout << "\n== zero initialization ==\n";
    std::cout << "zero_initialized = " << zero_initialized << '\n';

    std::cout << "\n== thread_local ==\n";
    ++tls_value;
    ++tls_value;
    std::cout << "tls_value = " << tls_value << '\n';

    std::cout << "\n== addresses ==\n";
    std::cout << "&file_scope = " << &file_scope << '\n';
    std::cout << "&file_static = " << &file_static << '\n';
    std::cout << "&shared_counter = " << &shared_counter << '\n';

    return 0;
}
