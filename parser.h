#include <string>
#include <vector>
#include <re2/re2.h>
#include <map>
#include <unordered_set>

class Parser {
public:
    Parser() = default;
    ~Parser() = default;
    std::unordered_map<std::string, std::string> keywords = {
        {"bool", "type-keyword"},
        {"char", "type-keyword"},
        {"char8_t", "type-keyword"},
        {"char16_t", "type-keyword"},
        {"char32_t", "type-keyword"},
        {"double", "type-keyword"},
        {"float", "type-keyword"},
        {"int", "type-keyword"},
        {"long", "type-keyword"},
        {"short", "type-keyword"},
        {"signed", "type-keyword"},
        {"unsigned", "type-keyword"},
        {"void", "type-keyword"},
        {"wchar_t", "type-keyword"},
        {"false", "type-keyword"},
        {"true", "type-keyword"},
        {"typedef", "type-keyword"},

        {"break", "flow-control-keyword"},
        {"case", "flow-control-keyword"},
        {"continue", "flow-control-keyword"},
        {"default", "flow-control-keyword"},
        {"do", "flow-control-keyword"},
        {"else", "flow-control-keyword"},
        {"for", "flow-control-keyword"},
        {"goto", "flow-control-keyword"},
        {"if", "flow-control-keyword"},
        {"return", "flow-control-keyword"},
        {"switch", "flow-control-keyword"},
        {"while", "flow-control-keyword"},

        {"new", "memory-management-keyword"},
        {"delete", "memory-management-keyword"},

        {"class", "OOP-Keyword"},
        {"struct", "OOP-Keyword"},
        {"union", "OOP-Keyword"},
        {"enum", "OOP-Keyword"},
        {"public", "OOP-Keyword"},
        {"private", "OOP-Keyword"},
        {"protected", "OOP-Keyword"},
        {"virtual", "OOP-Keyword"},
        {"friend", "OOP-Keyword"},
        {"this", "OOP-Keyword"},
        {"mutable", "OOP-Keyword"},
        {"explicit", "OOP-Keyword"},
        {"operator", "OOP-Keyword"},

        {"catch", "exception-keyword"},
        {"throw", "exception-keyword"},
        {"try", "exception-keyword"},

        {"template", "templates-keyword"},
        {"typename", "templates-keyword"},
        {"export", "templates-keyword"},

        {"namespace", "namespaces-keyword"},
        {"using", "namespaces-keyword"},

        {"const_cast", "explicit-type-cast-keyword"},
        {"dynamic_cast", "explicit-type-cast-keyword"},
        {"reinterpret_cast", "explicit-type-cast-keyword"},
        {"static_cast", "explicit-type-cast-keyword"},

        {"const", "qualifiers-and-storage-keyword"},
        {"volatile", "qualifiers-and-storage-keyword"},
        {"static", "qualifiers-and-storage-keyword"},
        {"extern", "qualifiers-and-storage-keyword"},
        {"auto", "qualifiers-and-storage-keyword"},
        {"register", "qualifiers-and-storage-keyword"},
        {"thread_local", "qualifiers-and-storage-keyword"},

        {"alignas", "meta-keyword"},
        {"alignof", "meta-keyword"},
        {"decltype", "meta-keyword"},
        {"typeid", "meta-keyword"},
        {"sizeof", "meta-keyword"},
        {"static_assert", "meta-keyword"},
        {"noexcept", "meta-keyword"},
        {"concept", "meta-keyword"},
        {"requires", "meta-keyword"},
        {"constexpr", "meta-keyword"},
        {"consteval", "meta-keyword"},
        {"constinit", "meta-keyword"},

        {"nullptr", "modern-keyword"},
        {"co_await", "modern-keyword"},
        {"co_return", "modern-keyword"},
        {"co_yield", "modern-keyword"},

        {"or", "alternative-tokens-keyword"},
        {"and", "alternative-tokens-keyword"},
        {"xor", "alternative-tokens-keyword"},
        {"not", "alternative-tokens-keyword"},
        {"bitand", "alternative-tokens-keyword"},
        {"bitor", "alternative-tokens-keyword"},
        {"compl", "alternative-tokens-keyword"},
        {"and_eq", "alternative-tokens-keyword"},
        {"or_eq", "alternative-tokens-keyword"},
        {"xor_eq", "alternative-tokens-keyword"},
        {"not_eq", "alternative-tokens-keyword"},

        {"asm", "other-keyword"},
        {"atomic_cancel", "other-keyword"},
        {"atomic_commit", "other-keyword"},
        {"atomic_noexcept", "other-keyword"},
        {"contract_asset", "other-keyword"},
        {"reflexpr", "other-keyword"},
        {"synchronized", "other-keyword"},
        {"inline", "other-keyword"}
    };
    std::unordered_map<std::string, int> tokens {
        {"identifier", 0},
        {"keyword", 0},
        {"literal", 0},
        {"comments", 0},
        {"lines", 0},
        {"operator-or-punctuator", 0}
    };
    std::unordered_map<std::string, std::unordered_map<std::string, int>> detailedTokens{
        {"keyword",{
            {"type-keyword", 0},
            {"flow-control-keyword", 0},
            {"memory-management-keyword", 0},
            {"OOP-Keyword", 0},
            {"exсeption-keyword", 0},
            {"templates-keyword", 0},
            {"namespaces-keyword", 0},
            {"explicit-type-cast-keyword", 0},
            {"qualifiers-and-storage-keyword", 0},
            {"meta-keyword", 0},
            {"modern-keyword", 0},
            {"alternative-tokens-keyword", 0},
            {"other-keyword", 0}
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
        {"##", 0},
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
