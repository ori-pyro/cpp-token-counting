#include <filesystem>
#include <fstream>
#include <string>
#include <atomic>
#include <memory>
#include <map>

#include "re2/re2.h"
#include "cpr/cpr.h"
#include <nlohmann/json.hpp>
#include "nlohmann/json_fwd.hpp"

#include "structures.h"
#include "file_manager.h"

namespace fs = std::filesystem;
using json = nlohmann::json;


bool isValidGithubURL(const std::string& URL, std::string& owner, std::string& repos) {
    static const RE2 pattern(R"(^https://github\.com/([A-Za-z0-9_.-]+)/([A-Za-z0-9_.-]+)(?:\.git)?)");
    return RE2::FullMatch(URL, pattern, &owner, &repos);
}



// ========================= FileManager =========================

void FileManager::startPathScan() {
    total = 0;
    for (const auto& entry : fs::recursive_directory_iterator(path_or_url)) {
        if (entry.is_regular_file()) {
            std::string extension = entry.path().extension().string();

            if (chosen.check_and_count(extension)) {
                total++;
            }
        }
    }
    fileIterator = make_shared<FileIterator>(path_or_url, chosen);
    inputType.store(InputType::Path);
    event.store(FileManagerEvent::PrepareFinished);
}
void FileManager::startURLScan() {
    total = 0;

    // 1. Запрашиваем общую информацию о проекте, чтобы получить default_branch
    std::string git_info_url = "https://api.github.com/repos/" + owner + "/" + repos;
    cpr::Response git_info_response =
        cpr::Get(cpr::Url(git_info_url),
                 cpr::Header{{"User-Agent", "token_counter"},
                             {"Authorization", "Bearer " + github_token}}
        );
    if (!checkResponse(git_info_response)) { return; }
    json git_repos_info_json = json::parse(git_info_response.text);
    std::string repos_default_branch = git_repos_info_json["default_branch"];

    // 2. Запрашиваем структуру проекта
    cpr::Url repos_git_tree_url(git_info_url + "/git/trees/" + repos_default_branch);
    cpr::Response git_tree_response = cpr::Get(repos_git_tree_url,
                                                         cpr::Parameters{{"recursive", "1"}},
                                                         cpr::Header{{"User-Agent", "token_counter"},
                                                                     {"Authorization", "Bearer " + github_token},
                                                                     {"Accept", "application/vnd.github+json"}}
                                                );
    if (!checkResponse(git_tree_response)) { return; }
    json git_tree_json = json::parse(git_tree_response.text);

    files_to_download.clear();

    for (auto& tree: git_tree_json["tree"]) {
        if (tree["type"] == "blob") {
            fs::path entry_path(tree["path"]);
            std::string extension = fs::path(tree["path"]).extension().string();

            if (chosen.check_and_count(extension)) {
                total++;
                cpr::Url blob_text_url = "https://raw.githubusercontent.com/" + owner + "/" + repos + "/" +
                                        repos_default_branch + "/" +
                                        entry_path.generic_string(); // generic_string - всегда прямые слэши, чтобы не сломать HTTP запрос
                // Запускаем запрос асинхронно, не дожидаясь ответа
                files_to_download.emplace_back(entry_path.filename().string(),
                                               blob_text_url
                );
            }
        }
    }
    urlIterator = make_shared<URLIterator>(files_to_download, chosen);
    inputType.store(InputType::Url);
    event.store(FileManagerEvent::PrepareFinished);
}
bool FileManager::checkResponse(cpr::Response& response) {
    // Ошибки сети
    if (response.error) {
        hasNetworkError = true;
        errorString.store(std::make_shared<std::string>("Ошибка сети: " + response.error.message));
        event.store(FileManagerEvent::NetworkError);
        return false;
    }
    // Ошибки HTTP
    else if (response.status_code != 200) {
        hasNetworkError = true;
        errorString.store(std::make_shared<std::string>("Ошибка HTTP: " + std::to_string(response.status_code)));
        event.store(FileManagerEvent::NetworkError);
        return false;
    }
    // Всё ОК
    else {
        return true;
    }
}

