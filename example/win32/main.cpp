#include "resource.h"

#include <windows.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "jo_draftui/canvas.hpp"
#include "jo_draftui/controls/basic.hpp"
#include "jo_draftui/controls/behavior.hpp"
#include "jo_draftui/controls/kit.hpp"
#include "jo_draftui/theme.hpp"
#include "jo_draftui/win32/window.hpp"

namespace {

using jo::draftui::ButtonStyle;
using jo::draftui::Canvas;
using jo::draftui::Color;
using jo::draftui::D2DCanvas;
using jo::draftui::FontFamily;
using jo::draftui::FontWeight;
using jo::draftui::Interaction;
using jo::draftui::Palette;
using jo::draftui::PopupItem;
using jo::draftui::PopupLayout;
using jo::draftui::Rect;
using jo::draftui::TextAlign;
using jo::draftui::AutoscrollState;
using jo::draftui::Modifiers;
using jo::draftui::TextCommand;
using jo::draftui::TextInputBehavior;
using jo::draftui::TextInputState;
using jo::draftui::TextInputStyle;
using jo::draftui::TextMenuState;
using jo::draftui::TextStyle;
using jo::draftui::Theme;

enum class HitKind { nav, button, icon, segment, toggle, dropdown, list, category, card_action, task_cancel, theme };

struct HitTarget {
    Rect rect{};
    HitKind kind = HitKind::nav;
    int index = 0;
    bool content = false;
};

struct GalleryState {
    bool dark_theme = true;
    bool theme_animating = false;
    float theme_progress = 1.0f;
    std::uint64_t theme_started = 0;
    Palette current_palette = Palette::dark();
    Palette theme_from = Palette::dark();
    Palette theme_to = Palette::dark();
    bool toggle = true;
    bool dialog_open = false;
    TextInputState input_state;
    TextInputState field_input_state;
    TextInputState password_state;
    TextInputState disabled_state;
    TextInputState error_state;
    TextInputState ime_state;
    TextInputState dialog_input_state;
    Rect dialog_input_box{};
    bool dialog_input_active = false;
    bool field_active = false;
    Canvas* canvas = nullptr;
    Rect input_box{};
    Rect field_input_box{};
    Rect client_rect{};
    bool context_open = false;
    int context_hover = -1;
    std::vector<PopupItem> context_items;
    PopupLayout context_layout{};
    Rect window_scroll_track{};
    Rect window_scroll_thumb{};
    AutoscrollState autoscroll;
    float mouse_x = 0.0f;
    float mouse_y = 0.0f;
    Rect dialog_ok{};
    Rect dialog_cancel{};
    float side_width = 232.0f;
    float split_start_x = 0.0f;
    float split_start_width = 232.0f;
    bool split_drag = false;
    bool window_scroll_drag = false;
    bool window_scroll_hover = false;
    float window_scroll_drag_offset = 0.0f;
    float content_scroll = 0.0f;
    float content_max = 0.0f;
    int segment = 0;
    int selected = 2;
    int list_selected = 0;
    int category_selected = 0;
    int dropdown_index = 0;
    std::vector<HitTarget> hits;
    int hover_nav = -1;
    int hover_list_row = -1;
    int hover_category = -1;
    int hover_segment = -1;
    int hover_toggle = -1;
    int hover_button = -1;
    int hover_icon = -1;
    int hover_card_action = -1;
    int hover_task_cancel = -1;
    bool task_cancelled = false;
    Rect task_cancel_rect{};
    std::vector<Rect> card_action_rects;
    std::wstring status = L"就绪";
    float scale = 1.0f;

