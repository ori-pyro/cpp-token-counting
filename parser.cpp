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
   	while (RE2::FindAndConsume(&rest, separators, &match)) {
        std::string separator(match.data(), match.size());
        // std::cout << separator << '\n';

        start = match.data();

        // Перед разделителем (other)
        if (need_to_check_prev_token) {
            std::string token(prev, start - prev);
            // std::cout << "tok: " << token << std::endl;
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
            else if (!token.empty()) { // Если два сепаратора стоят в притык то получим пустой токен
                tokens["identifier"]++;
            }
        }

        // на следующей итерации будет обработан токен, полученный на этой, если это не литерал
        need_to_check_prev_token = true;

        // Разделитель (operator-or-punctuator)
        if (!(separator.starts_with(" ") || separator.starts_with("\n") || separator.starts_with("\t")
            || separator == ")" || separator == "]" || separator == "}"
            || separator == ":]" || separator == "%>" || separator == ":>"
            || separator.empty())
        ) {
            if (separator == "//" || separator == "#") {
                skip_to_end_of_line();
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

        if (rest.empty()) break;

        // std::cout << "sep: " << separator << '\n';


        // После разделителя (literals)
        if (rest.starts_with('.') || (rest.front() >= '0' && rest.front() <= '9') ) { // integer-literal и floating-point-literal
            digit_analyze();
            need_to_check_prev_token = false;
        } else {
            int string_prefix_len = (rest.starts_with("u") || rest.starts_with("U") || rest.starts_with("L")) + (rest.starts_with("u8")); // простите....
            // std::cout << rest << "\n\n\n";

            re2::StringPiece before_remove_prefix = rest;
            rest.remove_prefix(string_prefix_len); // обрезаем префикс строки (u, u8, ...)
            if (rest.starts_with('\'')) { // character-literal
                character_analyze();
                need_to_check_prev_token = false;
            } else if (rest.starts_with('\"')) { // string-literal
                string_analyze();
                need_to_check_prev_token = false;
            } else if (rest.starts_with("R\"")) { // raw string (string-literal)
                raw_string_analyze();
                need_to_check_prev_token = false;
            } else {
                rest = before_remove_prefix;
            }
        }

        // boolean-literal обрабатывается в keywords;

        prev = match.data() + match.size();

        // TODO user-defined-literal
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
    std::string match;
    // std::cout << "before: " << rest << '\n';
    RE2::Consume(&rest, string_consume, &match);
    // std::cout << "after: " << rest << '\n';

    if (!is_user_defined_literal()) {
        // std::cout << match << '\n';
        tokens["string-literal"]++;
    }
}

void Parser::raw_string_analyze() {
    std::string delimiter;
    RE2::Consume(&rest, raw_string_begin, &delimiter);
    delimiter = RE2::QuoteMeta(delimiter);

    std::string raw_string_end = "\\)" + delimiter + "\"";

    RE2::FindAndConsume(&rest, RE2(raw_string_end));

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

void Parser::skip_to_end_of_line() {
    RE2::FindAndConsume(&rest, end_of_line);
}

void Parser::skip_multy_comment() {
    RE2::FindAndConsume(&rest, multy_comment);
}

bool Parser::is_user_defined_literal() {
    // TODO
    return false;
}
