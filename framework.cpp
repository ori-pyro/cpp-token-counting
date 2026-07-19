#include "imgui.h"                  // Фреймворк
#include "raylib.h"                 // Бэкенд для ImGui
#include "rlImGui.h"                // Мостик между RayLib и ImGui
#include <misc/cpp/imgui_stdlib.h>  // Работа с std::string в ImGui

#include "JetBrainsMono.h"          // Подключаем шрифт
#include "framework.h"

using namespace std;


Framework::Framework() {
        SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_HIGHDPI);
        InitWindow(WIDTH, HEIGHT, "Token Counter");
        SetTargetFPS(60);

        rlImGuiSetup(true);

        ImGuiIO& io = ImGui::GetIO();

        // ШРИФТЫ
        regular_font = io.Fonts->AddFontFromMemoryCompressedTTF(
                myFontData_compressed_data,        // Имя массива из JetBrainsMonoFont.h
                myFontData_compressed_size,        // Размер массива из JetBrainsMonoFont.h
                18.0f,                             // Размер шрифта в пикселях
                nullptr,                           // Настройки (оставляем nullptr)
                io.Fonts->GetGlyphRangesCyrillic() // ВКЛЮЧАЕМ РУССКИЙ ЯЗЫК
            );
        small_font = io.Fonts->AddFontFromMemoryCompressedTTF(
                myFontData_compressed_data,        // Имя массива из JetBrainsMonoFont.h
                myFontData_compressed_size,        // Размер массива из JetBrainsMonoFont.h
                12.0f,                             // Размер шрифта в пикселях
                nullptr,                           // Настройки (оставляем nullptr)
                io.Fonts->GetGlyphRangesCyrillic() // ВКЛЮЧАЕМ РУССКИЙ ЯЗЫК
            );

        // move_by_titlebar()
        titleBarRect = { 0, 0, WIDTH, TITLE_BAR_HEIGHT };
        isDragging = false;
        dragOffset = { 0, 0 }; // Координаты мыши относительно левого верхнего угла окна

        // Делаем шрифтом по умлочанию
        if (regular_font != nullptr) {
            io.FontDefault = regular_font;
        }

        ImGui::GetIO().IniFilename = nullptr; // Отключаем создание imgui.ini

        // КАСТОМНЫЙ ТАЙТЛБАР

        ImGuiStyle& style = ImGui::GetStyle();

        // ГЛОБАЛЬНЫЕ НАСТРОЙКИ СТИЛЯ КНОПОК
        style.Colors[ImGuiCol_Button]        = BASIC_COLOR;             // Обычная
        style.Colors[ImGuiCol_ButtonHovered] = BASIC_COLOR_HIGHLIGHTED; // При наведении
        style.Colors[ImGuiCol_ButtonActive]  = BASIC_COLOR_ACTIVATED;   // При нажатии

        // СТИЛЬ РАМОК
        style.FramePadding = FRAME_PADDING_VEC;
        style.Colors[ImGuiCol_FrameBg]              = TRANSPERENT;
        style.Colors[ImGuiCol_Border]               = TITLEBAR_COLOR_ACTIVATED;
        style.Colors[ImGuiCol_TableBorderLight]     = TITLEBAR_COLOR_ACTIVATED;
        style.Colors[ImGuiCol_TableBorderStrong]    = TITLEBAR_COLOR_ACTIVATED;
}
void Framework::loop() {
    running.store(true);
    while (!WindowShouldClose() && running.load()) {
        move_by_drag_titlebar();

        BeginDrawing();
        ClearBackground(DARKGRAY);

        // TODO Нужно ли это убрать?
        // Если мышь за пределами окна,
        // то сообщаем ImGui об этом, иначе он
        // будет думать, что курсор осталься на краю окна
        if (!IsCursorOnScreen()) {
            ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
            ImGui::GetIO().MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
        } else {
            ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
        }

        // TODO что делать с этим?
        rlImGuiBegin();
        {
            Vector2 dpiScale = GetWindowScaleDPI();
            ImGuiIO& io = ImGui::GetIO();
            io.DisplayFramebufferScale = ImVec2(dpiScale.x, dpiScale.y);
        }

        // Убираем внутренние отступы окна в ноль
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                                ImGuiWindowFlags_NoResize |
                                ImGuiWindowFlags_NoMove |
                                ImGuiWindowFlags_NoCollapse |
                                ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("MainPanel", nullptr, flags);
            drawTitlebar();

            ImGui::PushStyleColor(ImGuiCol_ChildBg, BASIC_COLOR);

                // Контент в основном окне
                ImGui::SetCursorPosY(TITLE_BAR_HEIGHT);
                    switch (screen.load()) {
                        case Screen::InputScreen: {
                            drawInputScreen();
                            break;
                        }
                        case Screen::LoadingScreen: {
                            drawLoadingScreen();
                            break;
                        }
                        case Screen::TableScreen: {
                            drawTableScreen();
                            break;
                        }
                    }
                ImGui::PopStyleColor();

        ImGui::End();
        ImGui::PopStyleVar();


        rlImGuiEnd();
        EndDrawing();
    }
    running.store(false); // на случай если вышли именно по WindowShouldClose() (Alt+F4 или Mod+Q)
}

