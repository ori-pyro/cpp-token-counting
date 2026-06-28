#include "imgui.h"      // Фреймворк
#include "raylib.h"     // Бэкенд для ImGui
#include <string>
#include <vector>


enum Work_state {WAITING_INPUT, JUST_INPUT, WORK_IN_PROGRESS, SHOULD_CLOSE, SHOW_TABLE};


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
    const ImVec4 TEXT_COLOR = ImVec4(184.0/255.0f, 192/255.0f, 224/255.0f, 1.0f);
    const ImVec4 TRANSPERENT = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);



    ImFont* regular_font = nullptr;
    ImFont* small_font = nullptr;

    std::string dir_path = "";
    std::string input_buffer = "";

    bool isDragging;
    Rectangle titleBarRect;
    Vector2 dragOffset;

    Work_state work_state = WAITING_INPUT;

    std::string progress_bar_text = "";
    float progress_bar_fraction;

    std::vector<std::string> token_names;
    std::vector<int> token_count;
    bool input_is_correct = true;
    bool save_dialog_opened = false;

    Framework();
    ~Framework();

    std::string get_input();
    void draw_GUI();
    void text_input();
    void set_progress_bar(float);
    void draw_progress_bar();
    void draw_titlebar();
    void draw_input_field();
    void move_by_drag_titlebar();
    void draw_table();
    void save_button();
    void back_button();
    void incorrect_input();
    void draw_error_message();
    void set_table(const std::vector<std::string>&, const std::vector<int>&);
};
