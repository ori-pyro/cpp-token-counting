#pragma once

#include <map>
#include <string>
#include <atomic>
#include <memory>
#include <filesystem>

#include "cpr/cpr.h"
#include "cpr/api.h"
#include "cpr/cprtypes.h"
#include "cpr/response.h"
#include <portable-file-dialogs.h>

#include "structures.h"

namespace fs = std::filesystem;

#ifndef GITHUB_TOKEN
#define GITHUB_TOKEN ""
#endif
inline std::string github_token = GITHUB_TOKEN;

enum class FileManagerEvent { Empty,
                              PathEntered, URLEntered, IncorrectEntered,
                              NetworkError,
                              SaveDialogOpened, BrowseDialogOpened,
                              ScanStarted, PrepareFinished, Working };

enum class InputType { Empty, Path, Url };




// ========================= FileData =========================

struct FileData {
    int curr = 0;
    std::shared_ptr<std::string> text ;
    std::shared_ptr<std::string> filename;
};




// ========================= FileIterator =========================

struct FileIterator {
    ChosenExtensions chosen;
    FileData data;
    fs::recursive_directory_iterator dirIterator;
    fs::recursive_directory_iterator endIterator;

    // Конструктор
    explicit FileIterator(const fs::path&, ChosenExtensions);
    void fillCurrentData();

    FileIterator& operator++();
    FileIterator begin();
    FileIterator end();
    bool operator!=(const FileIterator& other) const;
    bool operator==(const FileIterator& other) const;
    FileData& operator*();
};




// ========================= URLIterator =========================

struct URLIterator {
    ChosenExtensions chosen;
    FileData data;

    size_t curr_in_butch = 0;
    const size_t BATCH_SIZE = 20;

    std::vector<std::pair<std::string, cpr::AsyncResponse>> pending_requests;
    std::vector<std::pair<std::string, cpr::AsyncResponse>>::iterator pending_iter;
    std::vector<std::pair<std::string, std::string>> files_to_download; // Получаем при инициализации

    explicit URLIterator(const std::vector<std::pair<std::string, std::string>>& files, ChosenExtensions ext);

    void fillCurrentData();

    struct URLIteratorEnd {};

    URLIterator& operator++();
    URLIterator& begin();
    URLIteratorEnd end();

    bool operator!=(const URLIterator& other) const;
    bool operator==(const URLIterator& other) const;

    bool operator!=(const URLIteratorEnd& other);
    bool operator==(const URLIteratorEnd& other);

    FileData& operator*();

    void fill_butch();
};




// ========================= FileManager =========================

class FileManager {
    std::string path_or_url = "";

    std::atomic<FileManagerEvent> event;
    std::atomic<std::shared_ptr<std::string>> errorString;

    ChosenExtensions chosen;

    std::atomic<InputType> inputType;
    std::shared_ptr<URLIterator> urlIterator;
    std::shared_ptr<FileIterator> fileIterator;

    // Для startURLScan()
    std::string owner;
    std::string repos;
    bool hasNetworkError = false;
    bool checkResponse(cpr::Response& response);
    std::vector<std::pair<std::string, std::string>> files_to_download;

    fs::path save_path;
    std::unique_ptr<pfd::save_file> save_dialog = nullptr;
    std::unique_ptr<pfd::select_folder> select_dialog = nullptr;
public:
    int total = 0;

    void startURLScan();
    void startPathScan();
    void eventProcessed();
    void resetInputType();
    void recognizeInput(const std::string&);

    void setExtensions(ChosenExtensions);

    std::string getPath();
    std::string getSavePath();
    FileManagerEvent getEvent();
    std::map<std::string, int> getExtensionCount();
    std::shared_ptr<std::string> getErrorMessage();

    InputType getInputType();
    std::shared_ptr<URLIterator> getURLIterator();
    std::shared_ptr<FileIterator> getFileIterator();

    void openBrowseDialog();
    bool updateBrowseDialog();
    void openSaveDialog();
    bool updateSaveDialog();
    void save(const std::string& path,
              const std::map<std::string, int>& type,
              const std::map<std::string, std::map<std::string, int>>& sub_type);
};
