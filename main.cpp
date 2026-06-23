#include <iostream>
#include <string>
#include <re2/re2.h>

int main() {
    // Простейший тест RE2: ищем регулярным выражением ключевые слова модулей
    std::string test_lexeme = "import-keyword";
    
    if (re2::RE2::PartialMatch(test_lexeme, R"(import-keyword|module-keyword|export-keyword)")) {
        std::cout << "Успех! RE2 нашел лексему: " << test_lexeme << std::endl;
    } else {
        std::cout << "Лексема не найдена." << std::endl;
    }

    return 0;
}
