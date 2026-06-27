#include <string>
#include <re2/re2.h>
#include <map>
#include <unordered_set>
#include "parser.h"
#include <iostream>


void Parser::parser(std::string& text) {
    rest = re2::StringPiece(text);
   	const char* start = text.data();
   	const char* prev = start;
   	re2::StringPiece match;
    bool need_to_check_prev_token = true;
   	while (RE2::FindAndConsume(&rest, separators, &match) && !rest.empty()) {
        std::string separator(match.data(), match.size());

        // std::cout << separator << std::endl;

        // operator-or-punctuator
        if (!(separator == " " || separator == "\n" || separator == "\t"
            || separator == ")" || separator == "]" || separator == "}"
            || separator == ":]" || separator == "%>" || separator == ":>")
        ) {
            if (separator == "//") {
                skip_comment();
                need_to_check_prev_token = false;
               	prev = rest.data();
            } else if (separator == "/*") {
                skip_multy_comment();
                need_to_check_prev_token = false;
               	prev = rest.data();
            } else {
                tokens["operator-or-punctuator"]++;
            }
        }

        start = match.data();

        // не lliterals
        if (need_to_check_prev_token) {
            std::string token(prev, start - prev);
            std::cout << token << std::endl;
            if (keywords.contains(token)) {
                tokens["keyword"]++;
                if (token == "true" || token == "false") {
                    tokens["boolean-literal"]++;
                } else if (token == "nullptr") {
                    tokens["pointer-literal"]++;
                }
            } else if (operatorsKeywords.contains(token)) {
                tokens["keyword"]++;
                tokens["operator-or-punctuator"]++;
            }
            else {
                tokens["identifier"]++;
            }
        }

        // на следующей итерации будет обработан токен, полученный на этой, если это не литерал
        need_to_check_prev_token = true;

        // literals
        if (rest.starts_with('.') || (rest.at(0) >= '0' && rest.at(0) <= '9') ) { // integer-literal и floating-point-literal
            digit_analyze();
            need_to_check_prev_token = false;
        } else if (rest.starts_with('\'')) { // character-literal
            character_analyze();
            need_to_check_prev_token = false;
        } else if (rest.starts_with('\"')) { // string-literal
            string_analyze();
            need_to_check_prev_token = false;
        }
        // boolean-literal обрабатывается в keywords;

       	prev = match.data() + match.size();

        // TODO user-defined-literal
        // TODO Убрать комменты из исходного текста
        // TODO Сырые строки и прочие строки
   	}
}


// Запускаем если начинается с числа или точки
void Parser::digit_analyze() {
    if (RE2::Consume(&rest, float_exp)) {
        if (!is_user_defined_literal()){
            tokens["floating-point-literal"]++;
        }
    } else {
        if (!is_user_defined_literal()) {
            tokens["integer-literal"]++;
        }
    }
}

void Parser::string_analyze() {
    // re2::StringPiece match;
    // re2::StringPiece match_kakashka;

    RE2::Consume(&rest, string_consume);
    // std::cout << std::string_view(match.data(), match.size()) << std::string_view(match_kakashka.data(), match_kakashka.size()) << "ti" << std::endl;
    if (!is_user_defined_literal()) {
        tokens["string-literal"]++;
    }
}

void Parser::character_analyze() {
    RE2::Consume(&rest, character_consume);
    if (!is_user_defined_literal()) {
        tokens["character-literal"]++;
    }
}

void Parser::skip_comment() { // TODO ЗДЕСЬ КАКАЯТО ЛАЖА
    RE2::FindAndConsume(&rest, comment);
}

void Parser::skip_multy_comment() {
    RE2::FindAndConsume(&rest, multy_comment);
}

bool Parser::is_user_defined_literal() {
    // TODO
    return false;
}
