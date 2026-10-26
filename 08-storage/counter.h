#ifndef COUNTER_H
#define COUNTER_H

/*
 * shared_counter 在 counter.cpp 中定义，在这里只声明。
 * 这是外部链接：别的翻译单元也能访问同一个对象。
 */
extern int shared_counter;

/*
 * external_const 在 storage.cpp 中用 extern 定义。
 * C++ 的文件作用域 const 默认是内部链接；加 extern 才改成外部链接。
 */
extern const int external_const;

/*
 * C++17 的 inline 变量：头文件里可以给出定义，
 * 所有包含它的翻译单元看到的是同一个对象。
 */
inline int inline_shared = 7;

void bump();
int read_counter();
int internal_bump_count();

/*
 * 下面几个函数在另一个翻译单元里读取 storage.cpp 定义的对象，
 * 用来演示跨翻译单元的外部链接。
 */
int read_file_scope();
int read_external_const();
int read_inline_shared();

#endif
