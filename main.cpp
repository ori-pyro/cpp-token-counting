#include <string>
#include <thread>
#include <atomic>
#include <memory>
#include <map>

#include "framework.h"
#include "parser.h"

#define Rectangle WinRectangle
#define CloseWindow WinCloseWindow
#define ShowCursor WinShowCursor

#include "file_manager.h"

#undef Rectangle
#undef CloseWindow
#undef ShowCursor

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

template <typename iteratorT>
void work(Parser& parser, std::shared_ptr<iteratorT> iterator, Framework& gui, std::atomic<bool>& is_work_finished) {
    auto end_marker = iterator->end();

    while (*iterator != end_marker) {
        auto& iter = **iterator;

        parser.parse(*iter.text);

        // отдаём инфу для вывода в прогресс бар в atomic переменные
        gui.curr.store(iter.curr);
        gui.filename.store(std::move(iter.filename));
        gui.setProgressBarText(nullptr);

        ++(*iterator);
    }
    parser.Summ();
    is_work_finished.store(true);
}

void dispatcher(Framework& gui, FileManager& fileManager, Parser& parser, Table& table, std::atomic<bool>& is_work_finished) {
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
                                    case FileManagerEvent::PathEntered: {
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
                                        gui.setErrorMassege(fileManager.getErrorMessage());
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
                    case FileManagerEvent::PrepareFinished: {
                        switch (fileManager.getInputType()) {
                            case InputType::Path: {
                                std::thread parserThread(work<FileIterator>, std::ref(parser), fileManager.getFileIterator(), std::ref(gui), std::ref(is_work_finished));
                                parserThread.detach();
                                fileManager.resetInputType();
                                break;
                            }

                            case InputType::Url: {
                                std::thread parserThread(work<URLIterator>, std::ref(parser), fileManager.getURLIterator(), std::ref(gui), std::ref(is_work_finished));
                                parserThread.detach();
                                fileManager.resetInputType();
                                break;
                            }
                        }

                        gui.total.store(fileManager.total);

                        fileManager.eventProcessed();
                        break;
                    }
                    // ВЫВОДИМ СОСТОЯНИЕ РАБОТЫ
                    case FileManagerEvent::NetworkError: {
                        gui.setErrorMassege(fileManager.getErrorMessage());
                        gui.setScreen(Screen::InputScreen);
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
                                is_work_finished.store(false);

                                // TODO Чистим таблицу и прогресс бар и вообще всё чистим на всякий случай
                                parser.clear_table();
                                gui.curr.store(0);
                                gui.total.store(0);
                                gui.setProgressBarText(nullptr);
                                gui.setErrorMassege(nullptr);

                                gui.setScreen(Screen::InputScreen);
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

int main() {
    Framework gui;

    Parser parser;
    FileManager fileManager;
    Table table;

    std::atomic<bool> is_work_finished = false;

    // Запускаем логику-диспетчер в отдельном потоке
    std::thread dispatcherThread(dispatcher, std::ref(gui), std::ref(fileManager), std::ref(parser), std::ref(table), std::ref(is_work_finished));
    dispatcherThread.detach();

    gui.loop();

    return 0;
}
