/*
 * ============================================================================
 *  C++ 语言圣经 · 第 11 章
 *  综合练习：一个小型表达式求值器
 * ============================================================================
 *
 *  语法：
 *
 *      expression := term (('+' | '-') term)*
 *      term       := factor (('*' | '/') factor)*
 *      factor     := number | '(' expression ')' | '-' factor | '+' factor
 *      number     := digit+ ('.' digit*)? | '.' digit+
 *
 *  支持：
 *      + - * /、括号、一元正负号、十进制小数、空白
 *
 *  不支持：
 *      变量、函数、指数记法、隐式乘法
 *
 *  编译：
 *      c++ -std=c++17 -Wall -Wextra -Wpedantic calc.cpp -o calc
 *
 *  运行：
 *      ./calc
 *      ./calc "1 + 2 * 3"
 * ============================================================================
 */

#include <cstddef>
#include <iomanip>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

enum class TokenKind {
    Number,
    Plus,
    Minus,
    Star,
    Slash,
    LeftParen,
    RightParen,
    End,
    Invalid
};

struct Token {
    TokenKind kind = TokenKind::End;
    double number = 0.0;
    std::size_t offset = 0;
};

bool is_digit(char character)
{
    return character >= '0' && character <= '9';
}

bool is_space(char character)
{
    return character == ' ' || character == '\t' ||
           character == '\n' || character == '\r';
}

class Lexer {
public:
    explicit Lexer(std::string_view input) : input_(input), position_(0)
    {
    }

    Token next()
    {
        skip_space();

        if (position_ >= input_.size()) {
            return {TokenKind::End, 0.0, position_};
        }

        char character = input_[position_];
        std::size_t offset = position_;
        ++position_;

        switch (character) {
        case '+':
            return {TokenKind::Plus, 0.0, offset};
        case '-':
            return {TokenKind::Minus, 0.0, offset};
        case '*':
            return {TokenKind::Star, 0.0, offset};
        case '/':
            return {TokenKind::Slash, 0.0, offset};
        case '(':
            return {TokenKind::LeftParen, 0.0, offset};
        case ')':
            return {TokenKind::RightParen, 0.0, offset};
        default:
            break;
        }

        if (is_digit(character)) {
            --position_;
            return number();
        }

        if (character == '.' && position_ < input_.size() &&
            is_digit(input_[position_])) {
            --position_;
            return number();
        }

        return {TokenKind::Invalid, 0.0, offset};
    }

private:
    void skip_space()
    {
        while (position_ < input_.size() && is_space(input_[position_])) {
            ++position_;
        }
    }

    Token number()
    {
        std::size_t start = position_;
        double value = 0.0;

        while (position_ < input_.size() && is_digit(input_[position_])) {
            value = value * 10.0 + static_cast<double>(input_[position_] - '0');
            ++position_;
        }

        if (position_ < input_.size() && input_[position_] == '.') {
            ++position_;
            double scale = 0.1;
            while (position_ < input_.size() && is_digit(input_[position_])) {
                value += static_cast<double>(input_[position_] - '0') * scale;
                scale *= 0.1;
                ++position_;
            }
        }

        return {TokenKind::Number, value, start};
    }

    std::string_view input_;
    std::size_t position_;
};

class Parser {
public:
    explicit Parser(std::string_view input) : lexer_(input), current_(lexer_.next())
    {
    }

    std::optional<double> parse()
    {
        double value = expression();
        if (!error_.empty()) {
            return std::nullopt;
        }

        if (current_.kind != TokenKind::End) {
            fail("unexpected token", current_.offset);
            return std::nullopt;
        }

        return value;
    }

    const std::string &error() const
    {
        return error_;
    }

    std::size_t error_offset() const
    {
        return error_offset_;
    }

private:
    double expression()
    {
        double value = term();

        while (error_.empty() &&
               (current_.kind == TokenKind::Plus ||
                current_.kind == TokenKind::Minus)) {
            TokenKind operation = current_.kind;
            advance();

            double right = term();
            if (!error_.empty()) {
                return 0.0;
            }

            if (operation == TokenKind::Plus) {
                value += right;
            } else {
                value -= right;
            }
        }

        return value;
    }

    double term()
    {
        double value = factor();

        while (error_.empty() &&
               (current_.kind == TokenKind::Star ||
                current_.kind == TokenKind::Slash)) {
            TokenKind operation = current_.kind;
            std::size_t operation_offset = current_.offset;
            advance();

            double right = factor();
            if (!error_.empty()) {
                return 0.0;
            }

            if (operation == TokenKind::Star) {
                value *= right;
            } else {
                if (right == 0.0) {
                    fail("division by zero", operation_offset);
                    return 0.0;
                }
                value /= right;
            }
        }

        return value;
    }

    double factor()
    {
        if (current_.kind == TokenKind::Plus) {
            advance();
            return factor();
        }

        if (current_.kind == TokenKind::Minus) {
            advance();
            return -factor();
        }

        return primary();
    }

    double primary()
    {
        if (current_.kind == TokenKind::Number) {
            double value = current_.number;
            advance();
            return value;
        }

        if (current_.kind == TokenKind::LeftParen) {
            advance();
            double value = expression();
            if (!error_.empty()) {
                return 0.0;
            }
            if (current_.kind != TokenKind::RightParen) {
                fail("expected ')'", current_.offset);
                return 0.0;
            }
            advance();
            return value;
        }

        fail("expected number or '('", current_.offset);
        return 0.0;
    }

    void advance()
    {
        current_ = lexer_.next();
    }

    void fail(std::string message, std::size_t offset)
    {
        if (error_.empty()) {
            error_ = std::move(message);
            error_offset_ = offset;
        }
    }

    Lexer lexer_;
    Token current_;
    std::string error_;
    std::size_t error_offset_ = 0;
};

struct Evaluation {
    std::optional<double> value;
    std::string error;
    std::size_t error_offset = 0;
};

Evaluation evaluate(std::string_view text)
{
    Parser parser(text);
    Evaluation result;
    result.value = parser.parse();

    if (!result.value.has_value()) {
        result.error = parser.error();
        result.error_offset = parser.error_offset();
    }

    return result;
}

void report(std::string_view text)
{
    Evaluation result = evaluate(text);

    std::cout << text << " = ";
    if (result.value.has_value()) {
        std::cout << std::setprecision(12) << *result.value << '\n';
        return;
    }

    std::cout << "error: " << result.error
              << " at offset " << result.error_offset << '\n';
    std::cout << "  " << text << '\n';
    std::cout << "  " << std::string(result.error_offset, ' ') << "^\n";
}

} // namespace

int main(int argc, char *argv[])
{
    const std::vector<std::string> samples = {
        "1 + 2 * 3",
        "(1 + 2) * 3",
        "10 / 4",
        "-3 + 5",
        "2 * (3 + 4) - 5",
        "0.1 + 0.2",
        "1 / 0",
        "2 +",
    };

    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            report(argv[i]);
        }
        return 0;
    }

    std::cout << "== expression evaluator ==\n";
    for (const std::string &sample : samples) {
        report(sample);
    }
    std::cout << "\nusage: calc \"1 + 2 * 3\"\n";

    return 0;
}
