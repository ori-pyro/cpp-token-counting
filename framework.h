#pragma once

#include "imgui.h"      // Фреймворк
#include "raylib.h"     // Бэкенд для ImGui

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <atomic>

#include "structures.h"


struct Table_row_expandable {
    std::string type = "";
    int cnt = 0;
    bool is_expanded = false;
    std::vector<std::string> sub_types;
    std::vector<int> sub_count;
};

struct vectorTable {
    std::vector<Table_row_expandable> expandable_rows;
};


enum class Screen { InputScreen, LoadingScreen, TableScreen };
enum class FrameworkEvent { Empty, ContinuePressed, BackPressed, SavePressed, BrowsePressed };


class Framework {
private:
    // framework.h, в private:
    // static SDL_HitTestResult titleBarHitTest(SDL_Window* win, const SDL_Point* pt, void* data);

    float WIDTH                 = 600.0f;
    float HEIGHT                = 400.0f;
    float TITLE_BAR_HEIGHT      = 30.0f;
    float BUTTON_BAR            = 60.0f;
    float TABLE_HEIGHT          = HEIGHT - TITLE_BAR_HEIGHT - BUTTON_BAR;
    float OBSERVE_BUTTON_WIDTH  = 80.0f;
    const float content_top     = TITLE_BAR_HEIGHT;
    const float content_height  = HEIGHT - content_top - BUTTON_BAR;
    ImVec2 FRAME_PADDING_VEC    = { 10.0f, 10.0f };
    ImVec2 BUTTON_SIZE          = { 100.0f, 40.0f };
    ImVec2 TABLE_CELL_VEC       = { 24.0f, 24.0f };

    // ЦВЕТА
    const ImVec4 BASIC_COLOR = ImVec4(36.0/255.0f, 39/255.0f, 58/255.0f, 1.0f);
    const ImVec4 BASIC_COLOR_HIGHLIGHTED = ImVec4(47.0/255.0f, 50/255.0f, 71/255.0f, 1.0f);
    const ImVec4 BASIC_COLOR_ACTIVATED = ImVec4(68.0/255.0f, 72/255.0f, 94/255.0f, 1.0f);
    const ImVec4 TITLEBAR_COLOR = ImVec4(24.0/255.0f, 25/255.0f, 38/255.0f, 1.0f);
    const ImVec4 TITLEBAR_COLOR_HIGHLIGHTED = ImVec4(39.0/255.0f, 41/255.0f, 57/255.0f, 1.0f);
    const ImVec4 TITLEBAR_COLOR_ACTIVATED = ImVec4(64.0/255.0f, 68/255.0f, 87/255.0f, 1.0f);
    const ImVec4 CLOSE_BUTTON_HOVERED_COLOR = ImVec4(0.8f, 0.12f, 0.12f, 1.0f);
    const ImVec4 CLOSE_BUTTON_ACTIVE_COLOR = ImVec4(0.6f, 0.12f, 0.12f, 1.0f);
    const ImVec4 TEXT_COLOR = ImVec4(184.0/255.0f, 192/255.0f, 224/255.0f, 1.0f);
    const ImVec4 TRANSPERENT = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    const ImVec4 SEMI_TRANSPERENT = ImVec4(1.0f, 1.0f, 1.0f, 0.3f);
    const ImVec4 LIGTHER_TRANSPERENT = ImVec4(1.0f, 1.0f, 1.0f, 0.6f);

    std::map<std::string, int> types;
    std::map<std::string, std::map<std::string, int>> sub_types;

    vectorTable table;

    ImFont* regular_font = nullptr;
    ImFont* small_font = nullptr;

    // ввод — трогается и из guiThread (ImGui-виджет), и из main-потока (getInput/setInput)
    std::atomic<std::shared_ptr<std::string>> input_buffer{ std::make_shared<std::string>("") };

    ChosenExtensions chosen;

    std::atomic<Screen> screen{ Screen::InputScreen };
    std::atomic<FrameworkEvent> frameworkEvent{ FrameworkEvent::Empty };

    std::atomic<std::shared_ptr<std::string>> error_massege{ std::make_shared<std::string>("") };
    std::atomic<std::shared_ptr<std::string>> progress_bar_text{ std::make_shared<std::string>("") };

    bool input_is_correct = true;

    void update();

    void drawInputScreen();
    void drawLoadingScreen();
    void drawTableScreen();

    void drawTitlebar();

    void drawInputField();
    void drawProgressBar();
    void drawTable();

    void drawCheckBox();
    void saveButton();
    void backButton();
    void browseButton();
    void continueButton();

    void drawErrorMessage();

public:
    Framework();
    ~Framework();
    // главный цикл — запускается в отдельном потоке из main (std::thread(&Framework::loop, &gui))
    void loop();

    // используется в while(gui.running) в main
    std::atomic<bool> running{ false };

    // прогресс - worker-поток пишет напрямую (gui.curr.store(...), и т.д.)
    std::atomic<int> curr{ 0 };
    std::atomic<int> total{ 0 };
    std::atomic<std::shared_ptr<std::string>> filename;

    // навигация между экранами
    Screen getScreen();
    void setScreen(Screen);

    // события от gui (кнопки)
    FrameworkEvent getEvent();
    void eventProcessed();

    // поле ввода пути/ссылки
    std::string getInput();
    void setInput(std::string); // Принимает ввод из selectDialog.

    // выбранные расширения файлов
    ChosenExtensions getExtensions();

    // сообщение об ошибке / текст прогресс-бара — вызываются из main
    void setErrorMassege(std::shared_ptr<std::string>);
    void setProgressBarText(std::shared_ptr<std::string>);

    // таблица результатов
    void setTable(
        const std::map<std::string, int>&,
        const std::map<std::string, std::map<std::string, int>>&
    );
};
