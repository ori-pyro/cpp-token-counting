#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include "cpr/api.h"
#include "cpr/cprtypes.h"
#include "cpr/response.h"
#include "framework.h"
#include "nlohmann/json_fwd.hpp"
#include "parser.h"

#ifndef GITHUB_TOKEN
#define GITHUB_TOKEN ""
#endif

std::string github_token = GITHUB_TOKEN;

using namespace std;
namespace fs = filesystem;
using json = nlohmann::json;

// Меж поточные переменные
atomic<Work_state> work_state_atomic{ WAITING_INPUT };
atomic<float> progress_bar_fraction_atomic{ 0.0f };
char shared_file_name_buffer[512] = { 0 };
atomic<bool> file_name_updated{ false };

unordered_map<string, int> tokens;
unordered_map<string, unordered_map<string, int>> detailedTokens;

bool has_error = false;
string error_massege;


bool error_check(cpr::Response& response) {
    // Ошибки сети
    if (response.error) {
        has_error = true;
        error_massege = "Ошибка сети: " + response.error.message;
        work_state_atomic = WAITING_INPUT;
        return false;
    }
    // Ошибки HTTP
    else if (response.status_code != 200) {
        has_error = true;
        error_massege = "Ошибка HTTP: " + std::to_string(response.status_code);
        work_state_atomic = WAITING_INPUT;
        return false;
    }
    // Всё ОК
    else {
        return true;
    }
}

void token_counting_online(std::string dir_url, uint8_t check_box_flag) {
    Parser parser;

    // TODO Сделать валидацию URL черещ re2

    // 1. Запрашиваем общую информацию о проекте, чтобы получить deafult_branch
    string repos_info_url = "https://api.github.com/repos/" + dir_url.substr(19);
    cpr::Response git_repos_info_response = cpr::Get(cpr::Url(repos_info_url),
                                            cpr::Header{{"User-Agent", "token_counter"},
                                                        {"Authorization", "Bearer " + github_token}});
    if (!error_check(git_repos_info_response)) { return; }
    json git_repos_info_json = json::parse(git_repos_info_response.text);
    string repos_default_branch = git_repos_info_json["default_branch"];

    // 2. Запрашиваем структуру проекта
    cpr::Url repos_git_tree_url(repos_info_url + "/git/trees/" + repos_default_branch);
    cpr::Response git_repos_git_tree_response = cpr::Get(repos_git_tree_url,
                                                cpr::Parameters{{"recursive", "1"}},
                                                cpr::Header{{"User-Agent", "token_counter"},
                                                            {"Authorization", "Bearer " + github_token},
                                                            {"Accept", "application/vnd.github+json"}});
    if (!error_check(git_repos_git_tree_response)) { return; }
    json git_repos_git_tree_json = json::parse(git_repos_git_tree_response.text);

    // Вектор для асинхронной загрузки файлов
    std::vector<std::pair<std::string, std::string>> files_to_download;
    const size_t BATCH_SIZE = 20;

    int cpp_cnt = 0;
    int h_cnt = 0;
    int hpp_cnt = 0;

    // 3. Собираем все URL для скачивания
    for (auto& tree: git_repos_git_tree_json["tree"]) {
        if (tree["type"] == "blob") {
            fs::path entry_path(tree["path"]);
            fs::path extension = fs::path(tree["path"]).extension();

            bool chosen_extention = check_box_flag & ((extension == ".cpp") + (extension == ".h")*2 + (extension == ".hpp")*4);
            if (extension == ".cpp") { cpp_cnt++; }
            else if (extension == ".h") { h_cnt++; }
            else if (extension == ".hpp") { hpp_cnt++; }
            if (chosen_extention) {
                cpr::Url blob_text_url = "https://raw.githubusercontent.com/" +
                                        dir_url.substr(19) + "/" +
                                        repos_default_branch + "/" +
                                        entry_path.generic_string(); // generic_string - всегда прямые слеши, чтобы не сломать HTTP запрос

                // Запускаем запрос асинхронно, не дожидаясь ответа
                files_to_download.emplace_back(
                    entry_path.filename().string(),
                    blob_text_url);
            }
        }
    }

    int curr_file_number = 0;

    int count_of_files = files_to_download.size();

    // 4. Качаем пачками
    for (size_t i = 0; i < files_to_download.size(); i += BATCH_SIZE) {
        std::vector<std::pair<std::string, cpr::AsyncResponse>> pending_requests;

        // Запускаем очередную пачку
        for (size_t j = i; j < std::min(i + BATCH_SIZE, files_to_download.size()); ++j) {
            pending_requests.emplace_back(
                files_to_download[j].first,
                cpr::GetAsync(cpr::Url{files_to_download[j].second})
            );
        }

        // Ждем выполнения текущей пачки
        for (auto& [file_name, future_response] : pending_requests) {
            cpr::Response response = future_response.get();

            curr_file_number++;
            progress_bar_fraction_atomic = static_cast<float>(curr_file_number) / count_of_files;
            std::string file_name_and_fraction = to_string(curr_file_number) + "/" + to_string(count_of_files) + " " + file_name;
            size_t copy_size = std::min(file_name_and_fraction.size(), sizeof(shared_file_name_buffer) - 1);
            memcpy(shared_file_name_buffer, file_name_and_fraction.data(), copy_size);
            shared_file_name_buffer[copy_size] = '\0';
            file_name_updated = true;

            if (error_check(response)) {
                parser.parser(response.text);
            } else {
                return; // Если упали по ошибке сети — выходим
            }
        }
    }
    parser.Summ();

    // копируем
    tokens = parser.tokens;
    detailedTokens = parser.detailedTokens;

    tokens[".cpp"] = cpp_cnt;
    tokens[".h"] = h_cnt;
    tokens[".hpp"] = hpp_cnt;


    parser.clear_table();

    work_state_atomic = SHOW_TABLE;
    has_error = false;
}

