#include "imgui.h"                  // Фреймворк
#include "raylib.h"                 // Бэкенд для ImGui
#include "rlImGui.h"                // Мостик между RayLib и ImGui
#include "JetBrainsMono.h"          // Подключаем шрифт
#include <misc/cpp/imgui_stdlib.h>  // Работа с std::string в ImGui
#include "save_dialog.h"
#include "framework.h"

#include <iostream>

using namespace std;

Framework::Framework() {
        SetConfigFlags(FLAG_WINDOW_UNDECORATED);        // Настройки внешнего окна Raylib (без рамок)
        InitWindow(WIDTH, HEIGHT, "Lexeme Counter");
        SetTargetFPS(60);

        rlImGuiSetup(true); // Иниициализация ImGui и интеграция с RayLib
                            // Запускает внутри себя ImGui::CreateContext()

        ImGuiIO& io = ImGui::GetIO();

        // ШРИФТЫ
        regular_font = io.Fonts->AddFontFromMemoryCompressedTTF(
                myFontData_compressed_data,        // Имя массива из JetBrainsMonoFont.h
                myFontData_compressed_size,        // Размер массива из JetBrainsMonoFont.h
                24.0f,                             // Размер шрифта в пикселях
                nullptr,                           // Настройки (оставляем nullptr)
                io.Fonts->GetGlyphRangesCyrillic() // ВКЛЮЧАЕМ РУССКИЙ ЯЗЫК
            );
        small_font = io.Fonts->AddFontFromMemoryCompressedTTF(
                myFontData_compressed_data,        // Имя массива из JetBrainsMonoFont.h
                myFontData_compressed_size,        // Размер массива из JetBrainsMonoFont.h
                18.0f,                             // Размер шрифта в пикселях
                nullptr,                           // Настройки (оставляем nullptr)
                io.Fonts->GetGlyphRangesCyrillic() // ВКЛЮЧАЕМ РУССКИЙ ЯЗЫК
            );

        // Делаем шрифтом по умлочанию
        if (regular_font != nullptr) {
            io.FontDefault = regular_font;
        }

        ImGui::GetIO().IniFilename = nullptr; // Отключаем создание imgui.ini


        // КАСТОМНЫЙ ТАЙТЛБАР
        titleBarRect = { 0, 0, WIDTH, TITLE_BAR_HEIGHT };
        isDragging = false;
        dragOffset = { 0, 0 }; // Координаты мыши относительно левого верхнего угла окна

        // ГЛОБАЛЬНЫЕ НАСТРОЙКИ СТИЛЯ КНОПОК
        ImGuiStyle& style = ImGui::GetStyle();
        style.Colors[ImGuiCol_Button]        = BASIC_COLOR;             // Обычная
        style.Colors[ImGuiCol_ButtonHovered] = BASIC_COLOR_HIGHLIGHTED; // При наведении
        style.Colors[ImGuiCol_ButtonActive]  = BASIC_COLOR_ACTIVATED;   // При нажатии

        // СТИЛЬ РАМОК
        style.Colors[ImGuiCol_Border] = TITLEBAR_COLOR_ACTIVATED;
        style.Colors[ImGuiCol_TableBorderLight] = TITLEBAR_COLOR_ACTIVATED;
        style.Colors[ImGuiCol_TableBorderStrong] = TITLEBAR_COLOR_ACTIVATED;


}

void Framework::draw_GUI() {
    move_by_drag_titlebar();

    BeginDrawing();
    ClearBackground(DARKGRAY);

    // Если мышь за пределами окна,
    // то сообщаем ImGui об этом, иначе он
    // будет думать, что курсор осталься на краю окна
    if (!IsCursorOnScreen()) {
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
        ImGui::GetIO().MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    } else {
        ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
    }

    rlImGuiBegin();

    // Убираем внутренние отступы окна в ноль
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                            ImGuiWindowFlags_NoResize |
                            ImGuiWindowFlags_NoMove |
                            ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_NoBringToFrontOnFocus;


    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(GetScreenWidth(), GetScreenHeight()), ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("MainPanel", nullptr, flags);



    draw_titlebar();

    // Контент в основном окне
    ImGui::SetCursorPosY(TITLE_BAR_HEIGHT);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, BASIC_COLOR);
    ImGui::BeginChild("Content", ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding);
        if (work_state == WAITING_INPUT) {
            draw_input_field();
            if (!input_is_correct) {
                draw_error_message();
            }
        }
        else if (work_state == WORK_IN_PROGRESS) {
            draw_progress_bar();
                    }
        else if (work_state == SHOW_TABLE) {
            draw_table();
            save_button();
            back_button();
        }
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    ImGui::End();

    rlImGuiEnd();
    EndDrawing();
}

void Framework::text_input() {} //TODO