Framework::~Framework() {
    rlImGuiShutdown(); // Освиобождаем ресурсы raylib
    CloseWindow();     // Просим ОС закрыть окно
}

void Framework::drawInputScreen() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, FRAME_PADDING_VEC);
    ImGui::BeginChild("Content", ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding);
        // Поле ввода, кнопка обзор
        drawInputField(); ImGui::SameLine(); browseButton();

        // Чекбоксы
        drawCheckBox();

        // Сообщение об ошибке
        if (!input_is_correct) { drawErrorMessage(); }
        continueButton();
    ImGui::PopStyleVar();
    ImGui::EndChild();
}
void Framework::drawLoadingScreen() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, FRAME_PADDING_VEC);
    ImGui::BeginChild("Content", ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding);
        drawProgressBar();
    ImGui::PopStyleVar();
    ImGui::EndChild();
}
void Framework::drawTableScreen() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, FRAME_PADDING_VEC);
    ImGui::BeginChild("Content", ImVec2(0, TABLE_HEIGHT), ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding);
        drawTable();
    ImGui::PopStyleVar();
    ImGui::EndChild();


    ImGui::SetCursorPosY(HEIGHT-BUTTON_BAR);

    ImGui::BeginChild("Buttons", ImVec2(0, BUTTON_BAR), ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding);
        saveButton(); backButton();
    ImGui::EndChild();
}


