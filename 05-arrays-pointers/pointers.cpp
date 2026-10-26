/*
 * ============================================================================
 *  C++ 语言圣经 · 第 5 章（附）
 *  指针与引用
 * ============================================================================
 *
 *  本文件观察：
 *
 *    1. 取地址与解引用
 *    2. 指针算术
 *    3. const 与指针的四种组合
 *    4. 指针的指针、数组指针
 *    5. 函数指针
 *    6. 引用与 nullptr
 *
 *  编译：
 *      c++ -std=c++17 -Wall -Wextra -Wpedantic pointers.cpp -o pointers
 * ============================================================================
 */

#include <cstddef>
#include <iostream>

namespace {

void show_address()
{
    int value = 42;
    int *pointer = &value;
    int &reference = value;

    std::cout << "== address / dereference ==\n";
    std::cout << "value = " << value << '\n';
    std::cout << "*pointer = " << *pointer << '\n';
    std::cout << "reference = " << reference << '\n';
    std::cout << "&value == pointer: " << std::boolalpha
              << (&value == pointer) << std::noboolalpha << '\n';
    std::cout << "sizeof(pointer) = " << sizeof(pointer) << '\n';
    std::cout << "sizeof(reference) = " << sizeof(reference) << '\n';

    *pointer = 100;
    std::cout << "after *pointer = 100: value = " << value << '\n';
    reference = 200;
    std::cout << "after reference = 200: value = " << value << '\n';
}

void show_arithmetic()
{
    int values[5] = {10, 20, 30, 40, 50};
    int *begin = values;
    int *end = values + 5;

    std::cout << "\n== pointer arithmetic ==\n";
    std::cout << "*begin = " << *begin << '\n';
    std::cout << "*(begin + 2) = " << *(begin + 2) << '\n';
    std::cout << "begin[3] = " << begin[3] << '\n';

    std::cout << "walk:";
    for (int *cursor = begin; cursor != end; ++cursor) {
        std::cout << ' ' << *cursor;
    }
    std::cout << '\n';

    std::cout << "end - begin = " << (end - begin) << '\n';
    std::cout << "distance in bytes = "
              << reinterpret_cast<const char *>(end)
                     - reinterpret_cast<const char *>(begin)
              << '\n';
}

void show_const()
{
    int value = 10;
    int other = 20;

    const int *pointer_to_const = &value;
    int *const const_pointer = &value;
    const int *const const_pointer_to_const = &value;

    std::cout << "\n== const and pointers ==\n";
    std::cout << "*pointer_to_const = " << *pointer_to_const << '\n';
    pointer_to_const = &other; // 指针本身可以改
    std::cout << "after rebind: " << *pointer_to_const << '\n';

    *const_pointer = 30; // 指向的值可以改
    std::cout << "after *const_pointer = 30: value = " << value << '\n';

    std::cout << "*const_pointer_to_const = "
              << *const_pointer_to_const << '\n';
}

void show_pointer_to_pointer()
{
    int value = 7;
    int *pointer = &value;
    int **pointer_to_pointer = &pointer;

    std::cout << "\n== pointer to pointer ==\n";
    std::cout << "**pointer_to_pointer = " << **pointer_to_pointer << '\n';
    **pointer_to_pointer = 8;
    std::cout << "after assignment: value = " << value << '\n';
}

void show_array_pointer()
{
    int matrix[2][3] = {
        {1, 2, 3},
        {4, 5, 6},
    };
    int (*row)[3] = matrix; // 指向“含 3 个 int 的数组”

    std::cout << "\n== pointer to array ==\n";
    std::cout << "row[1][2] = " << row[1][2] << '\n';
    std::cout << "sizeof(*row) = " << sizeof(*row) << '\n';

    const char *names[] = {"alpha", "beta", "gamma"};
    const char **cursor = names;
    std::cout << "names:";
    for (std::size_t i = 0; i < 3; ++i) {
        std::cout << ' ' << cursor[i];
    }
    std::cout << '\n';
}

int add(int left, int right)
{
    return left + right;
}

int apply(int (*operation)(int, int), int left, int right)
{
    return operation(left, right);
}

void show_function_pointer()
{
    int (*operation)(int, int) = add;
    auto lambda = [](int left, int right) {
        return left * right;
    };

    std::cout << "\n== function pointer ==\n";
    std::cout << "operation(2, 3) = " << operation(2, 3) << '\n';
    std::cout << "apply(add, 4, 5) = " << apply(add, 4, 5) << '\n';
    std::cout << "apply(lambda, 4, 5) = "
              << apply(static_cast<int (*)(int, int)>(lambda), 4, 5) << '\n';
}

void show_nullptr()
{
    int *pointer = nullptr;
    int *other = nullptr;

    std::cout << "\n== nullptr ==\n";
    std::cout << "pointer is null: " << std::boolalpha
              << (pointer == nullptr) << std::noboolalpha << '\n';
    std::cout << "two null pointers equal: " << std::boolalpha
              << (pointer == other) << std::noboolalpha << '\n';
    std::cout << "sizeof(nullptr_t) = " << sizeof(std::nullptr_t) << '\n';

    if (pointer != nullptr) {
        std::cout << *pointer << '\n';
    } else {
        std::cout << "not dereferenced\n";
    }
}

} // namespace

int main()
{
    show_address();
    show_arithmetic();
    show_const();
    show_pointer_to_pointer();
    show_array_pointer();
    show_function_pointer();
    show_nullptr();
    return 0;
}
