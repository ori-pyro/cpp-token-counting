#include <iostream>
#include <fstream>      // Чтение файлов
#include <re2/re2.h>    // Регулярки
#include <filesystem>
#include <windows.h>    // Windows API (для нормального вывода кирилицы)

using namespace std;
namespace fs = filesystem;


int main() {
    // Настраиваем консоль Windows на работу с UTF-8 для обработки кирилицы в пути
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);

    fs::path dir_path;
    cout << "Введите путь к директории: ";
    cin >> dir_path;
    cout << '\n';

    if (!fs::exists(dir_path)) {
        cout << "Такой дериктории не существует" << '\n';
        return 1;
    }

    for (const auto& entry : fs::recursive_directory_iterator(dir_path)) {
        if (entry.path().extension() == ".cpp") {
            fstream file(entry.path(), ios::in);        // Создаём объект файла и открываем для чтения

            if (!file) return 1;

            stringstream buffer;
            buffer << file.rdbuf();                     //TODO Сделать убирание пробелов через оператор << и цикл while
            string code = buffer.str();                 // Весь код в одну строку
        }
    }

    return 0;
}