void FileManager::recognizeInput(const std::string& input_str) {
    if (fs::is_directory(input_str)) {
        event.store(FileManagerEvent::PathEntered);
        path_or_url = input_str;
    }
    else if (isValidGithubURL(input_str, owner, repos)) {
        event.store(FileManagerEvent::URLEntered);
        path_or_url = input_str;
    }
    else {
        errorString.store(std::make_shared<std::string>("Некорректный путь или URL"));
        event.store(FileManagerEvent::IncorrectEntered);
    }
}
void FileManager::eventProcessed() {
    event.store(FileManagerEvent::Empty);
}
void FileManager::resetInputType() {
    inputType.store(InputType::Empty);
}

void FileManager::setExtensions(ChosenExtensions new_chosen) {
    chosen = new_chosen;
}

std::string FileManager::getPath() {
    return path_or_url;
}
std::string FileManager::getSavePath() {
    return save_path.string();
}
InputType FileManager::getInputType() {
    return inputType;
}

FileManagerEvent FileManager::getEvent() {
    return event;
}
std::map<std::string, int> FileManager::getExtensionCount() {
    std::map<std::string, int> extension_cnt;
    if (chosen.cpp) { extension_cnt[".cpp"] = chosen.cpp_cnt; }
    if (chosen.cc) { extension_cnt[".cc"]  = chosen.cc_cnt; }
    if (chosen.cxx) { extension_cnt[".cxx"] = chosen.cxx_cnt; }
    if (chosen.h)   { extension_cnt[".h"]   = chosen.h_cnt; }
    if (chosen.hpp) { extension_cnt[".hpp"] = chosen.hpp_cnt; }
    if (chosen.hh)  { extension_cnt[".hh"]  = chosen.hh_cnt; }
    if (chosen.hxx) { extension_cnt[".hxx"] = chosen.hxx_cnt; }
    return extension_cnt;
}
std::shared_ptr<std::string> FileManager::getErrorMessage() {
    return errorString;
}
std::shared_ptr<FileIterator> FileManager::getFileIterator() {
    return fileIterator;
}
std::shared_ptr<URLIterator> FileManager::getURLIterator() {
    return urlIterator;
}

void FileManager::openBrowseDialog() {
    select_dialog = std::make_unique<pfd::select_folder>("Выберите директорию");
    event.store(FileManagerEvent::BrowseDialogOpened);
}
bool FileManager::updateBrowseDialog() {
    if (select_dialog->ready()) {
        path_or_url = select_dialog->result();
        select_dialog.reset();
        return true;
    }
    return false;
}
void FileManager::openSaveDialog() {
    save_dialog = std::make_unique<pfd::save_file>(
        "Выберите директорию",
        "token_count.txt",
        std::vector<std::string>{ "Text Files (*.txt)", "*.txt" },
        pfd::opt::force_overwrite
    );
    event.store(FileManagerEvent::SaveDialogOpened);
}
bool FileManager::updateSaveDialog() {
    if (save_dialog->ready()) {
        save_path = save_dialog->result();
        save_dialog.reset();
        return true;
    }
    return false;
}
void FileManager::save(const std::string& path,
                       const std::map<std::string, int>& rows,
                       const std::map<std::string, std::map<std::string, int>>& sub_rows) {
    std::ofstream file(path);
    file << "\nProject: " << path_or_url << "\n\n";
    for (auto& [type, cnt] : rows) {
        file << std::left << std::setw(30) << type
             << std::right << std::setw(10) << cnt << "\n";
        if (sub_rows.contains(type)) {
            for (auto& [sub_type, sub_cnt] : sub_rows.at(type)) {
                file << '\t' << std::left << std::setw(30) << sub_type
                     << std::right << std::setw(10) << sub_cnt << "\n";
            }
        }
    }
}




// ========================= FileIterator =========================