void Framework::drawTitlebar() {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, TITLEBAR_COLOR);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);

    ImGui::BeginChild("FakeTitleBarVisual", ImVec2(0, TITLE_BAR_HEIGHT), ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);

        // Заголовок тайтлбара
        ImGui::PushFont(regular_font, 16.0f);
            ImGui::Text("Token Counter");
        ImGui::PopFont();

        ImGui::SameLine(ImGui::GetWindowWidth() - TITLE_BAR_HEIGHT);
        ImGui::SetCursorPosY(0);

        // Стиль кнопки закрытия
        ImGui::PushStyleColor(ImGuiCol_Button, TITLEBAR_COLOR);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, CLOSE_BUTTON_ACTIVE_COLOR);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, CLOSE_BUTTON_HOVERED_COLOR);
            if (ImGui::Button("✕", ImVec2(TITLE_BAR_HEIGHT, TITLE_BAR_HEIGHT))) {

                running.store(false);
            }
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();


        ImGui::SameLine(ImGui::GetWindowWidth() - TITLE_BAR_HEIGHT * 2);
        ImGui::SetCursorPosY(0);

        // Стиль кнопки сворачивания
        ImGui::PushStyleColor(ImGuiCol_Button, TITLEBAR_COLOR);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, TITLEBAR_COLOR_HIGHLIGHTED);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, TITLEBAR_COLOR_ACTIVATED);
            if (ImGui::Button("—", ImVec2(TITLE_BAR_HEIGHT, TITLE_BAR_HEIGHT))) {
                MinimizeWindow();
            }
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();

    ImGui::EndChild();

    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void Framework::drawInputField() {
    // Поле ввода
    ImGui::GetStyle().FrameBorderSize = 1.8f;
    ImGui::PushStyleColor(ImGuiCol_FrameBg, BASIC_COLOR);
    ImGui::PushStyleColor(ImGuiCol_Border, BASIC_COLOR_HIGHLIGHTED);
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, TEXT_COLOR);
        ImGui::SetNextItemWidth(-(OBSERVE_BUTTON_WIDTH + FRAME_PADDING_VEC.x));
            if (ImGui::InputTextWithHint("##dir_path_unput", "Введите URL или путь к локальной директории", &*input_buffer.load(), ImGuiInputTextFlags_EnterReturnsTrue)) {
                frameworkEvent.store(FrameworkEvent::ContinuePressed);
            }
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::GetStyle().FrameBorderSize = 0.0f;
}
void Framework::drawProgressBar() {
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, TITLEBAR_COLOR_ACTIVATED);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, TITLEBAR_COLOR);
    ImGui::PushStyleColor(ImGuiCol_Text, TEXT_COLOR);
        ImVec2 cursor_pos_before_progress_bar = ImGui::GetCursorPos();

        ImGui::ProgressBar(static_cast<float>(curr.load())/total.load(), ImVec2(-1, 25), "");

        cursor_pos_before_progress_bar.y += 3;
        ImGui::SetCursorPos(cursor_pos_before_progress_bar);

        if (!(curr.load() && total.load())) {
            ImGui::Text(" %s", progress_bar_text.load()->c_str());
        } else {
            std::string str = " " + std::to_string(curr.load()) + "/" + std::to_string(total.load()) +
                            " " + *filename.load();

            ImGui::Text("%s", str.c_str());
        }
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();

    ImGui::PopStyleColor();
}
void Framework::drawTable() {
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 3.0f));
    if (ImGui::BeginTable("my_table", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        ImGui::TableSetupColumn("Button", ImGuiTableColumnFlags_WidthFixed, TABLE_CELL_VEC.x); // Колонка для кнопки раскрытия
        ImGui::TableSetupColumn("Token");
        ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_WidthFixed, 130.0f);
        for (int i = 0; i < table.expandable_rows.size(); i++) {
            Table_row_expandable& row = table.expandable_rows[i];
            ImGui::TableNextRow(0, TABLE_CELL_VEC.y); // Высота строки

            // Рисуем кнопку в первой колонке
            ImGui::TableNextColumn();
            ImGui::PushID(i);

            if (!table.expandable_rows[i].sub_types.empty()) {
                ImGui::PushStyleColor(ImGuiCol_Button, TRANSPERENT);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, SEMI_TRANSPERENT);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, LIGTHER_TRANSPERENT);
                    if (ImGui::Button(row.is_expanded ? "-" : "+", TABLE_CELL_VEC)) {
                        row.is_expanded = !row.is_expanded;
                    }
                ImGui::PopStyleColor();
                ImGui::PopStyleColor();
                ImGui::PopStyleColor();
            }


            std::string count_str = std::to_string(row.cnt);
            std::string type_cpy = std::string(row.type);

            ImGui::TableNextColumn();
            ImGui::InputText("##token", &type_cpy, ImGuiInputTextFlags_ReadOnly);

            ImGui::TableNextColumn();
            ImGui::InputText("##count", &count_str, ImGuiInputTextFlags_ReadOnly);

            if (row.is_expanded) {
                for (int j = 0; j < row.sub_types.size(); j++) {
                    ImGui::PushID(j);
                    ImGui::TableNextRow(0, TABLE_CELL_VEC.y); // Высота строки

                    ImGui::TableNextColumn(); // скип колонки с кнопкой
                    std::string count_str = std::to_string(row.sub_count[j]);
                    std::string sub_type_cpy = row.sub_types[j];

                    ImGui::TableNextColumn();
                    ImGui::SameLine(0.0f, 30.0f); // сдвиг
                    ImGui::InputText("##sub_token", &sub_type_cpy, ImGuiInputTextFlags_ReadOnly);

                    ImGui::TableNextColumn();
                    ImGui::SameLine(0.0f, 30.0f); // сдвиг
                    ImGui::InputText("##sub_count", &count_str, ImGuiInputTextFlags_ReadOnly);

                    ImGui::PopID();
                }
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();

}

