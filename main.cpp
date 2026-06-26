#include <iostream>
#include <fstream>      // Чтение файлов
#include <re2/re2.h>    // Регулярки
#include <filesystem>
#include <vector>
#include "framework.h"

using namespace std;
namespace fs = filesystem;


int main() {

    Framework GUI;

    vector<string> token_names = {
        "identifier",
        "keyword",
        "   integer-literal",
        "   character-literal",
        "   floating-point-literal",
        "   string-literal",
        "   boolean-literal",
        "   pointer-literal",
        "   user-defined-literal",
        "header-name",
        "operator-or-punctuator"
    };
    vector<int> token_count = {
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0
    };

    while (!WindowShouldClose()) {

        // Рисуем
        if (GUI.work_state == SHOULD_CLOSE) {
            return 0;
        } else {
            GUI.draw_GUI();
        }


        // Запускается после того как пользователь нажал ВВОД
        if (GUI.work_state == JUST_INPUT) {
            fs::path dir_path = GUI.get_input();

            // Проверка корректности ввода
            if (!fs::exists(dir_path)) {
                GUI.work_state = WAITING_INPUT;
                cout << "dir isn't exists" << '\n';
                continue;
            } else {
                GUI.work_state = WORK_IN_PROGRESS;
            }

            // Подсчёт общего количества файлов для прогрессбара
            int count_of_files = 0;
            int curr_file_number = 0;
            for (const auto& entry : fs::recursive_directory_iterator(dir_path)) {
                if (entry.path().extension() == ".cpp") {
                    count_of_files++;
                }
            }

            // Проход по всем файлам
            for (const auto& entry : fs::recursive_directory_iterator(dir_path)) {
                if (entry.path().extension() == ".cpp") {

                    GUI.progress_bar_text = entry.path().filename().string(); // Название текущего обрабатываемого файла
                    GUI.progress_bar_fraction = static_cast<float>(curr_file_number)/count_of_files; // Состояние прогрессбара
                    GUI.draw_GUI(); // Рисуем перед обработкой файла

                    curr_file_number++;
                    fstream file(entry.path(), ios::in);        // Создаём объект файла и открываем для чтения

                    stringstream buffer;
                    buffer << file.rdbuf();                     // TODO Сделать убирание пробелов через оператор << и цикл while
                    string code = buffer.str();                 // Весь код в одну строку
                }
            }

            GUI.set_table(token_names, token_count);
            GUI.work_state = SHOW_TABLE;
        }
    }

    return 0;
}
