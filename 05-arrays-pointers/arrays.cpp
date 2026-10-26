/*
 * ============================================================================
 *  C++ 语言圣经 · 第 5 章
 *  数组与 std::array
 * ============================================================================
 *
 *  本文件观察：
 *
 *    1. 原生数组的声明、初始化与遍历
 *    2. 数组到指针的退化
 *    3. std::array 与 std::vector
 *    4. 二维数组
 *    5. 结构化绑定
 *
 *  编译：
 *      c++ -std=c++17 -Wall -Wextra -Wpedantic arrays.cpp -o arrays
 * ============================================================================
 */

#include <algorithm>
#include <array>
#include <cstddef>
#include <iostream>
#include <vector>

namespace {

template <typename T, std::size_t N>
constexpr std::size_t length(const T (&)[N])
{
    return N;
}

void print_array(const int *data, std::size_t size)
{
    std::cout << "[";
    for (std::size_t i = 0; i < size; ++i) {
        if (i != 0) {
            std::cout << ", ";
        }
        std::cout << data[i];
    }
    std::cout << "]\n";
}

void show_native_arrays()
{
    int values[5] = {10, 20, 30, 40, 50};
    int zeros[5] = {};
    int deduced[] = {1, 2, 3};

    std::cout << "== native arrays ==\n";
    std::cout << "values: ";
    print_array(values, length(values));

    std::cout << "zeros: ";
    print_array(zeros, length(zeros));

    std::cout << "deduced: ";
    print_array(deduced, length(deduced));

    values[1] = 21;
    std::cout << "after values[1] = 21: ";
    print_array(values, length(values));

    int sum = 0;
    for (int value : values) {
        sum += value;
    }
    std::cout << "sum = " << sum << '\n';
}

void show_decay()
{
    int values[5] = {1, 2, 3, 4, 5};

    std::cout << "\n== decay ==\n";
    std::cout << "sizeof(values) = " << sizeof(values) << '\n';
    std::cout << "length(values) = " << length(values) << '\n';

    const int *pointer = values; // 数组名退化为指向首元素的指针
    std::cout << "sizeof(pointer) = " << sizeof(pointer) << '\n';
    std::cout << "pointer[2] = " << pointer[2] << '\n';

    // 下面这行在函数参数里拿不到数组长度：
    // std::cout << sizeof(values) / sizeof(values[0]);
}

void show_std_array()
{
    std::array<int, 5> values = {5, 1, 4, 2, 3};

    std::cout << "\n== std::array ==\n";
    std::cout << "size = " << values.size() << '\n';
    std::cout << "front = " << values.front()
              << ", back = " << values.back() << '\n';

    std::sort(values.begin(), values.end());
    std::cout << "sorted:";
    for (int value : values) {
        std::cout << ' ' << value;
    }
    std::cout << '\n';

    auto [first, second, third, fourth, fifth] = values;
    std::cout << "structured binding: " << first << ' ' << second << ' '
              << third << ' ' << fourth << ' ' << fifth << '\n';
}

void show_vector()
{
    std::vector<int> values = {3, 1, 4};
    values.push_back(1);
    values.push_back(5);

    std::cout << "\n== std::vector ==\n";
    std::cout << "size = " << values.size()
              << ", capacity = " << values.capacity() << '\n';
    std::cout << "values:";
    for (int value : values) {
        std::cout << ' ' << value;
    }
    std::cout << '\n';

    const int *data = values.data();
    std::cout << "data[2] = " << data[2] << '\n';
}

void show_matrix()
{
    int matrix[3][4] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12},
    };

    std::cout << "\n== 2D array ==\n";
    for (auto &row : matrix) {
        for (int value : row) {
            std::cout << value << '\t';
        }
        std::cout << '\n';
    }

    std::cout << "sizeof(matrix) = " << sizeof(matrix) << '\n';
    std::cout << "sizeof(matrix[0]) = " << sizeof(matrix[0]) << '\n';
    std::cout << "rows = " << sizeof(matrix) / sizeof(matrix[0]) << '\n';
    std::cout << "columns = " << sizeof(matrix[0]) / sizeof(matrix[0][0])
              << '\n';
}

} // namespace

int main()
{
    show_native_arrays();
    show_decay();
    show_std_array();
    show_vector();
    show_matrix();
    return 0;
}
