#include "imgui.h"                  // Фреймворк
#include "raylib.h"                 // Бэкенд для ImGui
#include "rlImGui.h"                // Мостик между RayLib и ImGui
#include "JetBrainsMono.h"          // Подключаем шрифт
#include <misc/cpp/imgui_stdlib.h>  // Работа с std::string в ImGui
#include "save_dialog.h"
#include "framework.h"

using namespace std;

Framework::Framework() {
        SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_HIGHDPI);
        InitWindow(WIDTH, HEIGHT, "Token Counter");
        SetTargetFPS(60);

        rlImGuiSetup(true); // Иниициализация ImGui и интеграция с RayLib
                            // Запускает внутри себя ImGui::CreateContext()

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

        // Делаем шрифтом по умлочанию
        if (regular_font != nullptr) {
            io.FontDefault = regular_font;
        }

        ImGui::GetIO().IniFilename = nullptr; // Отключаем создание imgui.ini


        // КАСТОМНЫЙ ТАЙТЛБАР
        titleBarRect = { 0, 0, WIDTH, TITLE_BAR_HEIGHT };
        isDragging = false;
        dragOffset = { 0, 0 }; // Координаты мыши относительно левого верхнего угла окна

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
void Framework::update() {
    float TABLE_HEIGHT = HEIGHT - TITLE_BAR_HEIGHT - BUTTON_BAR; // обновляем высоту таблицы если изменится размер окна

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
    // Принудительно отключаем DPI scaling: всегда 1:1.
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
        draw_titlebar();

        ImGui::PushStyleColor(ImGuiCol_ChildBg, BASIC_COLOR);

            // Контент в основном окне
            const float content_top = TITLE_BAR_HEIGHT;
            const float content_height = HEIGHT - content_top - BUTTON_BAR;
            ImGui::SetCursorPosY(content_top);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, FRAME_PADDING_VEC);
                ImGui::BeginChild("Content", ImVec2(0, content_height), ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding);
                    if (work_state == WAITING_INPUT) {
                        draw_input_field();

                        // Кнопка Обзор
                        ImGui::SameLine();
                        observe_button();

                        ImGui::Dummy(ImVec2(0.0f, 15.0f));
                        ImGui::PushStyleColor(ImGuiCol_Text, BASIC_COLOR_ACTIVATED);
                            ImGui::Text("! URL запрос может быть прерван со сторны GitHub,");
                            ImGui::Text("  если проект слишком большой.");
                            ImGui::Text("  Для анализа больших проектов рекомендуется скачать их.");
                        ImGui::PopStyleColor();

                        draw_check_box();

                        // Сообщение об ошибке
                        if (!input_is_correct) { draw_error_message(); }
                    }
                    else if (work_state == WORK_IN_PROGRESS) {
                        draw_progress_bar();
                    }
                    else if (work_state == SHOW_TABLE) {
                        draw_table();
                    }
                ImGui::EndChild();
                ImGui::PopStyleVar();


                // Контент снизу с кнопками
                ImGui::SetCursorPosY(HEIGHT-BUTTON_BAR);
                ImGui::BeginChild("Buttons", ImVec2(0, BUTTON_BAR), ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding);
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, FRAME_PADDING_VEC);
                        if (work_state == WAITING_INPUT) {
                            continue_button();
                        } else if (work_state == SHOW_TABLE) {
                            save_button();

                            back_button();
                        }
                    ImGui::PopStyleVar();
                ImGui::EndChild();
            ImGui::PopStyleColor();

    ImGui::End();
    ImGui::PopStyleVar();


    rlImGuiEnd();
    EndDrawing();
}
Framework::~Framework() {
    rlImGuiShutdown();
    CloseWindow();
}

