#include <string>
#include <atomic>
#include <memory>
#include <map>
#include "structures.h"

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
    int total;
    FileIterator& operator++();
    FileIterator begin();
    FileIterator end();
    bool operator!=(const FileIterator& other) const;
    bool operator==(const FileIterator& other) const;
    FileData& operator*();
};


class FileManager {
    std::atomic<FileManagerEvent> event;
    std::atomic<std::shared_ptr<std::string>> errorString;
    std::shared_ptr<FileIterator> fileIterator;
    ChosenExtensions chosen;
public:
    void startPathScan();
    void startURLScan();
    void recognizeInput(const std::string&);

    void setEvent(FileManagerEvent);
    void setExtensions(ChosenExtensions);

    FileManagerEvent getEvent();
    std::shared_ptr<std::string> getErrorMassege();
    std::shared_ptr<FileIterator> getFileIterator();
    std::map<std::string, int> getExtensionCount();
    std::string getPath();

    void eventProcessed();

    void openBrowseDialog();
    bool updateBrowseDialog();
    void openSaveDialog();
    bool updateSaveDialog();

    void save(const std::map<std::string, int>& type,
              const std::map<std::string, std::map<std::string, int>>& sub_type);
};