void Framework::drawCheckBox() {
    ImGui::Dummy(ImVec2(0.0f, 15.0f));

    ImGui::GetStyle().FrameBorderSize = 1.5f;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::Checkbox(".cpp", &chosen.cpp);
        ImGui::Checkbox(".cc", &chosen.cc);
        ImGui::Checkbox(".cxx", &chosen.cxx);
        ImGui::Checkbox(".h", &chosen.h);
        ImGui::Checkbox(".hpp", &chosen.hpp);
        ImGui::Checkbox(".hh", &chosen.hh);
        ImGui::Checkbox(".hxx", &chosen.hxx);
    ImGui::PopStyleVar();
    ImGui::GetStyle().FrameBorderSize = 0.0f;
}
void Framework::saveButton() {
    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - (BUTTON_SIZE.x + FRAME_PADDING_VEC.x), ImGui::GetWindowHeight()-(BUTTON_SIZE.y + FRAME_PADDING_VEC.y)));

    if (ImGui::Button("Сохранить", BUTTON_SIZE)) {
        frameworkEvent.store(FrameworkEvent::SavePressed);
    }
}
void Framework::backButton() {
    ImGui::SetCursorPos(ImVec2(FRAME_PADDING_VEC.x, ImGui::GetWindowHeight()-(BUTTON_SIZE.y + FRAME_PADDING_VEC.y)));

    if (ImGui::Button("Назад", BUTTON_SIZE)) {
        frameworkEvent.store(FrameworkEvent::BackPressed);
    }
}
void Framework::browseButton() {
    if (ImGui::Button("Обзор", ImVec2(OBSERVE_BUTTON_WIDTH, 0))) {
        frameworkEvent.store(FrameworkEvent::BrowsePressed);
    }
}
void Framework::continueButton() {
    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth()-(BUTTON_SIZE.x+FRAME_PADDING_VEC.x), ImGui::GetWindowHeight()-(BUTTON_SIZE.y+FRAME_PADDING_VEC.y)));
    if (ImGui::Button("Далее", BUTTON_SIZE)) {
        frameworkEvent.store(FrameworkEvent::ContinuePressed);
    }
}

void Framework::setScreen(Screen new_screen) {
    screen.store(new_screen);
}
Screen Framework::getScreen() {
    return screen.load();
}

FrameworkEvent Framework::getEvent() {
    return frameworkEvent.load();
}
void Framework::eventProcessed() {
    frameworkEvent.store(FrameworkEvent::Empty);
}

ChosenExtensions Framework::getExtensions() {
    return chosen;
}

string Framework::getInput() {
    return *input_buffer.load();
}
void Framework::setTable(
    const std::map<std::string, int>& types_new,
    const std::map<std::string, std::map<std::string, int>>& sub_types_new) {

    table = vectorTable();

    // Превращаем unordered_map в map для вывода в алфавитном порядке
    types.clear(); types.insert(types_new.begin(), types_new.end());

    sub_types.clear();
    for (auto& [key, unordered_sub_map] : sub_types_new) {
        std::map<std::string, int> ordered_sub_map(unordered_sub_map.begin(), unordered_sub_map.end());
        sub_types[key] = ordered_sub_map;
    }

    for (auto& [type, cnt] : types) {
        Table_row_expandable row;
        row.type = type;
        row.cnt = cnt;

        if (sub_types.contains(type)) {
            for (auto& [sub_type, sub_cnt] : sub_types.at(type)) {
                row.sub_types.push_back(sub_type);
                row.sub_count.push_back(sub_cnt);
            }
        }
        table.expandable_rows.push_back(row);
    }
}
void Framework::setInput(std::string new_input) {
    input_buffer.store(std::make_shared<std::string>(new_input));
}

void Framework::setErrorMassege(std::shared_ptr<std::string> new_error) {
    // Пустое сообщение = нет ошибки
    if (new_error) {
        input_is_correct = false;
    } else {
        input_is_correct = true;
    }
    error_massege.store(new_error);
}
void Framework::setProgressBarText(std::shared_ptr<std::string> new_text) {
    progress_bar_text.store(new_text);
}

void Framework::drawErrorMessage() {
    if (error_massege.load()) {
        ImGui::InputText("##error_text", &*error_massege.load(), ImGuiInputTextFlags_ReadOnly);
    }
}

void Framework::move_by_drag_titlebar() {

    titleBarRect.width = (float)GetScreenWidth();

    Vector2 mousePos = GetMousePosition();

    // Двигание окна, зажатием тайтлбара
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(mousePos, titleBarRect))
    {
        if (mousePos.x < (GetScreenWidth() - TITLE_BAR_HEIGHT)) { // Проверяем что не попали кнопку закрытия
            isDragging = true;
            dragOffset = mousePos;
        }
    }

    // Двигаем окно
    if (isDragging) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 currentWinPos = GetWindowPosition();
            int nextX = (int)(currentWinPos.x + mousePos.x - dragOffset.x);
            int nextY = (int)(currentWinPos.y + mousePos.y - dragOffset.y);
            SetWindowPosition(nextX, nextY);
        } else {
            isDragging = false; // Отпустили мышь — закончили движение
        }
    }
}
