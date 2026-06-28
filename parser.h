#include <string>
#include <vector>
#include <re2/re2.h>
#include <map>
#include <unordered_set>

class Parser {
public:
    Parser() = default;
    ~Parser() = default;

    const std::unordered_set<std::string> keywords = {"alignas","alignof","asm","atomic_cancel","atomic_commit","atomic_noexcept","auto","bool","break","case",
    "catch","char","char8_t","char16_t","char32_t","class","concept","const","consteval","constexpr","constinit","const_cast","continue",
    "contract_asset","co_await","co_return","co_yield","decltype","default","delete","do","double","dynamic_cast",
    "else","enum","explicit","export","extern","false","float","for","friend","goto","if","inline","int","long","mutable",
    "namespace","new","noexcept","nullptr","operator","private","protected","public","reflexpr","register","reinterpret_cast",
    "requires","return","short","signed","sizeof","static","static_assert","static_cast","struct","switch","synchronized","template",
    "this","thread_local","throw","true","try","typedef","typeid","typename","union","unsigned","using","virtual","void","volatile","wchar_t","while"
    };
    const std::unordered_set<std::string> operatorsKeywords = {"or","and","xor","not","bitand","bitor","compl","and_eq","or_eq","xor_eq","not_eq"};
    std::map<std::string, int> tokens {
        {"identifier", 0},
        {"keyword", 0},
        {"integer-literal", 0},
        {"character-literal", 0},
        {"floating-point-literal", 0},
        {"string-literal", 0},
        {"boolean-literal", 0},
        {"pointer-literal", 0},
        {"user-defined-literal", 0},
        {"header-name", 0},
        {"operator-or-punctuator", 0}
    };
    RE2 separators = RE2(R"((>>=|<<=|>>|<<|&&|\|\||\+\+|//|/\*|--|<=>|>=|<=|==|\+=|-=|\*=|/=|%=|^=|&=|\|=|!=|<%|%>|<:|:>|\.\.\.|::|\.\*|->|->\*|\^\^|[][(){};:?~!*/%\^&|=<>,.+\-#]|\s+))");
    RE2 float_exp = RE2(R"((\d*\.\d*[eE]?[-+]?\d*[fF]?))");
    RE2 string_consume = RE2(R"("(([^"\\]|\\.)*"))");
    RE2 raw_string_begin = RE2(R"(R"([^(]*)\()");
    RE2 character_consume = RE2(R"('([^'\\]|\\.)*')");
    RE2 end_of_line = RE2(R"(\n)");
    RE2 multy_comment = RE2(R"(\*/)"); // многострочный

    re2::StringPiece rest;

    void parser(std::string&);

    // Запускаем если начинается с числа или точки
    void digit_analyze();

    void string_analyze();

    void raw_string_analyze();

    void character_analyze();

    void skip_to_end_of_line();

    void skip_multy_comment();

    bool is_user_defined_literal();
};
