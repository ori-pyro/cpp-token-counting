#include <string>
#include <re2/re2.h>
#include <unordered_set>

class Parser {
public:
    Parser() = default;
    ~Parser() = default;
    std::unordered_map<std::string, int> tokens {
        {".cpp", 0},
        {".h", 0},
        {".hpp", 0},
        {"identifier", 0},
        {"keyword", 0},
        {"literal", 0},
        {"comment", 0},
        {"line", 0},
        {"operator-or-punctuator", 0}
    };
    std::unordered_set<std::string> operators {
            {"or", 0},
            {"and", 0},
            {"xor", 0},
            {"not", 0},
            {"bitand", 0},
            {"bitor", 0},
            {"compl", 0},
            {"and_eq", 0},
            {"or_eq", 0},
            {"xor_eq", 0},
            {"not_eq", 0}
    };
    std::unordered_map<std::string, std::unordered_map<std::string, int>> detailedTokens{
        {"keyword",{

            {"bool", 0},
            {"char", 0},
            {"char8_t", 0},
            {"char16_t", 0},
            {"char32_t", 0},
            {"double", 0},
            {"float", 0},
            {"int", 0},
            {"long", 0},
            {"short", 0},
            {"signed", 0},
            {"unsigned", 0},
            {"void", 0},
            {"wchar_t", 0},
            {"false", 0},
            {"true", 0},
            {"typedef", 0},

            {"break", 0},
            {"case", 0},
            {"continue", 0},
            {"default", 0},
            {"do", 0},
            {"else", 0},
            {"for", 0},
            {"goto", 0},
            {"if", 0},
            {"return", 0},
            {"switch", 0},
            {"while", 0},

            {"new", 0},
            {"delete", 0},

            {"class", 0},
            {"struct", 0},
            {"union", 0},
            {"enum", 0},
            {"public", 0},
            {"private", 0},
            {"protected", 0},
            {"virtual", 0},
            {"friend", 0},
            {"this", 0},
            {"mutable", 0},
            {"explicit", 0},
            {"operator", 0},

            {"catch", 0},
            {"throw", 0},
            {"try", 0},

            {"template", 0},
            {"typename", 0},
            {"export", 0},

            {"namespace", 0},
            {"using", 0},

            {"const_cast", 0},
            {"dynamic_cast", 0},
            {"reinterpret_cast", 0},
            {"static_cast", 0},

            {"const", 0},
            {"volatile", 0},
            {"static", 0},
            {"extern", 0},
            {"auto", 0},
            {"register", 0},
            {"thread_local", 0},

            {"alignas", 0},
            {"alignof", 0},
            {"decltype", 0},
            {"typeid", 0},
            {"sizeof", 0},
            {"static_assert", 0},
            {"noexcept", 0},
            {"concept", 0},
            {"requires", 0},
            {"constexpr", 0},
            {"consteval", 0},
            {"constinit", 0},

            {"nullptr", 0},
            {"co_await", 0},
            {"co_return", 0},
            {"co_yield", 0},

            {"or", 0},
            {"and", 0},
            {"xor", 0},
            {"not", 0},
            {"bitand", 0},
            {"bitor", 0},
            {"compl", 0},
            {"and_eq", 0},
            {"or_eq", 0},
            {"xor_eq", 0},
            {"not_eq", 0},

            {"asm", 0},
            {"atomic_cancel", 0},
            {"atomic_commit", 0},
            {"atomic_noexcept", 0},
            {"contract_asset", 0},
            {"reflexpr", 0},
            {"synchronized", 0},
            {"inline", 0}
        }},
        {"literal", {
            {"integer-literal", 0},
            {"character-literal", 0},
            {"floating-point-literal", 0},
            {"string-literal", 0},
            {"boolean-literal", 0},
            {"pointer-literal", 0},
            {"user-defined-literal", 0},
        }},
        {"operator-or-punctuator", {
        {"+", 0},
        {"-", 0},
        {"*", 0},
        {"/", 0},
        {"%", 0},

        {"++", 0},
        {"--", 0},
        {"==", 0},
        {"!=", 0},
        {"<", 0},
        {">", 0},
        {"<=", 0},
        {">=", 0},
        {"<=>", 0},

        {"!", 0},
        {"&&", 0},
        {"||", 0},

        {"~", 0},
        {"&", 0},
        {"|", 0},
        {"^", 0},
        {"<<", 0},
        {">>", 0},

        {"=", 0},
        {"+=", 0},
        {"-=", 0},
        {"*=", 0},
        {"/=", 0},
        {"%=", 0},
        {"&=", 0},
        {"|=", 0},
        {"^=", 0},
        {"<<=", 0},
        {">>=", 0},

        {".", 0},
        {"->", 0},
        {".*", 0},
        {"->*", 0},

        {"::", 0},

        {"?:", 0},

        {",", 0},

        {"(", 0},
        {"{", 0},
        {"[", 0},
        {";", 0},
        {":", 0},
        {"#", 0},
        {"...", 0},

        {"and", 0},
        {"and_eq", 0},
        {"bitand", 0},
        {"bitor", 0},
        {"compl", 0},
        {"not", 0},
        {"not_eq", 0},
        {"or", 0},
        {"or_eq", 0},
        {"xor", 0},
        {"xor_eq", 0},

        {"<%", 0},
        {"<:", 0},
        {"%:", 0},
        {"%:%:", 0}
        }}
    }; //TODO приплюсовать в tokens

    RE2 separators = RE2(R"((>>=|<<=|>>|<<|&&|\|\||\+\+|//|/\*|--|<=>|>=|<=|==|\+=|-=|\*=|/=|%=|^=|&=|\|=|!=|<%|%>|<:|:>|\.\.\.|::|\.\*|->|->\*|\^\^|[][(){};:?~!*/%\^&|=<>,.+\-#]|\s+))");
    RE2 float_exp = RE2(R"((\d*\.\d*[eE]?[-+]?\d*[fF]?))");
    RE2 int_skip = RE2(R"([0-9'A-Fa-fxXbB]+)");
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

    void clear_table();
    void Summ();
};
