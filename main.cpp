#include <iostream>
#include <fstream>      // Чтение файлов
#include <filesystem>
#include <vector>
#include <thread>
#include <atomic>
#include <unordered_map>
#include "framework.h"
#include "parser.h"

using namespace std;
namespace fs = filesystem;

atomic<Work_state> work_state_atomic{ WAITING_INPUT };

atomic<float> progress_bar_fraction_atomic{ 0.0f };

char shared_file_name_buffer[512] = {0};
atomic<bool> file_name_updated{ false };

unordered_map<string, int> tokens;
unordered_map<string, unordered_map<string, int>> detailedTokens;

void token_counting(std::string dir_path) {

    Parser parser;

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
            curr_file_number++;

            // передаём atomic заполненность прогрессбара в gui
            progress_bar_fraction_atomic = static_cast<float>(curr_file_number)/count_of_files;

            // передаём atomic имя файла в gui
            std::string file_name = entry.path().filename().string();
            size_t copy_size = std::min(file_name.size(), sizeof(shared_file_name_buffer) - 1);
            memcpy(shared_file_name_buffer, file_name.data(), copy_size);
            shared_file_name_buffer[copy_size] = '\0'; // Строка обязана заканчиваться нулем
            file_name_updated = true;

            fstream file(entry.path(), ios::in);        // Создаём объект файла и открываем для чтения
            if (file.is_open()) {
                stringstream buffer;
                buffer << file.rdbuf();
                string code = buffer.str(); // Весь код в одну строку
                parser.parser(code);
            }
        }
    }

    // копируем
    tokens = parser.tokens;
    detailedTokens = parser.detailedTokens;

    parser.clear_table();

    work_state_atomic = SHOW_TABLE;
}

int main() {

    Framework GUI;
    fs::path dir_path;

    while (!WindowShouldClose()) {

        // Рисуем
        if (GUI.work_state == SHOULD_CLOSE) {
            return 0;
        } else {
            GUI.update();
        }


        // Запускается после того как пользователь нажал ВВОД
        if (GUI.work_state == JUST_INPUT) {
            dir_path = GUI.get_input();

            // Проверка корректности ввода
            if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
                GUI.work_state = WAITING_INPUT;
                GUI.incorrect_input();
                continue;
            } else {
                std::thread analyzer(token_counting, dir_path.string());
                analyzer.detach();
                GUI.work_state = WORK_IN_PROGRESS;
            }
        }


        if (GUI.work_state == WORK_IN_PROGRESS) {
            GUI.progress_bar_fraction = progress_bar_fraction_atomic.load();
            GUI.progress_bar_text = std::string(shared_file_name_buffer);
            file_name_updated = false;

            // Если паралельный алгоритм завершил работу
            if (work_state_atomic == SHOW_TABLE) {
                GUI.work_state = SHOW_TABLE;
                GUI.set_table(tokens, detailedTokens);

                work_state_atomic = WAITING_INPUT; // Если пользователь нажмёт ОТМЕНА
            }
        }
    }

    return 0;
}