void Framework::draw_titlebar() {
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
                work_state = SHOULD_CLOSE;
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

void Framework::draw_input_field() {
    // Поле ввода
    ImGui::GetStyle().FrameBorderSize = 1.8f;
    ImGui::PushStyleColor(ImGuiCol_FrameBg, BASIC_COLOR);
    ImGui::PushStyleColor(ImGuiCol_Border, BASIC_COLOR_HIGHLIGHTED);
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, TEXT_COLOR);
        ImGui::SetNextItemWidth(-(OBSERVE_BUTTON_WIDTH + FRAME_PADDING_VEC.x));
            if (ImGui::InputTextWithHint("##dir_path_unput", "Введите URL или путь к локальной директории", &input_buffer, ImGuiInputTextFlags_EnterReturnsTrue)) {
                dir_path = input_buffer;
                work_state = JUST_INPUT;
            }

    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::GetStyle().FrameBorderSize = 0.0f;
}
void Framework::draw_progress_bar() {
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, TITLEBAR_COLOR_ACTIVATED);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, TITLEBAR_COLOR);
    ImGui::PushStyleColor(ImGuiCol_Text, TEXT_COLOR);
        ImVec2 cursor_pos_before_progress_bar = ImGui::GetCursorPos();
        ImGui::ProgressBar(progress_bar_fraction, ImVec2(-1, 25), "");

        cursor_pos_before_progress_bar.y += 3;
        ImGui::SetCursorPos(cursor_pos_before_progress_bar);
        ImGui::Text((" " + progress_bar_text).c_str(), ImVec2(-1, 25));
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
}
void Framework::draw_table() {
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

void Framework::draw_check_box() {
    ImGui::Dummy(ImVec2(0.0f, 15.0f));

    ImGui::GetStyle().FrameBorderSize = 1.5f;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::Checkbox(".cpp", &cpp);
        ImGui::Checkbox(".h", &h);
        ImGui::Checkbox(".hpp", &hpp);
    ImGui::PopStyleVar();
    ImGui::GetStyle().FrameBorderSize = 0.0f;
}
void Framework::save_button() {
    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - (BUTTON_SIZE.x + FRAME_PADDING_VEC.x), ImGui::GetWindowHeight()-(BUTTON_SIZE.y + FRAME_PADDING_VEC.y)));

    if (ImGui::Button("Сохранить", BUTTON_SIZE)) {
        open_save_dialog();
    }
    if (save_dialog) {
        save_dialog_update(dir_path, types, sub_types);
    }
}
void Framework::back_button() {
    ImGui::SetCursorPos(ImVec2(FRAME_PADDING_VEC.x, ImGui::GetWindowHeight()-(BUTTON_SIZE.y + FRAME_PADDING_VEC.y)));

    if (ImGui::Button("Назад", BUTTON_SIZE)) {
        work_state = WAITING_INPUT;
        input_is_correct = true;
        error_massege = "";
    }
}
void Framework::observe_button() {
    if (ImGui::Button("Обзор", ImVec2(OBSERVE_BUTTON_WIDTH, 0))) {
        open_select_dialog();
    }
    if (select_dialog) {
        select_dialog_update(input_buffer); // обновляет input_buffer
    }
    // ImGui::PopStyleColor();
    // ImGui::PopStyleColor();
}
void Framework::continue_button() {
    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth()-(BUTTON_SIZE.x+FRAME_PADDING_VEC.x), ImGui::GetWindowHeight()-(BUTTON_SIZE.y+FRAME_PADDING_VEC.y)));
    if (ImGui::Button("Далее", BUTTON_SIZE)) {
        dir_path = input_buffer;
        work_state = JUST_INPUT;
        check_box_flag = cpp + 2*h + 4*hpp;
    }
}

string Framework::get_input() {
    return dir_path;
}
void Framework::set_progress_bar(float value) {
    progress_bar_fraction = value;
}
void Framework::set_table(
    const std::unordered_map<std::string, int>& types_new,
    const std::unordered_map<std::string, std::unordered_map<std::string, int>>& sub_types_new) {


    table = Table();

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

void Framework::incorrect_input(std::string new_error_massege) {
    input_is_correct = false;
    error_massege = new_error_massege;
}
void Framework::draw_error_message() {
    ImGui::InputText("##error_text", &error_massege, ImGuiInputTextFlags_ReadOnly);
}