Framework::~Framework() {
    rlImGuiShutdown();
    CloseWindow();
}

string Framework::get_input() {
    return dir_path;
}

void Framework::set_progress_bar(float value) {
    progress_bar_fraction = value;
}

void Framework::draw_progress_bar() {
    ImGui::ProgressBar(progress_bar_fraction, ImVec2(-1, 30), progress_bar_text.c_str());
}

void Framework::draw_titlebar() {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, TITLEBAR_COLOR);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);

    ImGui::BeginChild("FakeTitleBarVisual", ImVec2(0, TITLE_BAR_HEIGHT), ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);

        // Заголовок тайтлбара
        ImGui::PushFont(regular_font, 20.0f);
            ImGui::Text("Lexeme Counter");
        ImGui::PopFont();

        ImGui::SameLine(ImGui::GetWindowWidth() - CLOSE_BUTTON_WIDTH);
        ImGui::SetCursorPosY(0);

        // Стиль кнопки закрытия
        ImGui::PushStyleColor(ImGuiCol_Button, TITLEBAR_COLOR);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, CLOSE_BUTTON_ACTIVE_COLOR);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, CLOSE_BUTTON_HOVERED_COLOR);
            if (ImGui::Button("✕", ImVec2(CLOSE_BUTTON_WIDTH, TITLE_BAR_HEIGHT))) {
                work_state = SHOULD_CLOSE;
            }
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();


        ImGui::SameLine(ImGui::GetWindowWidth() - CLOSE_BUTTON_WIDTH * 2);
        ImGui::SetCursorPosY(0);

        // Стиль кнопки сворачивания
        ImGui::PushStyleColor(ImGuiCol_Button, TITLEBAR_COLOR);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, TITLEBAR_COLOR_HIGHLIGHTED);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, TITLEBAR_COLOR_ACTIVATED);
            if (ImGui::Button("—", ImVec2(CLOSE_BUTTON_WIDTH, TITLE_BAR_HEIGHT))) {
                MinimizeWindow();
            }
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();

    ImGui::EndChild();

    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    ImGui::PopStyleVar();
}

void Framework::draw_input_field() {
    // Поле ввода
    ImGui::PushStyleColor(ImGuiCol_FrameBg, BASIC_COLOR_HIGHLIGHTED);
        ImGui::SetNextItemWidth(-INPUT_BUTTON_WIDTH);
        ImGui::PushStyleColor(ImGuiCol_TextDisabled, BASIC_COLOR_ACTIVATED);
            if (ImGui::InputTextWithHint("##dir_path_unput", "Введите путь к дирректории", &input_buffer, ImGuiInputTextFlags_EnterReturnsTrue)) {
                dir_path = input_buffer;
                work_state = JUST_INPUT;
            }
        ImGui::PopStyleColor();
    ImGui::PopStyleColor();

    // Кнопка ввода
    ImGui::SameLine();
    if (ImGui::Button("Ввод", ImVec2(INPUT_BUTTON_WIDTH, 0))) {
        dir_path = input_buffer;
        work_state = JUST_INPUT;
    }
}

void Framework::draw_table() {
    if (ImGui::BeginTable("my_table", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        ImGui::TableSetupColumn("Token");
        ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        for (int i = 0; i < token_names.size(); i++) {
            ImGui::TableNextColumn(); ImGui::Text(token_names[i].c_str());
            ImGui::TableNextColumn(); ImGui::Text(std::to_string(token_count[i]).c_str());
        }
        ImGui::EndTable();
    }
}

void Framework::set_table(const std::vector<std::string>& string_column, const std::vector<int>& int_column) {
    token_names = string_column;
    token_count = int_column;
}

void Framework::move_by_drag_titlebar() {

    titleBarRect.width = (float)GetScreenWidth();

    Vector2 mousePos = GetMousePosition();

    // Двигание окна, зажатием тайтлбара
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(mousePos, titleBarRect))
    {
        if (mousePos.x < (GetScreenWidth() - CLOSE_BUTTON_WIDTH)) { // Проверяем что не попали кнопку закрытия
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

void Framework::save_button() {
    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth()-150.0f, ImGui::GetWindowHeight()-55.0f));
    if (ImGui::Button("Сохранить", ImVec2(130, 40))) {
        open_save_dialog();
    }
    if (save_dialog) {
        save_dialog_update(token_names, token_count);
    }
}

void Framework::back_button() {
    ImGui::SetCursorPos(ImVec2(15.0f, ImGui::GetWindowHeight()-55.0f));
    if (ImGui::Button("Назад", ImVec2(130, 40))) {
        work_state = WAITING_INPUT;
        input_is_correct = true;
    }
}

void Framework::incorrect_input() {
    input_is_correct = false;
}

void Framework::draw_error_message() {
    ImGui::Text(" Ошибка: Некорректный ввод");
}
