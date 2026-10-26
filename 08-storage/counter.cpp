/*
 * ============================================================================
 *  C++ 语言圣经 · 第 8 章（附）
 *  另一个翻译单元
 * ============================================================================
 *
 *  这个文件与 storage.cpp 一起链接。它只通过 counter.h 了解外部世界。
 * ============================================================================
 */

#include "counter.h"

/*
 * 定义在这里，声明在 counter.h。
 * 这是外部链接：storage.cpp 也能看到同一个 shared_counter。
 */
int shared_counter = 0;

/*
 * 匿名命名空间：内部的 internal_bumps 只在这个翻译单元里有链接。
 * 效果类似 C 的 static 文件作用域变量，但更符合 C++ 的作用域规则。
 */
namespace {
int internal_bumps = 0;
}

void bump()
{
    ++shared_counter;
    ++internal_bumps;
}

int read_counter()
{
    return shared_counter;
}

int internal_bump_count()
{
    return internal_bumps;
}

int read_external_const()
{
    return external_const;
}