FileIterator::FileIterator(const fs::path& root, ChosenExtensions ext) : dirIterator(root), endIterator() {
    chosen = ext;
    while (dirIterator != endIterator) {
        if (dirIterator->is_regular_file() && chosen.check(dirIterator->path().extension().string())) {
            break;
        }
        ++dirIterator;
    }
    fillCurrentData(); // заполняем data для ПЕРВОГО элемента сразу в конструкторе
}
void FileIterator::fillCurrentData() {
    if (dirIterator != endIterator) {
        ++data.curr;
        data.filename = std::make_shared<std::string>(dirIterator->path().filename().string());
        std::fstream file(dirIterator->path(), std::ios::in);
        if (file.is_open()) { // ! Слабое место
            std::stringstream buffer;
            buffer << file.rdbuf();
            data.text = std::make_shared<std::string>(buffer.str());
        } else {
            std::cout << "not open" << std::endl;
        }
    }
}

FileIterator& FileIterator::operator++() {
    ++dirIterator;
    while (dirIterator != endIterator) {
        if (dirIterator->is_regular_file() && chosen.check(dirIterator->path().extension().string())) {
            break;
        }
        ++dirIterator;
    }
    fillCurrentData();
    return *this;
}
FileIterator FileIterator::begin() {
    return *this;
}
FileIterator FileIterator::end() {
    FileIterator end = *this;
    end.dirIterator = endIterator;
    return end;
}
bool FileIterator::operator!=(const FileIterator& other) const {
    return this->dirIterator != other.dirIterator;
}
bool FileIterator::operator==(const FileIterator& other) const {
    return this->dirIterator == other.dirIterator;
}
FileData& FileIterator::operator*() {
    return data;
}




// ========================= URLIterator =========================

URLIterator::URLIterator(const std::vector<std::pair<std::string, std::string>>& files, ChosenExtensions ext) : files_to_download(files) {
    chosen = ext;
    if (!files_to_download.empty()) {
        fill_butch(); // Вызываем снаружи потому что внутри operator++() для этой операции есть условие
        operator++();
    }
}

void URLIterator::fillCurrentData() {
    ++curr_in_butch;
    ++data.curr;
    auto& [file_name, future_response] = *pending_iter;
    cpr::Response response = future_response.get();

    // TODO Нормальная обработка ошибок
    if (response.status_code != 200) { std::cout << "HTTP error with downloading of file: " << response.status_code << std::endl; }


    data.filename = std::make_shared<std::string>(file_name);
    data.text = std::make_shared<std::string>(response.text);
}

URLIterator& URLIterator::operator++() {
    // Если дошли до конца, то так и оставляем, чтобы сработало begin == end в base-ranged for
    if (data.curr != files_to_download.size()) {
        // Делаем подгрузку файлов пачкой
        if (curr_in_butch == BATCH_SIZE) {
            fill_butch();
        }

        // Пишем данные из текущего файла и делаем ++
        fillCurrentData();
        ++pending_iter;
    } else {
        data.curr++;
    }

    return *this;
}
URLIterator& URLIterator::begin() {
    return *this;
}
URLIterator::URLIteratorEnd URLIterator::end() {
    return {};
}

bool URLIterator::operator!=(const URLIterator& other) const {
    return this->data.curr != other.data.curr + 1;
}
bool URLIterator::operator==(const URLIterator& other) const {
    return this->data.curr == other.data.curr;
}

bool URLIterator::operator!=(const URLIteratorEnd& other) {
    return this->data.curr != files_to_download.size() + 1;
}
bool URLIterator::operator==(const URLIteratorEnd& other) {
    return this->data.curr == files_to_download.size() + 1;
}

FileData& URLIterator::operator*() {
    return data;
}

void URLIterator::fill_butch() {
    pending_requests.clear();
    for (size_t j = data.curr; j < std::min(data.curr + BATCH_SIZE, files_to_download.size()); ++j) {
        pending_requests.emplace_back(files_to_download[j].first, cpr::GetAsync(cpr::Url{files_to_download[j].second}));
    }
    curr_in_butch = 0;
    pending_iter = pending_requests.begin();
}