    TextInputState& active_input() {
        if (dialog_input_active) return dialog_input_state;
        return field_active ? field_input_state : input_state;
    }
    const Rect& active_box() const {
        if (dialog_input_active) return dialog_input_box;
        return field_active ? field_input_box : input_box;
    }
};

float s(float logical, float scale) { return logical * scale; }

Palette blend_palette(const Palette& from, const Palette& to, float t) {
    Palette result;
    result.bg = lerp(from.bg, to.bg, t);
    result.surface = lerp(from.surface, to.surface, t);
    result.border = lerp(from.border, to.border, t);
    result.text = lerp(from.text, to.text, t);
    result.secondary = lerp(from.secondary, to.secondary, t);
    result.icon = lerp(from.icon, to.icon, t);
    result.accent = lerp(from.accent, to.accent, t);
    result.is_dark = t < 0.5f ? from.is_dark : to.is_dark;
    return result;
}

void text(Canvas& canvas, const Rect& area, std::wstring_view value,
          float size, Color color, FontWeight weight = FontWeight::normal,
          TextAlign align = TextAlign::leading,
          bool center_vertical = true) {
    TextStyle style;
    style.family = FontFamily::sans;
    style.size = size;
    style.color = color;
    style.weight = weight;
    style.align = align;
    style.center_vertical = center_vertical;
    canvas.draw_text(area, value, style);
}

void glyph(Canvas& canvas, const Rect& area, wchar_t code, float size,
           Color color) {
    TextStyle style;
    style.family = FontFamily::icon;
    style.size = size;
    style.color = color;
    style.align = TextAlign::center;
    style.center_vertical = true;
    const std::wstring value(1, code);
    canvas.draw_text(area, value, style);
}

void panel(Canvas& canvas, const Rect& area, const Theme& theme) {
    canvas.fill_rect(area, theme.palette.surface);
    canvas.stroke_rect(area, theme.palette.border);
}

void separator(Canvas& canvas, float x1, float y, float x2,
               const Theme& theme) {
    canvas.fill_rect(Rect{x1, y, x2, y + theme.px(1.0f)},
                     theme.palette.border);
}

Color jol_hover_color() { return Color::from_argb(0xFF1C1F25u); }

void jol_button(Canvas& canvas, const Rect& area, std::wstring_view label,
                const Theme& theme, bool primary, bool enabled, bool hover) {
    if (hover && enabled) canvas.fill_rect(area, jol_hover_color());
    canvas.stroke_rect(area, hover && enabled ? theme.palette.accent
                                              : theme.palette.border);
    text(canvas, area, label, theme.px(12.0f),
         !enabled ? theme.palette.secondary.with_alpha(0.5f)
                  : (primary ? theme.palette.accent : theme.palette.secondary),
         FontWeight::normal, TextAlign::center, true);
}

void jol_nav_row(Canvas& canvas, const Rect& area, std::wstring_view label,
                 wchar_t glyph_code, const Theme& theme, bool selected,
                 bool hover) {
    if (selected) {
        canvas.fill_rect(area, theme.palette.surface);
        canvas.fill_rect(
            Rect{area.left, area.top, area.left + theme.px(2.0f), area.bottom},
            theme.palette.accent);
    } else if (hover) {
        canvas.fill_rect(area, jol_hover_color());
    }
    glyph(canvas,
          Rect{area.left + theme.px(12.0f), area.top,
               area.left + theme.px(28.0f), area.bottom},
          glyph_code, theme.px(16.0f),
          selected ? theme.palette.accent : theme.palette.secondary);
    text(canvas,
         Rect{area.left + theme.px(40.0f), area.top, area.right,
              area.bottom},
         label, theme.px(14.0f),
         selected ? theme.palette.accent
                  : (hover ? theme.palette.text : theme.palette.secondary),
         FontWeight::normal, TextAlign::leading, true);
}

void jol_list_row(Canvas& canvas, const Rect& area, std::wstring_view title,
                  std::wstring_view subtitle, const Theme& theme,
                  bool selected, bool hover, bool enabled = true) {
    if (selected) {
        canvas.fill_rect(area, theme.palette.surface);
        canvas.fill_rect(
            Rect{area.left, area.top, area.left + theme.px(2.0f), area.bottom},
            theme.palette.accent);
    } else if (hover && enabled) {
        canvas.fill_rect(area, jol_hover_color());
    }
    const float left = area.left + theme.px(16.0f);
    const float right = area.right - theme.px(16.0f);
    const float title_top = subtitle.empty() ? area.top + theme.px(8.0f)
                                             : area.top + theme.px(5.0f);
    text(canvas,
         Rect{left, title_top, right, title_top + theme.px(18.0f)}, title,
         theme.px(13.0f),
         !enabled ? theme.palette.secondary.with_alpha(0.5f)
                  : (selected ? theme.palette.accent : theme.palette.text),
         FontWeight::normal, TextAlign::leading, false);
    if (!subtitle.empty()) {
        text(canvas,
             Rect{left, title_top + theme.px(18.0f), right,
                  title_top + theme.px(34.0f)},
             subtitle, theme.px(11.0f), theme.palette.secondary,
             FontWeight::normal, TextAlign::leading, false);
    }
}

void jol_category_row(Canvas& canvas, const Rect& area,
                      std::wstring_view label, const Theme& theme,
                      bool selected, bool hover) {
    if (selected) canvas.fill_rect(area, theme.palette.surface);
    else if (hover) canvas.fill_rect(area, jol_hover_color());
    canvas.fill_rect(
        Rect{area.left, area.top, area.left + theme.px(1.0f), area.bottom},
        selected ? theme.palette.accent : theme.palette.border);
    text(canvas,
         Rect{area.left + theme.px(16.0f), area.top, area.right,
              area.bottom},
         label, theme.px(13.0f),
         selected ? theme.palette.accent : theme.palette.secondary,
         FontWeight::normal, TextAlign::leading, true);
}

void jol_input(Canvas& canvas, const Rect& area, std::wstring_view value,
               const Theme& theme, bool focused,
               std::size_t caret = std::wstring::npos) {
    TextInputState state;
    state.text.assign(value);
    state.caret = caret == std::wstring::npos ? value.size() : caret;
    state.anchor = state.caret;
    state.focused = focused;
    paint_text_input(canvas, area, state, theme);
}

void segment(Canvas& canvas, const Rect& area, std::wstring_view value,
             const Theme& theme, bool selected, bool hover = false) {
    jol_button(canvas, area, value, theme, selected, true, hover);
}


void swatch(Canvas& canvas, const Rect& area, Color color) {
    canvas.fill_rect(area, color);
    canvas.stroke_rect(area, Color(1.0f, 1.0f, 1.0f, 0.2f));
}

std::wstring read_clipboard_text() {
    if (!OpenClipboard(nullptr)) return {};
    std::wstring result;
    HANDLE handle = GetClipboardData(CF_UNICODETEXT);
    if (handle != nullptr) {
        const auto* data = static_cast<const wchar_t*>(GlobalLock(handle));
        if (data != nullptr) result = data;
        GlobalUnlock(handle);
    }
    CloseClipboard();
    return result;
}

void write_clipboard_text(std::wstring_view text) {
    if (!OpenClipboard(nullptr)) return;
    EmptyClipboard();
    const std::size_t bytes = (text.size() + 1) * sizeof(wchar_t);
    HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (handle != nullptr) {
        auto* data = static_cast<wchar_t*>(GlobalLock(handle));
        if (data != nullptr) {
            std::copy(text.begin(), text.end(), data);
            data[text.size()] = 0;
            GlobalUnlock(handle);
            if (SetClipboardData(CF_UNICODETEXT, handle) != nullptr) {
                handle = nullptr;
            }
        }
        if (handle != nullptr) GlobalFree(handle);
    }
    CloseClipboard();
}

}  // namespace

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
    try {
        jo::draftui::win32::Window window;
        GalleryState state;
        state.input_state.text = L"示例文本";
        state.input_state.caret = state.input_state.text.size();
        state.input_state.anchor = state.input_state.caret;
        state.field_input_state.text = L"示例值";
        state.field_input_state.caret = state.field_input_state.text.size();
        state.field_input_state.anchor = state.field_input_state.caret;
        state.password_state.text = L"示例密码";
        state.disabled_state.text = L"禁用示例";
        state.error_state.text = L"错误示例";
        state.ime_state.text = L"示例";
        state.ime_state.composition = L"组合中";
        state.ime_state.composing = true;
        state.ime_state.focused = true;
        state.ime_state.caret = state.ime_state.text.size();
        state.dialog_input_state.text = L"示例值";
        state.dialog_input_state.caret = state.dialog_input_state.text.size();
        state.dialog_input_state.anchor = state.dialog_input_state.caret;

        const std::array<std::wstring, 6> items = {
            L"基元", L"按钮", L"选择", L"输入", L"容器", L"反馈"};
        const std::array<std::wstring, 3> menu_items = {L"文件", L"编辑",
                                                        L"帮助"};

        auto left_width = [&] { return s(state.side_width, state.scale); };
        auto menu_height = [&] { return s(30.0f, state.scale); };
        auto status_height = [&] { return s(26.0f, state.scale); };
        auto content_left = [&] {
            return left_width() + s(42.0f, state.scale); // JOL content origin
        };
        auto content_right = [&](float width) {
            return width - s(32.0f, state.scale);
        };

        auto row_rect = [&](int index) {
            const float top = menu_height() + s(54.0f, state.scale) +
                              s(37.0f, state.scale) *
                                  static_cast<float>(index);
            return Rect{s(1.0f, state.scale), top,
                        left_width() - s(1.0f, state.scale),
                        top + s(37.0f, state.scale)};
        };
        auto button_rect = [&](int index, float x, float y) {
            return Rect{x + s(80.0f * static_cast<float>(index), state.scale),
                        y, x + s(72.0f + 80.0f * static_cast<float>(index),
                                 state.scale),
                        y + s(34.0f, state.scale)};
        };
        auto toggle_rect = [&](float x, float y) {
            return Rect{x, y, x + s(72.0f, state.scale),
                        y + s(34.0f, state.scale)};
        };

        const auto sync_ime_position = [&] {
            if (state.canvas == nullptr) return;
            TextInputState& input = state.active_input();
            const Rect& box = state.active_box();
            TextStyle style;
            style.family = FontFamily::sans;
            style.size = s(12.0f, state.scale);
            const float prefix = state.canvas->measure_text(
                std::wstring_view(input.text).substr(0, input.caret), style);
            float caret_x = box.left + s(10.0f, state.scale) + prefix -
                            input.scroll;
            caret_x = std::clamp(caret_x, box.left, box.right);
            window.set_ime_position(
                Rect{caret_x, box.top, caret_x + 1.0f, box.bottom});
        };

        window.set_canvas_init_callback([](D2DCanvas& canvas) {
            canvas.add_font_file(L"flutter/assets/fonts/NotoSansSC-VF.ttf",
                                 FontFamily::sans);
            canvas.add_font_file(L"flutter/assets/fonts/NotoSerifSC-VF.ttf",
                                 FontFamily::serif);
            canvas.add_font_file(L"flutter/assets/fonts/NotoSansMono-VF.ttf",
                                 FontFamily::mono);
            canvas.add_font_file(L"flutter/assets/fonts/remixicon.ttf",
                                 FontFamily::icon);
        });
        window.set_dark_titlebar(true);

        window.set_mouse_move_callback([&](float x, float y) {
            state.mouse_x = x;
            state.mouse_y = y;
            if (state.autoscroll.active) {
                const int step = autoscroll_move(
                    state.autoscroll, y,
                    static_cast<int>(GetDpiForWindow(window.hwnd())),
                    static_cast<std::int64_t>(GetTickCount64()));
                if (step != 0) {
                    state.content_scroll = std::clamp(
                        state.content_scroll + static_cast<float>(step),
                        0.0f, state.content_max);
                    window.invalidate();
                }
            }
            if (state.context_open) {
                state.context_hover =
                    popup_at(state.context_layout, x, y);
                window.invalidate();
                return;
            }
            if (state.split_drag) {
                state.side_width = std::clamp(
                    state.split_start_width +
                        (x - state.split_start_x) / state.scale,
                    200.0f, 420.0f);
                window.invalidate();
                return;
            }
            if (state.canvas != nullptr) {
                TextInputState& active = state.active_input();
                const Rect& box = state.active_box();
                if (active.dragging &&
                    box.contains(x, y + state.content_scroll)) {
                    input_drag_to(*state.canvas, active, box, x, state.scale);
                    window.invalidate();
                    return;
                }
            }
            if (state.window_scroll_drag) {
                const float available = std::max(
                    0.0f, state.window_scroll_track.height() -
                              state.window_scroll_thumb.height());
                if (available > 0.0f) {
                    const float top = std::clamp(
                        y - state.window_scroll_drag_offset,
                        state.window_scroll_track.top,
                        state.window_scroll_track.top + available);
                    state.content_scroll =
                        (top - state.window_scroll_track.top) / available *
                        state.content_max;
                }
                window.invalidate();
                return;
            }
            state.window_scroll_hover =
                state.window_scroll_track.contains(x, y);
            state.hover_nav = -1;
            state.hover_list_row = -1;
            state.hover_category = -1;
            state.hover_segment = -1;
            state.hover_toggle = -1;
            state.hover_button = -1;
            state.hover_icon = -1;
            const float hit_y = y + state.content_scroll;
            for (const auto& hit : state.hits) {
                const bool inside =
                    hit.content ? hit.rect.contains(x, hit_y)
                                : hit.rect.contains(x, y);
                if (!inside) continue;
                switch (hit.kind) {
                    case HitKind::nav:
                        state.hover_nav = hit.index;
                        break;
                    case HitKind::list:
                        state.hover_list_row = hit.index;
                        break;
                    case HitKind::category:
                        state.hover_category = hit.index;
                        break;
                    case HitKind::segment:
                        state.hover_segment = hit.index;
                        break;
                    case HitKind::toggle:
                        state.hover_toggle = hit.index;
                        break;
                    case HitKind::button:
                        state.hover_button = hit.index;
                        break;
                    case HitKind::icon:
                        state.hover_icon = hit.index;
                        break;
                    case HitKind::card_action:
                        state.hover_card_action = hit.index;
                        break;
                    case HitKind::task_cancel:
                        state.hover_task_cancel = hit.index;
                        break;
                    default:
                        break;
                }
            }
            window.invalidate();
        });
        window.set_mouse_leave_callback([&] {
            state.hover_nav = -1;
            state.hover_list_row = -1;
            state.hover_category = -1;
            state.hover_segment = -1;
            state.hover_toggle = -1;
            state.hover_button = -1;
            state.hover_icon = -1;
            state.hover_card_action = -1;
            state.hover_task_cancel = -1;
            state.window_scroll_hover = false;
            window.invalidate();
        });
        window.set_key_callback([&](UINT key) {
            if (state.context_open && key == VK_ESCAPE) {
                state.context_open = false;
                state.context_hover = -1;
                window.invalidate();
                return true;
            }
            if (state.autoscroll.active && key == VK_ESCAPE) {
                autoscroll_stop(state.autoscroll);
                window.kill_timer(1);
                window.invalidate();
                return true;
            }
            TextInputState& input = state.active_input();
            if (!input.focused) return false;
            TextInputBehavior behavior;
            behavior.read_clipboard = [] { return read_clipboard_text(); };
            behavior.write_clipboard = [](std::wstring_view text) {
                write_clipboard_text(text);
            };
            Modifiers modifiers;
            modifiers.shift =
                (GetKeyState(VK_SHIFT) & 0x8000) != 0;
            modifiers.ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            if (!behavior.key(input, static_cast<int>(key), modifiers)) {
                return false;
            }
            sync_ime_position();
            window.invalidate();
            return true;
        });
        window.set_char_callback([&](wchar_t character) {
            TextInputState& input = state.active_input();
            if (!input.focused || character < 0x20) return false;
            TextInputBehavior behavior;
            if (!behavior.character(input, character)) return false;
            sync_ime_position();
            window.invalidate();
            return true;
        });
        window.set_wheel_callback([&](int delta) {
            if (state.context_open) {
                state.context_open = false;
                state.context_hover = -1;
            }
            if (state.autoscroll.active) {
                autoscroll_stop(state.autoscroll);
                window.kill_timer(1);
            }
            state.content_scroll = std::clamp(
                state.content_scroll - static_cast<float>(delta) / 3.0f,
                0.0f, state.content_max);
            window.invalidate();
        });
        window.set_mouse_button_callback([&](float x, float y, bool down) {
            if (down) {
                if (state.context_open) return;
                if (state.autoscroll.active) {
                    autoscroll_stop(state.autoscroll);
                    window.kill_timer(1);
                }
                if (state.dialog_open &&
                    state.dialog_input_box.contains(x, y)) {
                    state.dialog_input_active = true;
                    state.field_active = false;
                    state.dialog_input_state.focused = true;
                    state.input_state.focused = false;
                    state.field_input_state.focused = false;
                    if (state.canvas != nullptr) {
                        input_drag_begin(*state.canvas, state.dialog_input_state,
                                         state.dialog_input_box, x, state.scale);
                        sync_ime_position();
                    }
                    window.invalidate();
                    return;
                }
                if (state.field_input_box.contains(x,
                                                  y + state.content_scroll)) {
                    state.field_active = true;
                    state.field_input_state.focused = true;
                    state.input_state.focused = false;
                    if (state.canvas != nullptr) {
                        input_drag_begin(*state.canvas, state.field_input_state,
                                         state.field_input_box, x, state.scale);
                        sync_ime_position();
                    }
                    window.invalidate();
                    return;
                }
                if (state.input_box.contains(x, y + state.content_scroll)) {
                    state.field_active = false;
                    state.input_state.focused = true;
                    state.field_input_state.focused = false;
                    if (state.canvas != nullptr) {
                        input_drag_begin(*state.canvas, state.input_state,
                                         state.input_box, x, state.scale);
                        sync_ime_position();
                    }
                    window.invalidate();
                    return;
                }
                const float split_left = left_width();
                if (x >= split_left && x < split_left + s(10.0f, state.scale)) {
                    state.split_drag = true;
                    state.split_start_x = x;
                    state.split_start_width = state.side_width;
                    return;
                }
                if (state.window_scroll_track.contains(x, y)) {
                    if (state.window_scroll_thumb.contains(x, y)) {
                        state.window_scroll_drag = true;
                        state.window_scroll_drag_offset =
                            y - state.window_scroll_thumb.top;
                    } else {
                        const float available = std::max(
                            0.0f, state.window_scroll_track.height() -
                                      state.window_scroll_thumb.height());
                        if (available > 0.0f) {
                            state.content_scroll = std::clamp(
                                (y - state.window_scroll_track.top) /
                                    available,
                                0.0f, 1.0f) * state.content_max;
                        }
                    }
                    window.invalidate();
                    return;
                }
                return;
            }
            if (state.context_open) {
                const int index = popup_at(state.context_layout, x, y);
                if (index >= 0 &&
                    static_cast<std::size_t>(index) <
                        state.context_items.size() &&
                    !state.context_items[static_cast<std::size_t>(index)]
                         .divider) {
                    TextInputState& input =
                        state.field_active ? state.field_input_state
                                           : state.input_state;
                    TextInputBehavior behavior;
                    behavior.read_clipboard =
                        [] { return read_clipboard_text(); };
                    behavior.write_clipboard = [](std::wstring_view text) {
                        write_clipboard_text(text);
                    };
                    if (index == 0) {
                        behavior.execute(input, TextCommand::cut);
                    } else if (index == 1) {
                        behavior.execute(input, TextCommand::copy);
                    } else if (index == 2) {
                        behavior.execute(input, TextCommand::paste);
                    } else if (index == 4) {
                        behavior.execute(input, TextCommand::select_all);
                    }
                }
                state.context_open = false;
                state.context_hover = -1;
                window.invalidate();
                return;
            }
            if (state.split_drag) {
                state.split_drag = false;
                return;
            }
            if (state.window_scroll_drag) {
                state.window_scroll_drag = false;
                return;
            }
            if (state.input_state.dragging || state.field_input_state.dragging) {
                input_drag_end(state.input_state);
                input_drag_end(state.field_input_state);
                return;
            }
            if (state.dialog_open) {
                if (state.dialog_cancel.contains(x, y) ||
                    state.dialog_ok.contains(x, y)) {
                    state.dialog_open = false;
                    state.dialog_input_active = false;
                    state.dialog_input_state.focused = false;
                    state.status = L"对话框已关闭";
                }
                window.invalidate();
                return;
            }
            state.input_state.focused = false;
            state.field_input_state.focused = false;
            const float hit_y = y + state.content_scroll;
            for (const auto& hit : state.hits) {
                const bool inside =
                    hit.content ? hit.rect.contains(x, hit_y)
                                : hit.rect.contains(x, y);
                if (!inside) continue;
                switch (hit.kind) {
                    case HitKind::nav:
                        state.selected = hit.index;
                        state.status = L"选中：" + items[hit.index];
                        break;
                    case HitKind::button:
                        state.dialog_open = true;
                        state.dialog_input_active = true;
                        state.dialog_input_state.focused = true;
                        state.input_state.focused = false;
                        state.field_input_state.focused = false;
                        state.status = L"按钮：" +
                                       std::to_wstring(hit.index);
                        break;
                    case HitKind::icon:
                        state.status = L"图标：" +
                                       std::to_wstring(hit.index);
                        break;
                    case HitKind::card_action:
                        state.status = hit.index == 0 ? L"卡片：编辑"
                                                      : L"卡片：删除";
                        break;
                    case HitKind::task_cancel:
                        state.task_cancelled = true;
                        state.status = L"任务已取消";
                        break;
                    case HitKind::theme:
                        state.dark_theme = !state.dark_theme;
                        state.status = state.dark_theme ? L"深色主题"
                                                        : L"浅色主题";
                        state.theme_from = state.current_palette;
                        state.theme_to = state.dark_theme ? Palette::dark()
                                                          : Palette::light();
                        state.theme_progress = 0.0f;
                        state.theme_started = GetTickCount64();
                        state.theme_animating = true;
                        window.set_timer(2, 16, [&] {
                            if (!state.theme_animating) return;
                            const float elapsed = static_cast<float>(
                                GetTickCount64() - state.theme_started);
                            state.theme_progress =
                                std::clamp(elapsed / 180.0f, 0.0f, 1.0f);
                            const float eased =
                                1.0f - (1.0f - state.theme_progress) *
                                           (1.0f - state.theme_progress);
                            const Palette p = blend_palette(
                                state.theme_from, state.theme_to, eased);
                            window.set_titlebar_theme(
                                state.dark_theme, p.bg, p.border, p.text);
                            if (state.theme_progress >= 1.0f) {
                                state.theme_animating = false;
                                window.kill_timer(2);
                            }
                            window.invalidate();
                        });
                        break;
                    case HitKind::dropdown:
                        state.dropdown_index = (state.dropdown_index + 1) % 2;
                        state.status = state.dropdown_index == 0
                                           ? L"示例一"
                                           : L"示例二";
                        break;
                    case HitKind::segment:
                        state.segment = hit.index;
                        state.status = L"分段：" +
                                       std::to_wstring(hit.index);
                        break;
                    case HitKind::toggle:
                        state.toggle = !state.toggle;
                        state.status = state.toggle ? L"已启用" : L"已禁用";
                        break;
                    case HitKind::list:
                        state.list_selected = hit.index;
                        state.status = L"列表：" +
                                       std::to_wstring(hit.index);
                        break;
                    case HitKind::category:
                        state.category_selected = hit.index;
                        state.status = L"分类：" +
                                       std::to_wstring(hit.index);
                        break;
                }
                window.invalidate();
                return;
            }
            window.invalidate();
        });

        window.set_cursor_callback([&]() -> HCURSOR {
            if (state.autoscroll.active) {
                int resource = IDC_PAN_MIDDLE_VERTICAL;
            if (state.autoscroll.velocity > 0.0f) {
                resource = IDC_PAN_SOUTH;
            } else if (state.autoscroll.velocity < 0.0f) {
                resource = IDC_PAN_NORTH;
            }
            HCURSOR cursor = LoadCursorW(GetModuleHandleW(nullptr),
                                         MAKEINTRESOURCEW(resource));
                if (cursor == nullptr) {
                    cursor = LoadCursorW(nullptr,
                                         state.autoscroll.velocity == 0.0f
                                             ? IDC_SIZEALL
                                             : IDC_SIZENS);
                }
                return cursor;
            }
            POINT point{};
            GetCursorPos(&point);
            ScreenToClient(window.hwnd(), &point);
            const float px = static_cast<float>(point.x);
            const float py = static_cast<float>(point.y);
            if ((state.dialog_open &&
                 state.dialog_input_box.contains(px, py)) ||
                state.input_box.contains(px, py + state.content_scroll) ||
                state.field_input_box.contains(px, py + state.content_scroll)) {
                return LoadCursorW(nullptr, IDC_IBEAM);
            }
            if (px >= left_width() &&
                px < left_width() + s(10.0f, state.scale)) {
                return LoadCursorW(nullptr, IDC_SIZEWE);
            }
            return LoadCursorW(nullptr, IDC_ARROW);
        });
        window.set_middle_button_callback([&](float x, float y, bool down) {
            if (!down) {
                if (!autoscroll_up(state.autoscroll)) {
                    window.kill_timer(1);
                    state.status = L"就绪";
                }
                window.invalidate();
                return;
            }
            if (state.autoscroll.active) {
                autoscroll_stop(state.autoscroll);
                window.kill_timer(1);
                state.status = L"就绪";
                window.invalidate();
                return;
            }
            if (x < left_width() || state.content_max <= 0.0f) return;
            if (state.input_box.contains(x, y + state.content_scroll) ||
                state.field_input_box.contains(x,
                                              y + state.content_scroll)) {
                return;
            }
            autoscroll_begin(state.autoscroll, x, y,
                             static_cast<std::int64_t>(GetTickCount64()));
            state.status = L"自动滚动";
            state.mouse_x = x;
            state.mouse_y = y;
            window.set_timer(1, 16, [&] {
                const int step = autoscroll_advance(
                    state.autoscroll,
                    static_cast<std::int64_t>(GetTickCount64()));
                if (step != 0) {
                    state.content_scroll = std::clamp(
                        state.content_scroll + static_cast<float>(step),
                        0.0f, state.content_max);
                    window.invalidate();
                }
            });
            window.invalidate();
        });

        window.set_double_click_callback([&](float x, float y) {
            if (state.canvas == nullptr) return;
            const float split_left = left_width();
            if (x >= split_left &&
                x < split_left + s(10.0f, state.scale)) {
                state.side_width = 232.0f;
                window.invalidate();
                return;
            }
            const bool over_field =
                state.field_input_box.contains(x, y + state.content_scroll);
            if (!over_field &&
                !state.input_box.contains(x, y + state.content_scroll)) {
                return;
            }
            state.field_active = over_field;
            TextInputState& input = over_field ? state.field_input_state
                                               : state.input_state;
            const Rect& box = over_field ? state.field_input_box
                                         : state.input_box;
            TextInputBehavior behavior;
            behavior.double_click(*state.canvas, input, box, x, state.scale);
            window.invalidate();
        });
        window.set_ime_callback([&](std::wstring_view text, bool committed) {
            TextInputState& input = state.active_input();
            input_composition(input, text, committed);
            sync_ime_position();
            window.invalidate();
        });

        window.set_right_button_callback([&](float x, float y, bool down) {
            if (!down || state.canvas == nullptr) return;
            const bool over_dialog =
                state.dialog_open &&
                state.dialog_input_box.contains(x, y);
            const bool over_field =
                !over_dialog &&
                state.field_input_box.contains(x, y + state.content_scroll);
            const bool over_main =
                !over_dialog && !over_field &&
                state.input_box.contains(x, y + state.content_scroll);
            if (!over_dialog && !over_field && !over_main) return;
            state.dialog_input_active = over_dialog;
            state.field_active = over_field;
            TextInputState& input = state.active_input();
            input.focused = true;
            state.input_state.focused = over_main;
            state.field_input_state.focused = over_field;
            state.dialog_input_state.focused = over_dialog;
            const Rect& box = state.active_box();
            input_right_click(*state.canvas, input, box, x, state.scale);
            TextInputBehavior behavior;
            behavior.read_clipboard = [] { return read_clipboard_text(); };
            behavior.write_clipboard = [](std::wstring_view text) {
                write_clipboard_text(text);
            };
            const TextMenuState menu_state =
                behavior.menu_state(input, !read_clipboard_text().empty());
            state.context_items = {
                PopupItem{L"剪切", false, menu_state.cut},
                PopupItem{L"复制", false, menu_state.copy},
                PopupItem{L"粘贴", false, menu_state.paste},
                PopupItem{L"", true, true},
                PopupItem{L"全选", false, menu_state.select_all}};
            const Theme theme = Theme{}.with_scale(state.scale);
            state.context_layout = layout_popup(
                *state.canvas,
                Rect{x, y, x + state.scale, y + state.scale},
                state.context_items, theme, state.client_rect);
            state.context_open = true;
            state.context_hover = -1;
            window.invalidate();
        });

        window.set_paint_callback([&](Canvas& canvas, const Rect& client,
                                      float scale) {
            state.canvas = &canvas;
            state.scale = scale;
            state.client_rect = client;
            state.hits.clear();
            const auto add_hit = [&](const Rect& rect, HitKind kind, int index,
                                     bool content) {
                state.hits.push_back(HitTarget{rect, kind, index, content});
            };
            Palette palette = state.dark_theme ? Palette::dark()
                                              : Palette::light();
            if (state.theme_animating) {
                const float t = std::clamp(state.theme_progress, 0.0f, 1.0f);
                const float eased = 1.0f - (1.0f - t) * (1.0f - t);
                palette = blend_palette(state.theme_from, state.theme_to, eased);
            }
            state.current_palette = palette;
            Theme theme = Theme{}.with_scale(scale).with_palette(palette);
            canvas.clear(theme.palette.bg);

            // Top menu bar.
            canvas.fill_rect(Rect{0.0f, 0.0f, client.width(), menu_height()},
                             theme.palette.bg);
            separator(canvas, 0.0f, menu_height() - theme.px(1.0f),
                      client.width(), theme);
            float menu_x = s(8.0f, scale);
            for (const auto& menu : menu_items) {
                const Rect menu_rect{menu_x, 0.0f,
                                     menu_x + s(48.0f, scale),
                                     menu_height()};
                text(canvas, menu_rect, menu, s(13.0f, scale),
                     theme.palette.secondary, FontWeight::normal,
                     TextAlign::center, true);
                menu_x += s(48.0f, scale);
            }

            // Left rail.
            const float left_bottom =
                client.height() - status_height();
            canvas.fill_rect(Rect{0.0f, menu_height(), left_width(),
                                  left_bottom},
                             theme.palette.surface);
            separator(canvas, left_width() - theme.px(1.0f), menu_height(),
                      left_width(), theme);
            const Rect dropdown_rect{s(12.0f, scale),
                                     menu_height() + s(12.0f, scale),
                                     left_width() - s(12.0f, scale),
                                     menu_height() + s(42.0f, scale)};
            paint_dropdown(canvas, dropdown_rect,
                           state.dropdown_index == 0 ? L"示例一" : L"示例二",
                           theme);
            add_hit(dropdown_rect, HitKind::dropdown, 0, false);
            separator(canvas, 0.0f, menu_height() + s(54.0f, scale) -
                                      theme.px(1.0f),
                      left_width(), theme);

            const std::array<wchar_t, 6> nav_glyphs = {
                static_cast<wchar_t>(0xF097), static_cast<wchar_t>(0xF3A8),
                static_cast<wchar_t>(0xEC5A), static_cast<wchar_t>(0xF0EE),
                static_cast<wchar_t>(0xF0EE), static_cast<wchar_t>(0xF0EE)};
            for (int i = 0; i < 6; ++i) {
                jol_nav_row(canvas, row_rect(i), items[i], nav_glyphs[i],
                            theme, state.selected == i,
                            state.hover_nav == i);
                add_hit(row_rect(i), HitKind::nav, i, false);
            }

            // Splitter.
            const float split = left_width();
            canvas.fill_rect(Rect{split, menu_height(), split + s(10.0f, scale),
                                  left_bottom},
                             theme.palette.bg);
            canvas.fill_rect(Rect{split + s(5.0f, scale), menu_height(),
                                  split + s(6.0f, scale), left_bottom},
                             theme.palette.border);

            // Center content.
            const float x = content_left();
            const float right = content_right(client.width());
            const float content_top = menu_height();
            const float content_bottom = client.height() - status_height();
            canvas.push_clip(Rect{x, content_top, client.width(),
                                  content_bottom});
            canvas.push_translate(0.0f, -state.content_scroll);
            float y = menu_height() + s(24.0f, scale);
            text(canvas, Rect{x, y, right, y + s(22.0f, scale)},
                 L"JO-DraftUI 控件展厅", s(18.0f, scale),
                 theme.palette.text, FontWeight::bold);
            const Rect theme_button{right - s(88.0f, scale),
                                    y + s(2.0f, scale), right,
                                    y + s(30.0f, scale)};
            jol_button(canvas, theme_button,
                       state.dark_theme ? L"深色" : L"浅色", theme, false,
                       true, false);
            add_hit(theme_button, HitKind::theme, 0, true);
            y += s(30.0f, scale);

            // Button section.
            text(canvas, Rect{x, y, right, y + s(20.0f, scale)}, L"按钮",
                 s(12.0f, scale), theme.palette.secondary);
            y += s(24.0f, scale);
            panel(canvas, Rect{x, y, right, y + s(120.0f, scale)}, theme);
            text(canvas, Rect{x + s(16.0f, scale), y + s(8.0f, scale),
                              right - s(16.0f, scale), y + s(26.0f, scale)},
                 L"文字按钮", s(14.0f, scale), theme.palette.text);
            text(canvas, Rect{x + s(16.0f, scale), y + s(26.0f, scale),
                              right - s(16.0f, scale), y + s(42.0f, scale)},
                 L"细线、直角", s(12.0f, scale), theme.palette.secondary);
            const float buttons_left =
                right - s(16.0f + 232.0f, scale);
            const std::array<std::wstring, 3> labels = {L"保存", L"取消",
                                                        L"删除"};
            for (int i = 0; i < 3; ++i) {
                const Rect button =
                    button_rect(i, buttons_left, y + s(12.0f, scale));
                jol_button(canvas, button, labels[i], theme, false, true,
                           state.hover_button == i);
                add_hit(button, HitKind::button, i, true);
            }
            text(canvas, Rect{x + s(16.0f, scale), y + s(66.0f, scale),
                              right - s(16.0f, scale), y + s(84.0f, scale)},
                 L"状态按钮", s(14.0f, scale), theme.palette.text);
            ButtonStyle primary_style;
            primary_style.primary = true;
            ButtonStyle danger_style;
            danger_style.danger = true;
            Interaction primary_state;
            Interaction danger_state;
            Interaction disabled_state;
            disabled_state.enabled = false;
            paint_button(canvas,
                         button_rect(0, buttons_left, y + s(78.0f, scale)),
                         L"主要", theme, primary_state, primary_style);
            paint_button(canvas,
                         button_rect(1, buttons_left, y + s(78.0f, scale)),
                         L"危险", theme, danger_state, danger_style);
            paint_button(canvas,
                         button_rect(2, buttons_left, y + s(78.0f, scale)),
                         L"禁用", theme, disabled_state);
            y += s(132.0f, scale);

            // Icon actions.
            text(canvas, Rect{x, y, right, y + s(20.0f, scale)},
                 L"图标动作", s(12.0f, scale), theme.palette.secondary);
            y += s(24.0f, scale);
            panel(canvas, Rect{x, y, right, y + s(60.0f, scale)}, theme);
            text(canvas,
                 Rect{x + s(16.0f, scale), y + s(8.0f, scale),
                      right - s(16.0f, scale), y + s(26.0f, scale)},
                 L"图标按钮", s(14.0f, scale), theme.palette.text);
            const float icon_left = right - s(16.0f + 118.0f, scale);
            const std::array<wchar_t, 3> icon_glyphs = {
                static_cast<wchar_t>(0xF097), static_cast<wchar_t>(0xEC5A),
                static_cast<wchar_t>(0xF0EE)};
            for (int i = 0; i < 3; ++i) {
                const Rect icon_rect{icon_left + s(42.0f * i, scale),
                                     y + s(13.0f, scale),
                                     icon_left + s(42.0f * i + 34.0f, scale),
                                     y + s(47.0f, scale)};
                Interaction icon_state;
                icon_state.hover = state.hover_icon == i;
                const std::wstring icon_text(1, icon_glyphs[i]);
                paint_icon_action(canvas, icon_rect, icon_text, theme,
                                  icon_state, i == 2);
                add_hit(icon_rect, HitKind::icon, i, true);
            }
            if (state.hover_icon >= 0) {
                const float tooltip_x =
                    icon_left + s(42.0f * state.hover_icon, scale);
                paint_tooltip(
                    canvas,
                    Rect{tooltip_x, y + s(13.0f, scale),
                         tooltip_x + s(34.0f, scale), y + s(47.0f, scale)},
                    L"图标动作", theme, true,
                    Rect{x, content_top, right, content_bottom});
            }
            y += s(72.0f, scale);

            // Select section.
            text(canvas, Rect{x, y, right, y + s(20.0f, scale)}, L"选择",
                 s(12.0f, scale), theme.palette.secondary);
            y += s(24.0f, scale);
            panel(canvas, Rect{x, y, right, y + s(102.0f, scale)}, theme);
            text(canvas, Rect{x + s(16.0f, scale), y + s(9.0f, scale),
                              x + s(140.0f, scale), y + s(27.0f, scale)},
                 L"分段", s(14.0f, scale), theme.palette.text);
            const float seg_y = y + s(6.0f, scale);
            const float segments_left = right - s(16.0f + 220.0f, scale);
            const std::array<std::wstring, 3> seg_labels = {L"全部", L"地形",
                                                            L"道具"};
            for (int i = 0; i < 3; ++i) {
                const Rect segment_rect{
                    segments_left +
                        s(76.0f * static_cast<float>(i), scale),
                    seg_y,
                    segments_left +
                        s(68.0f + 76.0f * static_cast<float>(i), scale),
                    seg_y + s(30.0f, scale)};
                segment(canvas, segment_rect, seg_labels[i], theme,
                        state.segment == i, state.hover_segment == i);
                add_hit(segment_rect, HitKind::segment, i, true);
            }
            separator(canvas, x, y + s(42.0f, scale), right, theme);
            text(canvas, Rect{x + s(16.0f, scale), y + s(51.0f, scale),
                              x + s(200.0f, scale), y + s(71.0f, scale)},
                 L"开关", s(14.0f, scale), theme.palette.text);
            text(canvas, Rect{x + s(16.0f, scale), y + s(71.0f, scale),
                              x + s(320.0f, scale), y + s(87.0f, scale)},
                 L"文字样式，不用滑块", s(12.0f, scale),
                 theme.palette.secondary);
            const Rect toggle =
                toggle_rect(right - s(88.0f, scale), y + s(57.0f, scale));
            segment(canvas, toggle, state.toggle ? L"启用" : L"禁用", theme,
                    state.toggle, state.hover_toggle == 0);
            add_hit(toggle, HitKind::toggle, 0, true);
            y += s(114.0f, scale);

            // List rows section.
            text(canvas, Rect{x, y, right, y + s(20.0f, scale)},
                 L"列表行", s(12.0f, scale), theme.palette.secondary);
            y += s(24.0f, scale);
            panel(canvas, Rect{x, y, right, y + s(116.0f, scale)}, theme);
            const std::array<std::wstring, 3> list_labels = {
                L"示例项一", L"示例项二", L"不可用"};
            float list_y = y;
            for (int i = 0; i < 3; ++i) {
                const float row_height = i < 2 ? s(42.0f, scale)
                                               : s(30.0f, scale);
                const Rect list_rect{x, list_y, right, list_y + row_height};
                jol_list_row(
                    canvas, list_rect, list_labels[i],
                    i == 0 ? L"32×24" : (i == 1 ? L"32×12" : L""), theme,
                    state.list_selected == i,
                    state.hover_list_row == i,
                    i != 2);
                add_hit(list_rect, HitKind::list, i, true);
                if (i > 0) separator(canvas, x, list_y, right, theme);
                list_y += row_height;
            }
            y += s(128.0f, scale);

            // Palette section, if it fits.
            text(canvas, Rect{x, y, right, y + s(20.0f, scale)}, L"色板",
                 s(12.0f, scale), theme.palette.secondary);
            y += s(24.0f, scale);
            panel(canvas, Rect{x, y, right, y + s(158.0f, scale)}, theme);
            const auto palette_row = [&](float top, std::wstring_view label,
                                         const Palette& p) {
                text(canvas,
                     Rect{x + s(16.0f, scale), top, x + s(100.0f, scale),
                          top + s(20.0f, scale)},
                     label, s(14.0f, scale), theme.palette.text);
                float sx = x + s(120.0f, scale);
                const std::array<Color, 6> colors = {
                    p.bg, p.surface, p.border, p.text, p.secondary, p.accent};
                for (Color color : colors) {
                    swatch(canvas,
                           Rect{sx, top + s(4.0f, scale),
                                sx + s(28.0f, scale),
                                top + s(24.0f, scale)},
                           color);
                    sx += s(32.0f, scale);
                }
            };
            palette_row(y + s(12.0f, scale), L"暗 · 机器",
                        jo::draftui::Palette::dark());
            palette_row(y + s(54.0f, scale), L"明 · 纸",
                        jo::draftui::Palette::light());
            palette_row(y + s(96.0f, scale), L"当前", theme.palette);
            y += s(170.0f, scale);

            // Input sample.
            text(canvas, Rect{x, y, right, y + s(20.0f, scale)}, L"输入框",
                 s(12.0f, scale), theme.palette.secondary);
            y += s(24.0f, scale);
            panel(canvas, Rect{x, y, right, y + s(60.0f, scale)}, theme);
            text(canvas,
                 Rect{x + s(16.0f, scale), y, x + s(140.0f, scale),
                      y + s(60.0f, scale)},
                 L"示例字段", s(14.0f, scale), theme.palette.text);
            state.input_box = Rect{right - s(236.0f, scale),
                                   y + s(15.0f, scale),
                                   right - s(16.0f, scale),
                                   y + s(45.0f, scale)};
            paint_text_input(canvas, state.input_box, state.input_state, theme);
            y += s(72.0f, scale);

            // Input states.
            text(canvas, Rect{x, y, right, y + s(20.0f, scale)},
                 L"输入状态", s(12.0f, scale), theme.palette.secondary);
            y += s(24.0f, scale);
            panel(canvas, Rect{x, y, right, y + s(200.0f, scale)}, theme);
            const std::array<std::wstring, 4> input_labels = {
                L"密码", L"禁用", L"错误", L"IME"};
            const std::array<TextInputState*, 4> input_states = {
                &state.password_state, &state.disabled_state,
                &state.error_state, &state.ime_state};
            for (int i = 0; i < 4; ++i) {
                const float row_top = y + s(50.0f * i, scale);
                text(canvas,
                     Rect{x + s(16.0f, scale), row_top,
                          right - s(236.0f, scale),
                          row_top + s(50.0f, scale)},
                     input_labels[i], s(14.0f, scale), theme.palette.text);
                TextInputStyle input_style;
                input_style.password = i == 0;
                input_style.enabled = i != 1;
                input_style.error = i == 2;
                paint_text_input(
                    canvas,
                    Rect{right - s(236.0f, scale), row_top + s(10.0f, scale),
                         right - s(16.0f, scale), row_top + s(40.0f, scale)},
                    *input_states[i], theme, input_style);
                if (i < 3) {
                    separator(canvas, x, row_top + s(50.0f, scale), right,
                              theme);
                }
            }
            y += s(212.0f, scale);

            // Card row.
            text(canvas, Rect{x, y, right, y + s(20.0f, scale)},
                 L"卡片行", s(12.0f, scale), theme.palette.secondary);
            y += s(24.0f, scale);
            panel(canvas, Rect{x, y, right, y + s(58.0f, scale)}, theme);
            state.card_action_rects = paint_card_row(
                canvas, Rect{x, y, right, y + s(58.0f, scale)},
                L"示例项目", L"副标题示例",
                std::vector<std::wstring>{L"编辑", L"删除"}, theme);
            for (std::size_t i = 0; i < state.card_action_rects.size(); ++i) {
                add_hit(state.card_action_rects[i], HitKind::card_action,
                        static_cast<int>(i), true);
            }
            y += s(70.0f, scale);

            // Dialog scenario (static).
            text(canvas, Rect{x, y, right, y + s(20.0f, scale)},
                 L"对话框场景", s(12.0f, scale), theme.palette.secondary);
            y += s(24.0f, scale);
            panel(canvas, Rect{x, y, right, y + s(196.0f, scale)}, theme);
            const Rect dialog_demo{x + s(16.0f, scale), y + s(12.0f, scale),
                                   right - s(16.0f, scale),
                                   y + s(184.0f, scale)};
            panel(canvas, dialog_demo, theme);
            text(canvas,
                 Rect{dialog_demo.left + s(24.0f, scale),
                      dialog_demo.top + s(16.0f, scale),
                      dialog_demo.right - s(24.0f, scale),
                      dialog_demo.top + s(44.0f, scale)},
                 L"编辑名称", s(17.0f, scale), theme.palette.text,
                 FontWeight::semi_bold);
            text(canvas,
                 Rect{dialog_demo.left + s(24.0f, scale),
                      dialog_demo.top + s(48.0f, scale),
                      dialog_demo.right - s(24.0f, scale),
                      dialog_demo.top + s(72.0f, scale)},
                 L"对话框内容区域", s(12.0f, scale),
                 theme.palette.secondary);
            const Rect dialog_demo_input{
                dialog_demo.left + s(24.0f, scale),
                dialog_demo.top + s(82.0f, scale),
                dialog_demo.right - s(24.0f, scale),
                dialog_demo.top + s(112.0f, scale)};
            paint_text_input(canvas, dialog_demo_input,
                             state.dialog_input_state, theme);
            jol_button(canvas,
                       Rect{dialog_demo.right - s(224.0f, scale),
                            dialog_demo.bottom - s(54.0f, scale),
                            dialog_demo.right - s(124.0f, scale),
                            dialog_demo.bottom - s(22.0f, scale)},
                       L"取消", theme, false, true, false);
            jol_button(canvas,
                       Rect{dialog_demo.right - s(124.0f, scale),
                            dialog_demo.bottom - s(54.0f, scale),
                            dialog_demo.right - s(24.0f, scale),
                            dialog_demo.bottom - s(22.0f, scale)},
                       L"确定", theme, true, true, false);
            y += s(208.0f, scale);

            // Task progress.
            text(canvas, Rect{x, y, right, y + s(20.0f, scale)},
                 L"任务进度", s(12.0f, scale), theme.palette.secondary);
            y += s(24.0f, scale);
            panel(canvas, Rect{x, y, right, y + s(132.0f, scale)}, theme);
            const std::array<std::wstring, 3> task_labels = {
                L"下载资源包  45%", L"校验文件  100%", L"安装失败  60%"};
            const std::array<float, 3> task_ratios = {0.45f, 1.0f, 0.60f};
            for (int i = 0; i < 3; ++i) {
                const float row_top = y + s(44.0f * i, scale);
                const bool cancelled = i == 0 && state.task_cancelled;
                std::wstring task_label = task_labels[i];
                if (cancelled) task_label = L"下载资源包  已取消";
                text(canvas,
                     Rect{x + s(16.0f, scale), row_top,
                          right - s(16.0f, scale), row_top + s(20.0f, scale)},
                     task_label, s(12.0f, scale),
                     i == 2 ? Color(0.77f, 0.17f, 0.11f)
                            : theme.palette.text);
                const bool show_cancel = i == 0 && !cancelled;
                const Rect bar{x + s(16.0f, scale),
                               row_top + s(22.0f, scale),
                               right - s(show_cancel ? 96.0f : 16.0f,
                                         scale),
                               row_top + s(28.0f, scale)};
                if (i == 2) {
                    canvas.fill_rect(bar, theme.palette.border);
                    canvas.fill_rect(
                        Rect{bar.left, bar.top,
                             bar.left + bar.width() * task_ratios[i],
                             bar.bottom},
                        Color(0.77f, 0.17f, 0.11f));
                } else {
                    paint_progress_bar(canvas, bar,
                                       cancelled ? 0.0f : task_ratios[i],
                                       theme);
                }
                if (show_cancel) {
                    state.task_cancel_rect =
                        Rect{right - s(88.0f, scale), row_top + s(2.0f, scale),
                             right - s(16.0f, scale),
                             row_top + s(28.0f, scale)};
                    jol_button(canvas, state.task_cancel_rect, L"取消", theme,
                               false, true, state.hover_task_cancel == 0);
                    add_hit(state.task_cancel_rect, HitKind::task_cancel, 0,
                            true);
                }
                if (i < 2) {
                    separator(canvas, x, row_top + s(44.0f, scale), right,
                              theme);
                }
            }
            y += s(144.0f, scale);

            // Input context menu (static presentation).
            text(canvas, Rect{x, y, right, y + s(20.0f, scale)},
                 L"右键菜单示例", s(12.0f, scale),
                 theme.palette.secondary);
            y += s(24.0f, scale);
            panel(canvas, Rect{x, y, right, y + s(154.0f, scale)}, theme);
            const std::vector<PopupItem> static_menu_items = {
                PopupItem{L"剪切", false, true},
                PopupItem{L"复制", false, true},
                PopupItem{L"粘贴", false, true},
                PopupItem{L"", true, true},
                PopupItem{L"全选", false, true}};
            const PopupLayout static_menu = layout_popup(
                canvas,
                Rect{x + s(16.0f, scale), y + s(12.0f, scale),
                     x + s(17.0f, scale), y + s(13.0f, scale)},
                static_menu_items, theme);
            paint_popup(canvas, static_menu, static_menu_items, theme, 0);
            y += s(166.0f, scale);

            // Category navigation sample.
            text(canvas, Rect{x, y, right, y + s(20.0f, scale)},
                 L"分类行", s(12.0f, scale), theme.palette.secondary);
            y += s(24.0f, scale);
            const std::array<std::wstring, 6> categories = {
                L"分类一", L"分类二", L"分类三", L"分类四", L"分类五", L"分类六"};
            panel(canvas, Rect{x, y, right, y + s(204.0f, scale)}, theme);
            for (int i = 0; i < 6; ++i) {
                const Rect row{x, y + s(34.0f * static_cast<float>(i), scale),
                               right,
                               y + s(34.0f * static_cast<float>(i + 1), scale)};
                jol_category_row(canvas, row, categories[i], theme,
                                 state.category_selected == i,
                                 state.hover_category == i);
                add_hit(row, HitKind::category, i, true);
            }
            y += s(216.0f, scale);

            // Field sample.
            text(canvas, Rect{x, y, right, y + s(20.0f, scale)},
                 L"字段行", s(12.0f, scale), theme.palette.secondary);
            y += s(24.0f, scale);
            panel(canvas, Rect{x, y, right, y + s(60.0f, scale)}, theme);
            text(canvas,
                 Rect{x + s(16.0f, scale), y + s(10.0f, scale),
                      right - s(236.0f, scale), y + s(30.0f, scale)},
                 L"示例字段", s(14.0f, scale), theme.palette.text);
            text(canvas,
                 Rect{x + s(16.0f, scale), y + s(30.0f, scale),
                      right - s(236.0f, scale), y + s(50.0f, scale)},
                 L"字段说明", s(12.0f, scale),
                 theme.palette.secondary);
            state.field_input_box =
                Rect{right - s(236.0f, scale), y + s(15.0f, scale),
                     right - s(16.0f, scale), y + s(45.0f, scale)};
            paint_text_input(canvas, state.field_input_box,
                             state.field_input_state, theme);

            state.content_max = std::max(
                0.0f, y + s(120.0f, scale) - content_bottom);
            canvas.pop_transform();
            canvas.pop_clip();

            if (state.content_max > 0.0f) {
                state.window_scroll_track =
                    Rect{client.width() - s(14.0f, scale),
                         content_top + s(48.0f, scale),
                         client.width() - s(4.0f, scale),
                         client.height()};
                const float track_height = state.window_scroll_track.height();
                float thumb_height =
                    std::max(s(24.0f, scale),
                             track_height * track_height /
                                 std::max(1.0f,
                                          track_height + state.content_max));
                thumb_height = std::min(thumb_height, track_height);
                const float available =
                    std::max(0.0f, track_height - thumb_height);
                const float progress =
                    state.content_max > 0.0f
                        ? std::clamp(state.content_scroll / state.content_max,
                                     0.0f, 1.0f)
                        : 0.0f;
                state.window_scroll_thumb =
                    Rect{state.window_scroll_track.left,
                         state.window_scroll_track.top + available * progress,
                         state.window_scroll_track.right,
                         state.window_scroll_track.top + available * progress +
                             thumb_height};
                canvas.fill_rect(state.window_scroll_track,
                                 theme.palette.surface);
                canvas.stroke_rect(state.window_scroll_track,
                                  theme.palette.border);
                canvas.fill_rect(
                    state.window_scroll_thumb,
                    state.window_scroll_drag
                        ? theme.palette.accent
                        : (state.window_scroll_hover ? theme.palette.text
                                                     : theme.palette.secondary));
            } else {
                state.window_scroll_track = {};
                state.window_scroll_thumb = {};
            }

            // Status bar.
            const float status_top = client.height() - status_height();
            canvas.fill_rect(Rect{0.0f, status_top, client.width(),
                                  client.height()},
                             theme.palette.bg);
            separator(canvas, 0.0f, status_top, client.width(), theme);
            text(canvas,
                 Rect{s(12.0f, scale), status_top,
                      client.width() - s(100.0f, scale), client.height()},
                 state.status, s(12.0f, scale), theme.palette.secondary);
            text(canvas,
                 Rect{client.width() - s(92.0f, scale), status_top,
                      client.width() - s(12.0f, scale), client.height()},
                 L"120 项", s(12.0f, scale), theme.palette.secondary,
                 FontWeight::normal, TextAlign::trailing);

            if (state.dialog_open) {
                canvas.fill_rect(client, Color(0.0f, 0.0f, 0.0f, 0.62f));
                const float dialog_width = s(460.0f, scale);
                const float dialog_height = s(330.0f, scale);
                const float dialog_left =
                    (client.width() - dialog_width) / 2.0f;
                const float dialog_top =
                    (client.height() - dialog_height) / 2.0f;
                const Rect dialog{dialog_left, dialog_top,
                                  dialog_left + dialog_width,
                                  dialog_top + dialog_height};
                panel(canvas, dialog, theme);
                text(canvas,
                     Rect{dialog_left + s(24.0f, scale),
                          dialog_top + s(20.0f, scale),
                          dialog_left + dialog_width - s(24.0f, scale),
                          dialog_top + s(48.0f, scale)},
                     L"编辑名称", s(17.0f, scale), theme.palette.text,
                     FontWeight::semi_bold);
                state.dialog_input_box =
                    Rect{dialog_left + s(24.0f, scale),
                         dialog_top + s(60.0f, scale),
                         dialog_left + dialog_width - s(24.0f, scale),
                         dialog_top + s(94.0f, scale)};
                paint_text_input(canvas, state.dialog_input_box,
                                 state.dialog_input_state, theme);
                state.dialog_cancel =
                    Rect{dialog_left + dialog_width - s(224.0f, scale),
                         dialog_top + dialog_height - s(54.0f, scale),
                         dialog_left + dialog_width - s(124.0f, scale),
                         dialog_top + dialog_height - s(22.0f, scale)};
                state.dialog_ok =
                    Rect{dialog_left + dialog_width - s(124.0f, scale),
                         dialog_top + dialog_height - s(54.0f, scale),
                         dialog_left + dialog_width - s(24.0f, scale),
                         dialog_top + dialog_height - s(22.0f, scale)};
                jol_button(canvas, state.dialog_cancel, L"取消", theme, false,
                           true, false);
                jol_button(canvas, state.dialog_ok, L"确定", theme, true,
                           true, false);
            }
            if (state.context_open) {
                paint_popup(canvas, state.context_layout, state.context_items,
                            theme, state.context_hover);
            }
        });

        window.set_dark_titlebar(true);
        if (!window.create(L"JO-DraftUI", 960, 1040)) return 1;
        window.set_titlebar_theme(true, Palette::dark().bg,
                                  Palette::dark().border,
                                  Palette::dark().text);
        return window.run();
    } catch (...) {
        return 1;
    }
}
