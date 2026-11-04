#include "jo_draftui/controls/behavior.hpp"

#include <algorithm>
#include <cmath>
#include <iterator>

namespace jo::draftui {

// ---------------------------------------------------------------------------
// ButtonBehavior
// ---------------------------------------------------------------------------

void ButtonBehavior::mouse_move(bool inside, bool enabled) {
    visual.enabled = enabled;
    visual.hover = enabled && inside;
    if (!enabled) {
        pressed = false;
        visual.pressed = false;
    }
}

bool ButtonBehavior::mouse_down(bool inside, bool enabled) {
    if (!enabled || !inside) return false;
    pressed = true;
    visual.pressed = true;
    visual.hover = true;
    return true;
}

bool ButtonBehavior::mouse_up(bool inside, bool enabled) {
    const bool clicked = enabled && pressed && inside;
    pressed = false;
    visual.pressed = false;
    visual.enabled = enabled;
    visual.hover = enabled && inside;
    return clicked;
}

void ButtonBehavior::cancel() {
    pressed = false;
    visual.pressed = false;
}

// ---------------------------------------------------------------------------
// FocusRing
// ---------------------------------------------------------------------------

void FocusRing::set_order(std::vector<int> order) {
    order_ = std::move(order);
    if (std::find(order_.begin(), order_.end(), focused_) == order_.end()) {
        focused_ = -1;
    }
}

void FocusRing::clear() {
    order_.clear();
    focused_ = -1;
}

bool FocusRing::focus(int id) {
    if (std::find(order_.begin(), order_.end(), id) == order_.end()) {
        return false;
    }
    focused_ = id;
    return true;
}

bool FocusRing::focus_next() {
    if (order_.empty()) return false;
    if (focused_ == -1) {
        focused_ = order_.front();
        return true;
    }
    const auto current = std::find(order_.begin(), order_.end(), focused_);
    if (current == order_.end() || std::next(current) == order_.end()) {
        focused_ = order_.front();
    } else {
        focused_ = *std::next(current);
    }
    return true;
}

bool FocusRing::focus_previous() {
    if (order_.empty()) return false;
    if (focused_ == -1) {
        focused_ = order_.back();
        return true;
    }
    const auto current = std::find(order_.begin(), order_.end(), focused_);
    if (current == order_.begin() || current == order_.end()) {
        focused_ = order_.back();
    } else {
        focused_ = *std::prev(current);
    }
    return true;
}

int FocusRing::focused() const { return focused_; }

// ---------------------------------------------------------------------------
// TextInputBehavior
// ---------------------------------------------------------------------------

bool TextInputBehavior::key(TextInputState& state, int key,
                            Modifiers modifiers) {
    if (modifiers.ctrl && key == 'C') {
        const std::wstring selected = input_selected_text(state);
        if (!selected.empty() && write_clipboard) {
            write_clipboard(selected);
        }
        return true;
    }
    if (modifiers.ctrl && key == 'X') {
        std::wstring selected;
        if (input_cut(state, &selected) && write_clipboard) {
            write_clipboard(selected);
        }
        return true;
    }
    if (modifiers.ctrl && key == 'V') {
        if (read_clipboard) {
            const std::wstring text = read_clipboard();
            if (!text.empty()) input_paste(state, text);
        }
        return true;
    }
    return input_key(state, key, modifiers.shift, modifiers.ctrl);
}

bool TextInputBehavior::character(TextInputState& state, wchar_t character) {
    return input_char(state, character);
}

void TextInputBehavior::composition(TextInputState& state,
                                    std::wstring_view text,
                                    bool committed) {
    input_composition(state, text, committed);
}

TextMenuState TextInputBehavior::menu_state(const TextInputState& state,
                                             bool has_clipboard) const {
    const bool selection = state.caret != state.anchor;
    TextMenuState menu;
    menu.cut = selection;
    menu.copy = selection;
    menu.paste = has_clipboard;
    menu.select_all = !state.text.empty();
    return menu;
}

bool TextInputBehavior::execute(TextInputState& state, TextCommand command) {
    switch (command) {
        case TextCommand::cut: {
            std::wstring selected;
            if (!input_cut(state, &selected)) return false;
            if (write_clipboard) write_clipboard(selected);
            return true;
        }
        case TextCommand::copy: {
            const std::wstring selected = input_selected_text(state);
            if (selected.empty()) return false;
            if (write_clipboard) write_clipboard(selected);
            return true;
        }
        case TextCommand::paste: {
            if (!read_clipboard) return false;
            const std::wstring text = read_clipboard();
            return !text.empty() && input_paste(state, text);
        }
        case TextCommand::select_all:
            input_select_all(state);
            return true;
    }
    return false;
}

bool TextInputBehavior::double_click(Canvas& canvas, TextInputState& state,
                                      const Rect& rect, float x, float scale) {
    input_click(canvas, state, rect, x, scale);
    input_select_word_at(state, state.caret);
    return true;
}

// ---------------------------------------------------------------------------
// DialogBehavior
// ---------------------------------------------------------------------------

void DialogBehavior::open(std::vector<int> buttons, int initial) {
    buttons_ = std::move(buttons);
    focused_index_ = 0;
    if (!buttons_.empty()) {
        focused_index_ = std::clamp(initial, 0,
                                    static_cast<int>(buttons_.size()) - 1);
    }
}

void DialogBehavior::close() {
    buttons_.clear();
    focused_index_ = 0;
}

bool DialogBehavior::active() const { return !buttons_.empty(); }

int DialogBehavior::focused_button() const {
    if (buttons_.empty()) return -1;
    return buttons_[static_cast<std::size_t>(focused_index_)];
}

bool DialogBehavior::key(int key) {
    if (!active()) return false;
    if (key == VK_ESCAPE) {
        close();
        return true;
    }
    if (key == VK_RETURN) {
        close();
        return true;
    }
    if (key == VK_TAB) {
        focused_index_ = (focused_index_ + 1) %
                         static_cast<int>(buttons_.size());
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// TooltipState / MenuState
// ---------------------------------------------------------------------------

void autoscroll_begin(AutoscrollState& state, float x, float y,
                      std::int64_t now_ms) {
    state.active = true;
    state.origin_x = x;
    state.origin_y = y;
    state.velocity = 0.0f;
    state.accum = 0.0f;
    state.last_tick = now_ms;
    state.middle_down = true;
    state.mode = 0;
}

int autoscroll_move(AutoscrollState& state, float y, int dpi,
                    std::int64_t now_ms) {
    if (!state.active) return 0;
    const float distance =
        (y - state.origin_y) * 96.0f / std::max(1, dpi);
    constexpr float dead_zone = 15.0f;
    if (std::fabs(distance) <= dead_zone) {
        state.velocity = 0.0f;
    } else {
        const float sign = distance > 0.0f ? 1.0f : -1.0f;
        state.velocity = std::pow(std::fabs(distance), 2.2f) * 0.000008f *
                         sign;
        state.velocity = std::clamp(state.velocity, -8.0f, 8.0f);
        if (state.middle_down) state.mode = 1;
    }
    return autoscroll_advance(state, now_ms);
}

int autoscroll_advance(AutoscrollState& state, std::int64_t now_ms) {
    if (!state.active) return 0;
    const int elapsed = std::clamp(
        static_cast<int>(now_ms - state.last_tick), 1, 50);
    state.last_tick = now_ms;
    if (state.velocity == 0.0f) return 0;
    state.accum += state.velocity * static_cast<float>(elapsed);
    const int step = static_cast<int>(state.accum);
    state.accum -= static_cast<float>(step);
    return step;
}

bool autoscroll_up(AutoscrollState& state) {
    if (!state.active) return false;
    state.middle_down = false;
    if (state.mode == 1) {
        autoscroll_stop(state);
        return false;
    }
    state.mode = 2;
    return true;
}

void autoscroll_stop(AutoscrollState& state) {
    state.active = false;
    state.origin_x = 0.0f;
    state.origin_y = 0.0f;
    state.velocity = 0.0f;
    state.accum = 0.0f;
    state.last_tick = 0;
    state.middle_down = false;
    state.mode = 0;
}

void TooltipState::hover(int id, std::int64_t now_ms) {
    if (target != id) {
        target = id;
        shown_at = now_ms;
    }
}

void TooltipState::leave() {
    target = -1;
    shown_at = 0;
}

bool TooltipState::visible(std::int64_t now_ms,
                           std::int64_t delay_ms) const {
    return target >= 0 && now_ms - shown_at >= delay_ms;
}

bool MenuState::open(int index) {
    if (index < 0) return false;
    open_index = index;
    hover_index = index;
    return true;
}

void MenuState::close() {
    open_index = -1;
    hover_index = -1;
}

bool MenuState::key(int key, int item_count) {
    if (open_index < 0 || item_count <= 0) return false;
    if (key == VK_ESCAPE) {
        close();
        return true;
    }
    if (key == VK_DOWN) {
        hover_index = (hover_index + 1) % item_count;
        return true;
    }
    if (key == VK_UP) {
        hover_index = (hover_index - 1 + item_count) % item_count;
        return true;
    }
    return false;
}

}  // namespace jo::draftui