void token_counting_offline(std::string dir_path, uint8_t check_box_flag) {

    Parser parser;

    // Подсчёт общего количества файлов
    int cpp_cnt = 0;
    int h_cnt = 0;
    int hpp_cnt = 0;
    int count_of_files = 0;
    int curr_file_number = 0;

    for (const auto& entry : fs::recursive_directory_iterator(dir_path)) {
        fs::path extension = entry.path().extension();
        bool chosen_extention = check_box_flag & ((extension == ".cpp") + (extension == ".h")*2 + (extension == ".hpp")*4);
        if (extension == ".cpp") { cpp_cnt++; }
        else if (extension == ".h") { h_cnt++; }
        else if (extension == ".hpp") { hpp_cnt++; }
        if (chosen_extention) {
            count_of_files++;
        }
    }

    // Проход по всем файлам
    for (const auto& entry : fs::recursive_directory_iterator(dir_path)) {
        fs::path extension = entry.path().extension();
        bool chosen_extention = check_box_flag & ((extension == ".cpp") + (extension == ".h")*2 + (extension == ".hpp")*4);
        if (chosen_extention) {

            // Передаём в прогрессбар инфу
            curr_file_number++;
            progress_bar_fraction_atomic = static_cast<float>(curr_file_number)/count_of_files;
            std::string file_name = to_string(curr_file_number) + "/" + to_string(count_of_files) + " " + entry.path().filename().string();
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
    parser.Summ();

    // копируем
    tokens = parser.tokens;
    detailedTokens = parser.detailedTokens;

    tokens[".cpp"] = cpp_cnt;
    tokens[".h"] = h_cnt;
    tokens[".hpp"] = hpp_cnt;

    parser.clear_table();

    work_state_atomic = SHOW_TABLE;
    has_error = false;
}

int main() {

    Framework GUI;
    std::string input_str;

    while (!WindowShouldClose()) {

        // Рисуем
        if (GUI.work_state == SHOULD_CLOSE) {
            return 0;
        } else {
            GUI.update();
        }


        // Запускается после того как пользователь нажал ВВОД
        if (GUI.work_state == JUST_INPUT) {
            input_str = GUI.get_input();

            // URL
            if (input_str.substr(0, 8) == "https://") {
                std::thread analyzer(token_counting_online, input_str, GUI.check_box_flag);
                analyzer.detach();
                GUI.work_state = WORK_IN_PROGRESS;
            }
            // Локальная папка
            else if (fs::exists(input_str)) {
                fs::path local_path(input_str);
                std::thread analyzer(token_counting_offline, local_path.string(), GUI.check_box_flag);
                analyzer.detach();
                GUI.work_state = WORK_IN_PROGRESS;
            }
            else {
                GUI.incorrect_input("Некорректный URL или путь");
            }
        }

        if (GUI.work_state == WORK_IN_PROGRESS) {
            if (has_error) {
                GUI.work_state = WAITING_INPUT;
                GUI.incorrect_input(error_massege);
                continue;
            }

            // Обновляем progressbar
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
