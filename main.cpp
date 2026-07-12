#include <string>
#include <thread>
#include <atomic>
#include <memory>
#include <map>
#include "framework.h"
#include "parser.h"
#include "file_manager.h"

// enum class Screen { InputScreen, LoadingScreen, TableScreen };

// enum class FrameworkEvent { Empty, ContinuePressed, BackPressed, SavePressed, BrowsePressed };
// enum class FileManagerEvent { Empty,
//                               PathEnterd, URLEntered, IncorrectEntered,
//                               SaveDialogOpened, BrowseDialogOpened,
//                               ScanStarted, WorkStarted, Working };

struct Table {
    std::atomic<std::shared_ptr<std::map<std::string, int>>> rows;
    std::atomic<std::shared_ptr<std::map<std::string, std::map<std::string, int>>>> sub_rows;
};

void work(Parser& parser, std::shared_ptr<FileIterator> fileIterator, Framework& gui, std::atomic<bool>& is_work_finished) {
    for (auto& iter : *fileIterator) {
        parser.parse(*iter.text);

        // отдаём инфу для вывода в прогресс бар в atomic переменные
        gui.curr.store(iter.curr);
        gui.filename.store(std::move(iter.filename));
    }
    is_work_finished.store(true);
}

int main() {
    Parser parser;
    Framework gui;
    FileManager fileManager;

    Table table;

    std::atomic<std::shared_ptr<FileIterator>> fileIterator;

    // Запускаем поток с графикой
    std::thread guiThread(&Framework::loop, &gui);
    guiThread.detach();

    std::atomic<bool> is_work_finished = false;

    while (gui.running) {
        switch (gui.getScreen()) {
            // ЭКРАН ВВОДА
            case Screen::InputScreen: {
                switch(fileManager.getEvent()) {
                    // ОТКРЫТО ОКНО ВЫБОРА ПАПКИ
                    case FileManagerEvent::BrowseDialogOpened: {
                        if (fileManager.updateBrowseDialog()) {
                            gui.setInput(fileManager.getPath());
                            fileManager.eventProcessed(); // сбраысваем любые ивенты от gui
                        }
                        break;
                    }
                    // ОКНО ВЫБОРА ПАПКИ НЕ ОТКРЫТО => обрабатываем ввод
                    default: {
                        switch (gui.getEvent()) {
                            // НАЖАТА ПРОДОЛЖИТЬ
                            case FrameworkEvent::ContinuePressed: {
                                fileManager.setExtensions(gui.getExtensions());
                                fileManager.recognizeInput(gui.getInput());
                                switch (fileManager.getEvent()) {
                                    // РАСПОЗНАН ПУТЬ К ПАПКЕ
                                    case FileManagerEvent::PathEnterd: {
                                        std::thread reposScanThread(&FileManager::startPathScan, &fileManager);
                                        reposScanThread.detach();

                                        gui.setScreen(Screen::LoadingScreen);
                                        gui.setProgressBarText(std::make_shared<std::string>("Подготовка..."));
                                        fileManager.eventProcessed();
                                        break;
                                    }
                                    // РАСПОЗНАНА ССЫЛКА НА ГИТХАБ
                                    case FileManagerEvent::URLEntered: {
                                        std::thread reposScanThread(&FileManager::startURLScan, &fileManager);
                                        reposScanThread.detach();

                                        gui.setScreen(Screen::LoadingScreen);
                                        gui.setProgressBarText(std::make_shared<std::string>("Подготовка..."));
                                        fileManager.eventProcessed();
                                        break;
                                    }
                                    // НЕКОРРЕКТНЫЙ ВВОД
                                    case FileManagerEvent::IncorrectEntered: {
                                        gui.setErrorMassege(fileManager.getErrorMassege());
                                        fileManager.eventProcessed();
                                        break;
                                    }
                                    default: { break; }
                                }

                                gui.eventProcessed();
                                break;
                            }
                            // НАЖАТА ОБЗОР
                            case FrameworkEvent::BrowsePressed: {
                                fileManager.openBrowseDialog();
                                // ... Запуск даилога выбора папки

                                gui.eventProcessed();
                                break;
                            }
                        }
                    }
                }


                break;
            }
            // ЭКРАН ЗАГРУЗКИ
            case Screen::LoadingScreen: {
                // ПАРСИНГ ЗАВЕРШЁН
                if (is_work_finished) {
                    // Загружаем результаты в таблицу

                    // Берём таблицу из parser
                    std::shared_ptr<std::map<std::string, int>> tokens = parser.getTokens();
                    std::shared_ptr<std::map<std::string, std::map<std::string, int>>> detailedTokens = parser.getDetailedTokens();
                    // Добавляем инфу из fileManager
                    (*tokens)["file"] = fileManager.total;
                    (*detailedTokens)["file"] = fileManager.getExtensionCount();
                    // Загружаем в таблицу
                    table.rows.store(tokens);
                    table.sub_rows.store(detailedTokens);
                    // Передаём в gui
                    gui.setTable(*table.rows.load(), *table.sub_rows.load());

                    gui.setScreen(Screen::TableScreen);
                    fileManager.eventProcessed();
                    break;
                }

                switch (fileManager.getEvent()) {
                    // ЗАПУСКАЕМ РАБОТУ
                    case FileManagerEvent::WorkStarted: {
                        fileIterator.store(fileManager.getFileIterator());
                        gui.total.store(fileManager.total);

                        std::thread parserThread(work, std::ref(parser), fileIterator.load(), std::ref(gui), std::ref(is_work_finished));
                        parserThread.detach();

                        fileManager.eventProcessed();
                        break;
                    }
                    // ВЫВОДИМ СОСТОЯНИЕ РАБОТЫ
                    case FileManagerEvent::Working: {
                        // ... Сидим не рыпаемся

                        break;
                    }
                }
                break;
            }
            // ЭКРАН ТАБЛИЦЫ
            case Screen::TableScreen: {
                switch(fileManager.getEvent()) {
                    // ОКНО СОХРАНЕНИЯ ОТКРЫТО
                    case FileManagerEvent::SaveDialogOpened: {
                        if (fileManager.updateSaveDialog()) {
                            fileManager.save(fileManager.getSavePath(), *table.rows.load(), *table.sub_rows.load());
                            fileManager.eventProcessed();
                        }
                        break;
                    }
                    // ОКНО СОХРАНЕНИЯ НЕ ОТКРЫТО => обрабатываем ввод
                    default: {
                        switch(gui.getEvent()) {
                            case FrameworkEvent::SavePressed: {
                                fileManager.openSaveDialog();

                                gui.eventProcessed();
                                break;
                            }
                            case FrameworkEvent::BackPressed: {
                                gui.setScreen(Screen::InputScreen);
                                is_work_finished.store(false);
                                // TODO Чистим таблицу и прогресс бар и вообще всё чистим на всякий случай
                                gui.eventProcessed();
                                break;
                            }
                        }
                    }
                }

                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // просто ждём, чтобы не перегрузить процессор
    }
}
