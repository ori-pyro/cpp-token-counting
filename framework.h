#include "imgui.h"      // Фреймворк
#include "raylib.h"     // Бэкенд для ImGui
#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include <map>
#include <atomic>

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

enum Work_state {WAITING_INPUT, JUST_INPUT, WORK_IN_PROGRESS, SHOULD_CLOSE, SHOW_TABLE};

class Framework {
public:
    float WIDTH                 = 600.0f;
    float HEIGHT                = 400.0f;
    float TITLE_BAR_HEIGHT      = 30.0f;
    float BUTTON_BAR            = 60.0f;
    float TABLE_HEIGHT          = HEIGHT - TITLE_BAR_HEIGHT - BUTTON_BAR;
    float OBSERVE_BUTTON_WIDTH  = 80.0f;
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

    std::string dir_path = "";
    std::string input_buffer = "";

    uint8_t check_box_flag = 0;
    bool cpp = false;
    bool h = false;
    bool hpp = false;

    bool running = false;

    bool isDragging;
    Rectangle titleBarRect;
    Vector2 dragOffset;

    std::atomic<Screen> screen;
    std::atomic<FrameworkEvent> frameworkEvent;

    Work_state work_state = WAITING_INPUT;
    std::string error_massege;

    std::string progress_bar_text = "";
    std::atomic<int> curr{ 0 };
    std::atomic<int> total{ 0 };
    std::atomic<std::shared_ptr<std::string>> filename;
    float progress_bar_fraction;

    bool input_is_correct = true;

    Framework();
    void update();
    void loop();
    ~Framework();

    void draw_titlebar();
    void move_by_drag_titlebar();

    void draw_input_field();
    void draw_progress_bar();
    void draw_table();

    void draw_check_box();
    void save_button();
    void back_button();
    void observe_button();
    void continue_button();

    std::string getInput() const;
    Screen getScreen();
    FrameworkEvent getEvent();

    void setScreen(Screen);
    void setProgressBarText(std::shared_ptr<std::string>);
    void setErrorMassege(std::shared_ptr<std::string>);
    void set_progress_bar(float);
    void setTable(
        const std::map<std::string, int>& type,
        const std::map<std::string, std::map<std::string, int>>& sub_type
    );

    void incorrect_input(std::string);
    void draw_error_message();
};
