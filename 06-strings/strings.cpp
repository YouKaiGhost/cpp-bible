/*
 * ============================================================================
 *  C++ 语言圣经 · 第 6 章
 *  字符、字符串与视图
 * ============================================================================
 *
 *  本文件观察：
 *
 *    1. char、字符数组与空字符结尾
 *    2. std::string 的常用操作
 *    3. std::string_view：只读、非拥有
 *    4. 原始字符串与字符串字面量拼接
 *    5. 数值与字符串互转
 *    6. UTF-8 字节序列
 *
 *  编译：
 *      c++ -std=c++17 -Wall -Wextra -Wpedantic strings.cpp -o strings
 * ============================================================================
 */

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace {

void show_c_strings()
{
    char greeting[] = "hello";
    const char *pointer = "world";

    std::cout << "== C strings ==\n";
    std::cout << "greeting = " << greeting << '\n';
    std::cout << "pointer  = " << pointer << '\n';
    std::cout << "sizeof(greeting) = " << sizeof(greeting) << '\n';
    std::cout << "strlen(greeting) = " << std::strlen(greeting) << '\n';
    std::cout << "greeting[4] = " << greeting[4]
              << ", greeting[5] = " << static_cast<int>(greeting[5]) << '\n';

    char copy[16] = {};
    std::strcpy(copy, greeting);
    std::cout << "copy = " << copy << '\n';
    std::cout << "strcmp(greeting, copy) = "
              << std::strcmp(greeting, copy) << '\n';
}

void show_std_string()
{
    std::string text = "C++";
    text += " 语言";
    text.append("圣经");

    std::cout << "\n== std::string ==\n";
    std::cout << "text = " << text << '\n';
    std::cout << "size = " << text.size()
              << ", capacity = " << text.capacity() << '\n';
    std::cout << "empty = " << std::boolalpha << text.empty()
              << std::noboolalpha << '\n';

    std::size_t position = text.find("语言");
    std::cout << "find(\"语言\") = " << position << '\n';
    if (position != std::string::npos) {
        std::cout << "substr = " << text.substr(position, 6) << '\n';
    }

    text.replace(0, 3, "ISO C++");
    std::cout << "after replace = " << text << '\n';
    std::cout << "front = " << text.front()
              << ", back byte = "
              << static_cast<int>(static_cast<unsigned char>(text.back()))
              << '\n';
    std::cout << "c_str = " << text.c_str() << '\n';
}

void show_string_view()
{
    std::string owned = "one,two,three,four";
    std::string_view view = owned;

    std::cout << "\n== std::string_view ==\n";
    std::cout << "view = " << view << '\n';
    std::cout << "size = " << view.size() << '\n';

    std::string_view first = view.substr(0, view.find(','));
    std::cout << "first field = " << first << '\n';

    view.remove_prefix(first.size() + 1);
    std::cout << "after remove_prefix = " << view << '\n';

    std::string_view suffix = view.substr(view.rfind(',') + 1);
    std::cout << "last field = " << suffix << '\n';

    // string_view 不拥有字符；owned 仍然活着，所以这些视图有效。
    std::string materialized(suffix);
    std::cout << "materialized size = " << materialized.size() << '\n';
}

void show_literals()
{
    const char *raw = R"(C:\code\cpp "quoted")";
    const char *joined = "Hello, "
                         "World!";

    std::cout << "\n== literals ==\n";
    std::cout << "raw = " << raw << '\n';
    std::cout << "joined = " << joined << '\n';
    std::cout << "u8 text = " << u8"你好，世界" << '\n';
    std::cout << "utf-8 bytes = " << std::strlen(u8"你好，世界") << '\n';

    std::string text = "abc";
    std::cout << "literal + string = " << ("prefix:" + text) << '\n';
}

void show_conversions()
{
    std::cout << "\n== conversions ==\n";

    std::string number = std::to_string(42);
    std::string real = std::to_string(3.5);
    std::cout << "to_string(42) = " << number << '\n';
    std::cout << "to_string(3.5) = " << real << '\n';

    try {
        int parsed = std::stoi("123");
        double parsed_real = std::stod("2.5");
        std::cout << "stoi(\"123\") = " << parsed << '\n';
        std::cout << "stod(\"2.5\") = " << parsed_real << '\n';
    } catch (const std::exception &error) {
        std::cout << "conversion failed: " << error.what() << '\n';
    }
}

void show_split()
{
    std::string line = "alpha,beta,gamma";
    std::istringstream input(line);
    std::string field;
    std::vector<std::string> fields;

    while (std::getline(input, field, ',')) {
        fields.push_back(field);
    }

    std::cout << "\n== split ==\n";
    for (const std::string &item : fields) {
        std::cout << '[' << item << "] ";
    }
    std::cout << '\n';

    std::string sentence;
    std::istringstream words("one two three");
    while (words >> sentence) {
        std::cout << '<' << sentence << "> ";
    }
    std::cout << '\n';
}

} // namespace

int main()
{
    show_c_strings();
    show_std_string();
    show_string_view();
    show_literals();
    show_conversions();
    show_split();
    return 0;
}
