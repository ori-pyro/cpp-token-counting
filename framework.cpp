#include <iostream>
#include "imgui.h"                  // Фреймворк
#include "raylib.h"                 // Бэкенд для ImGui
#include "rlImGui.h"                // Мостик между RayLib и ImGui
#include "JetBrainsMono.h"          // Подключаем шрифт
#include <misc/cpp/imgui_stdlib.h>  // Работа с std::string в ImGui
using namespace std;

class Framework {
public:
    const float WIDTH = 800.0f;
    const float HEIGHT = 500.0f;
    const float TITLE_BAR_HEIGHT = 40.0f;
    const float CLOSE_BUTTON_WIDTH = 40.0f;
    const float INPUT_BUTTON_WIDTH = 80.0f;

    // ЦВЕТА
    const ImVec4 BASIC_COLOR = ImVec4(36.0/255.0f, 39/255.0f, 58/255.0f, 1.0f);
    const ImVec4 BASIC_COLOR_HIGHLIGHTED = ImVec4(47.0/255.0f, 50/255.0f, 71/255.0f, 1.0f);
    const ImVec4 BASIC_COLOR_ACTIVATED = ImVec4(68.0/255.0f, 72/255.0f, 94/255.0f, 1.0f);
    const ImVec4 TITLEBAR_COLOR = ImVec4(24.0/255.0f, 25/255.0f, 38/255.0f, 1.0f);
    const ImVec4 TITLEBAR_COLOR_HIGHLIGHTED = ImVec4(39.0/255.0f, 41/255.0f, 57/255.0f, 1.0f);
    const ImVec4 TITLEBAR_COLOR_ACTIVATED = ImVec4(64.0/255.0f, 68/255.0f, 87/255.0f, 1.0f);
    const ImVec4 CLOSE_BUTTON_HOVERED_COLOR = ImVec4(0.8f, 0.12f, 0.12f, 1.0f);
    const ImVec4 CLOSE_BUTTON_ACTIVE_COLOR = ImVec4(0.6f, 0.12f, 0.12f, 1.0f);

    ImFont* regular_font = nullptr;
    ImFont* small_font = nullptr;

    string dir_path = "";
    string input_buffer = "";

    bool isDragging;
    Rectangle titleBarRect;
    Vector2 dragOffset;

    Framework() {
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
    }

    bool draw_GUI() {
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

        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(GetScreenWidth(), GetScreenHeight()), ImGuiCond_Always);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                                 ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoMove |
                                 ImGuiWindowFlags_NoCollapse |
                                 ImGuiWindowFlags_NoBringToFrontOnFocus;

        // ====================================================================
        // ВИЗУАЛ КАСТОМНОГО ТАЙТЛБАРА
        // ====================================================================

        // Убираем внутренние отступы окна в ноль
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("MainPanel", nullptr, flags);

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
                    return false;
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

        // ====================================================================
        // КОНТЕНТ (Ниже плашки)
        // ====================================================================

        ImGui::SetCursorPosY(TITLE_BAR_HEIGHT);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, BASIC_COLOR);
        ImGui::BeginChild("Content", ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding);

            // Поле ввода
            ImGui::PushStyleColor(ImGuiCol_FrameBg, BASIC_COLOR_HIGHLIGHTED);
                ImGui::SetNextItemWidth(-INPUT_BUTTON_WIDTH);
                ImGui::PushStyleColor(ImGuiCol_TextDisabled, BASIC_COLOR_ACTIVATED);
                    if (ImGui::InputTextWithHint("##dir_path_unput", "Введите путь к дирректории", &input_buffer, ImGuiInputTextFlags_EnterReturnsTrue)) {
                        dir_path = input_buffer;
                    }
                ImGui::PopStyleColor();
            ImGui::PopStyleColor();

            // Кнопка ввода
            ImGui::SameLine();
            if (ImGui::Button("Ввод", ImVec2(INPUT_BUTTON_WIDTH, 0))) {
                dir_path = input_buffer;
            }

            ImGui::Text("Введённый текст: %s", dir_path.c_str());
        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();

        ImGui::End();

        rlImGuiEnd();
        EndDrawing();

        return true;
    }

    ~Framework() {
        rlImGuiShutdown();
        CloseWindow();
    }
};



int main() {
    Framework GUI;

    while (!WindowShouldClose()) {
        if (!GUI.draw_GUI()) {
            break;
        }
    }

    return 0;
}
