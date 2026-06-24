#include <iostream>
#include "imgui.h"
#include "raylib.h"
#include "rlImGui.h"
#include "JetBrainsMono.h"          // Подключаем шрифт
#include <misc/cpp/imgui_stdlib.h>
using namespace std;

const int WIDTH = 800;
const int HEIGHT = 300;
const int TITLE_BAR_HEIGHT = 40;
const int CLOSE_BUTTON_WIDTH = 40;


int main() {
    // Настройки внешнего окна Raylib (без рамок)
    SetConfigFlags(FLAG_WINDOW_UNDECORATED);
    InitWindow(WIDTH, HEIGHT, "Lexeme Counter");
    SetTargetFPS(60);

    rlImGuiSetup(true);

    ImGuiIO& io = ImGui::GetIO();

    ImFont* myCustomFont = io.Fonts->AddFontFromMemoryCompressedTTF(
            myFontData_compressed_data,        // Имя массива из JetBrainsMonoFont.h
            myFontData_compressed_size,        // Размер массива из JetBrainsMonoFont.h
            24.0f,                             // Размер шрифта в пикселях (например, 16)
            nullptr,                           // Настройки (оставляем nullptr)
            io.Fonts->GetGlyphRangesCyrillic() // <--- ВКЛЮЧАЕМ РУССКИЙ ЯЗЫК
        );

    // 4. ДЕЛАЕМ ШРИФТ ПО УМОЛЧАНИЮ
    if (myCustomFont != nullptr) {
        io.FontDefault = myCustomFont;
    }


    ImGui::GetIO().IniFilename = nullptr;

    // Прямоугольник нашей кастомной плашки
    Rectangle titleBarRect = { 0, 0, WIDTH, TITLE_BAR_HEIGHT };

    // Переменные для отслеживания состояния перетаскивания
    bool isDragging = false;
    Vector2 dragOffset = { 0, 0 };

    while (!WindowShouldClose()) {
        titleBarRect.width = (float)GetScreenWidth();

        Vector2 mousePos = GetMousePosition();

        // 1. Фиксируем момент клика по плашке
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
            CheckCollisionPointRec(mousePos, titleBarRect))
        {
            // Проверяем, что кликнули не на кнопку закрытия (правые 45 пикселей)
            if (mousePos.x < (GetScreenWidth() - CLOSE_BUTTON_WIDTH)) {
                isDragging = true;
                // Запоминаем, в каком именно месте плашки мы зажали курсор
                dragOffset = mousePos;
            }
        }

        // 2. Пока кнопка зажата — двигаем окно вслед за глобальными координатами
        if (isDragging) {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                // Получаем текущую позицию окна на рабочем столе
                Vector2 currentWinPos = GetWindowPosition();

                // Рассчитываем, куда должен переместиться левый верхний угол окна,
                // чтобы курсор оставался ровно в той же точке плашки, где мы кликнули
                int nextX = (int)(currentWinPos.x + mousePos.x - dragOffset.x);
                int nextY = (int)(currentWinPos.y + mousePos.y - dragOffset.y);

                SetWindowPosition(nextX, nextY);
            } else {
                isDragging = false; // Отпустили мышь — закончили движение
            }
        }

        BeginDrawing();
        ClearBackground(DARKGRAY);

        rlImGuiBegin();

        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(GetScreenWidth(), GetScreenHeight()), ImGuiCond_Always);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                                 ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoMove |
                                 ImGuiWindowFlags_NoCollapse |
                                 ImGuiWindowFlags_NoBringToFrontOnFocus;

        // 1. Убираем внутренние отступы окна в ноль ПЕРЕД созданием MainPanel
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        ImGui::Begin("MainPanel", nullptr, flags);

        // ====================================================================
        // ВИЗУАЛ КАСТОМНОЙ ПЛАШКИ
        // ====================================================================

        // Теперь эта панель встанет идеально вплотную к краям внешнего окна
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
        ImGui::BeginChild("FakeTitleBarVisual", ImVec2(0, TITLE_BAR_HEIGHT), ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);

            ImGui::SetCursorPosY(10);
            ImGui::Text("Lexeme Counter");

            ImGui::SameLine(ImGui::GetWindowWidth() - CLOSE_BUTTON_WIDTH);
            ImGui::SetCursorPosY(0);

            // Стиль кнопки закрытия
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(200.0/255.0f, 30.0/255.0f, 30.0/255.0f, 1.0f));
                    if (ImGui::Button("x", ImVec2(TITLE_BAR_HEIGHT, CLOSE_BUTTON_WIDTH))) {
                        break;
                    }
                ImGui::PopStyleColor();
            ImGui::PopStyleColor();

        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();

        // 2. ОБЯЗАТЕЛЬНО возвращаем дефолтные отступы назад,
        // чтобы остальной контент приложения не прилипал к краям экрана!
        ImGui::PopStyleVar();

        // ====================================================================
        // КОНТЕНТ (Ниже плашки)
        // ====================================================================

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
        ImGui::BeginChild("Content", ImVec2(0, 0), ImGuiChildFlags_AlwaysUseWindowPadding);

            static string dir_path = "";
            static string input_buffer = "";

            ImGui::InputText("##dir_path_unput", &input_buffer);

            ImGui::SameLine();

            if (ImGui::Button("Ввод")) {
                dir_path = input_buffer;
            }

            ImGui::Text("Введённый текст: %s", dir_path.c_str());
        ImGui::EndChild();
        ImGui::PopStyleVar();


        ImGui::End();
        rlImGuiEnd();
        EndDrawing();

    }

    rlImGuiShutdown();
    CloseWindow();


    return 0;
}
