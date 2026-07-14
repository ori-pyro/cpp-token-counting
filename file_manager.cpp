#include <filesystem>
#include <fstream>
#include <string>
#include <atomic>
#include <memory>
#include <map>
#include "structures.h"
#include "file_manager.h"

namespace fs = std::filesystem;

// ========= FileIterator =========
void FileIterator::fillCurrentData() {
    if (dirIterator != endIterator) {
        ++data.curr;
        data.filename = std::make_shared<std::string>(dirIterator->path().filename().string());
        std::fstream file(dirIterator->path(), std::ios::in);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            data.text = std::make_shared<std::string>(buffer.str());
        }
    }
}

FileIterator& FileIterator::operator++() {
    ++dirIterator;
    while (dirIterator != endIterator && !dirIterator->is_regular_file() && !chosen.check(dirIterator->path().extension().string())) {
        ++dirIterator;
    }
    fillCurrentData();
    return *this;
}
FileIterator FileIterator::begin() {
    operator++(); // Находим первый файл, который не папка
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


// ========= FileManager =========
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
    event.store(FileManagerEvent::WorkStarted);
}
void FileManager::startURLScan() {} // TODO
void FileManager::recognizeInput(const std::string& input_str) {
    if (fs::is_directory(input_str)) {
        event.store(FileManagerEvent::PathEnterd);
        path_or_url = input_str;
    }
    // else if (проверка на ссылку) { event.store(FileManagerEvent::URLEntered); }
    else {
        errorString.store(std::make_shared<std::string>("Некорректный путь или URL"));
        event.store(FileManagerEvent::IncorrectEntered);
    }
}
void FileManager::eventProcessed() {
    event = FileManagerEvent::Empty;
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
std::shared_ptr<std::string> FileManager::getErrorMassege() {
    return errorString;
}
std::shared_ptr<FileIterator> FileManager::getFileIterator() {
    fileIterator = std::make_shared<FileIterator>(fs::path(path_or_url), chosen);
    return fileIterator;
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
