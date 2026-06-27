#include <string>
#include <re2/re2.h>
#include <map>
#include <unordered_set>
#include "parser.h"


void Parser::parser(std::string& text) {
    re2::StringPiece rest(text);
   	const char* start = text.data();
   	const char* prev = start;
   	re2::StringPiece match;
    bool need_to_check_prev_token = true;
   	while (RE2::Consume(&rest, separators, &match)) {
        std::string separator(match.data(),match.size());

        // operator-or-punctuator
        if (!(separator == " " || separator == "\n" || separator == "\t"
            || separator == ")" || separator == "]" || separator == "}"
            || separator == ":]" || separator == "%>" || separator == ":>")
        ) {
            if (separator == "//") {
                skip_comment();
                need_to_check_prev_token = false;
            } else if (separator == "/*") {
                skip_multy_comment();
                need_to_check_prev_token = false;
            } else {
                tokens["operator-or-punctuator"]++;
            }
        }

        start = match.data();

        // не lliterals
        if (need_to_check_prev_token) {
            std::string token(start, start - prev);
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

       	prev = match.data()+match.size();

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
    RE2::Consume(&rest, string_consume);
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

void Parser::skip_comment() {
    RE2::FindAndConsume(&rest, comment);
}

void Parser::skip_multy_comment() {
    RE2::FindAndConsume(&rest, multy_comment);
}

bool Parser::is_user_defined_literal() {
    //TODO
    return false;
}
