#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <vector>
#include <cstdint>

#include "jo_draftui/controls/kit.hpp"

namespace jo::draftui {

struct Modifiers {
    bool shift = false;
    bool ctrl = false;
    bool alt = false;
};

/// Immediate-mode interaction state for buttons, icon actions and rows.
struct ButtonBehavior {
    Interaction visual{};
    bool pressed = false;

    void mouse_move(bool inside, bool enabled);
    bool mouse_down(bool inside, bool enabled);
    bool mouse_up(bool inside, bool enabled);
    void cancel();
};

/// A simple focus chain. The owner supplies stable integer IDs; Tab/Shift+Tab
/// move through them without knowing any control type.
class FocusRing {
public:
    void set_order(std::vector<int> order);
    void clear();
    bool focus(int id);
    bool focus_next();
    bool focus_previous();
    int focused() const;

private:
    std::vector<int> order_;
    int focused_ = -1;
};

/// Keyboard and clipboard behavior for TextInputState. The application owns
/// the actual clipboard; the callbacks keep the platform dependency outside
/// the control library.
struct TextInputBehavior {
    std::function<std::wstring()> read_clipboard;
    std::function<void(std::wstring_view)> write_clipboard;

    bool key(TextInputState& state, int key, Modifiers modifiers);
    bool character(TextInputState& state, wchar_t character);
    void composition(TextInputState& state, std::wstring_view text,
                     bool committed);
    TextMenuState menu_state(const TextInputState& state,
                             bool has_clipboard) const;
    bool execute(TextInputState& state, TextCommand command);
    bool double_click(Canvas& canvas, TextInputState& state, const Rect& rect,
                      float x, float scale);
};

/// Minimal modal focus order: Esc cancels, Enter confirms, Tab cycles the
/// dialog's action buttons.
class DialogBehavior {
public:
    void open(std::vector<int> buttons, int initial = 0);
    void close();
    bool active() const;
    int focused_button() const;
    bool key(int key);

private:
    std::vector<int> buttons_;
    int focused_index_ = 0;
};

/// Delayed tooltip visibility. The owner passes a stable target ID and the
/// current tick count; IDs are compared so moving between targets restarts
/// the delay.
struct AutoscrollState {
    bool active = false;
    float origin_x = 0.0f;
    float origin_y = 0.0f;
    float velocity = 0.0f;
    float accum = 0.0f;
    std::int64_t last_tick = 0;
    bool middle_down = false;
    int mode = 0;  // 0 = idle, 1 = moved while held, 2 = released
};

void autoscroll_begin(AutoscrollState& state, float x, float y,
                      std::int64_t now_ms);
int autoscroll_move(AutoscrollState& state, float y, int dpi,
                    std::int64_t now_ms);
int autoscroll_advance(AutoscrollState& state, std::int64_t now_ms);
bool autoscroll_up(AutoscrollState& state);
void autoscroll_stop(AutoscrollState& state);

struct TooltipState {
    int target = -1;
    std::int64_t shown_at = 0;
    void hover(int id, std::int64_t now_ms);
    void leave();
    bool visible(std::int64_t now_ms, std::int64_t delay_ms = 500) const;
};

/// Open/hover state for a menu bar or popup list.
struct MenuState {
    int open_index = -1;
    int hover_index = -1;
    bool open(int index);
    void close();
    bool key(int key, int item_count);
};

}  // namespace jo::draftui
