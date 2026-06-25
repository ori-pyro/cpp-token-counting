#include <iostream>
#include <fstream>      // Чтение файлов
#include <re2/re2.h>    // Регулярки
#include <filesystem>
#include "framework.h"

using namespace std;
namespace fs = filesystem;


int main() {

    Framework GUI;

    while (!WindowShouldClose()) {

        // Рисуем
        if (GUI.work_state == SHOULD_CLOSE) {
            return 0;
        } else {
            GUI.draw_GUI();
        }



        if (GUI.work_state == JUST_INPUT) {
            fs::path dir_path = GUI.get_input();

            if (!fs::exists(dir_path)) {
                GUI.work_state = WAITING_INPUT;
                cout << "dir isn't exists" << '\n';
                continue;
            } else {
                GUI.work_state = WORK_IN_PROGRESS;
            }

            int count_of_files = 0;
            for (const auto& entry : fs::recursive_directory_iterator(dir_path)) {
                if (entry.path().extension() == ".cpp") {
                    count_of_files++;
                }
            }

            int curr_file_number = 0;
            for (const auto& entry : fs::recursive_directory_iterator(dir_path)) {
                if (entry.path().extension() == ".cpp") {

                    cout << static_cast<float>(curr_file_number)/count_of_files << '\n';
                    GUI.progress_bar_text = entry.path().filename().string();
                    GUI.progress_bar_fraction = static_cast<float>(curr_file_number)/count_of_files;
                    GUI.draw_GUI();

                    curr_file_number++;
                    // cout << entry.path().filename().string() << '\n';
                    fstream file(entry.path(), ios::in);        // Создаём объект файла и открываем для чтения

                    stringstream buffer;
                    buffer << file.rdbuf();                     // TODO Сделать убирание пробелов через оператор << и цикл while
                    string code = buffer.str();                 // Весь код в одну строку
                }
            }
            GUI.work_state = WAITING_INPUT;
        }
    }

    return 0;
}
