#include <map>
#include <string>
#include <atomic>
#include <memory>
#include <filesystem>

#include <portable-file-dialogs.h>
#include "structures.h"

namespace fs = std::filesystem;

#ifndef GITHUB_TOKEN
#define GITHUB_TOKEN ""
#endif
std::string github_token = GITHUB_TOKEN;

enum class FileManagerEvent { Empty,
                              PathEnterd, URLEntered, IncorrectEntered,
                              SaveDialogOpened, BrowseDialogOpened,
                              ScanStarted, WorkStarted, Working };

struct FileData {
    int curr;
    std::shared_ptr<std::string> text;
    std::shared_ptr<std::string> filename;
};

struct FileIterator {
    fs::recursive_directory_iterator dirIterator;
    fs::recursive_directory_iterator endIterator;
    ChosenExtensions chosen;
    FileData data;

    // Конструктор
    explicit FileIterator(const fs::path& root, ChosenExtensions ext) : dirIterator(root), endIterator(), chosen(ext) {
        while (dirIterator != endIterator && !chosen.check(dirIterator->path().extension().string())) {
            ++dirIterator;
        }
        fillCurrentData(); // заполняем data для ПЕРВОГО элемента сразу в конструкторе
    }
    void fillCurrentData();

    FileIterator& operator++();
    FileIterator begin();
    FileIterator end();
    bool operator!=(const FileIterator& other) const;
    bool operator==(const FileIterator& other) const;
    FileData& operator*();
};


class FileManager {
    std::unique_ptr<pfd::save_file> save_dialog = nullptr;
    std::unique_ptr<pfd::select_folder> select_dialog = nullptr;

    fs::path save_path;

    std::atomic<FileManagerEvent> event;
    std::atomic<std::shared_ptr<std::string>> errorString;
    std::shared_ptr<FileIterator> fileIterator;
    std::string path_or_url = "";
    ChosenExtensions chosen;
public:
    int total = 0;

    void startPathScan();
    void startURLScan();
    void recognizeInput(const std::string&);

    void setExtensions(ChosenExtensions);

    std::string getPath();
    std::string getSavePath();
    FileManagerEvent getEvent();
    std::map<std::string, int> getExtensionCount();
    std::shared_ptr<std::string> getErrorMassege();
    std::shared_ptr<FileIterator> getFileIterator();

    void eventProcessed();

    void openBrowseDialog();
    bool updateBrowseDialog();
    void openSaveDialog();
    bool updateSaveDialog();

    void save(const std::string& path,
              const std::map<std::string, int>& type,
              const std::map<std::string, std::map<std::string, int>>& sub_type);
};
