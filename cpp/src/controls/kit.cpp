#include "jo_draftui/controls/kit.hpp"

#include <algorithm>
#include <cmath>
#include <cwctype>

namespace jo::draftui {
namespace {

void draw_text(Canvas& canvas, const Rect& rect, std::wstring_view value,
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
    canvas.draw_text(rect, value, style);
}

void draw_icon(Canvas& canvas, const Rect& rect, wchar_t glyph, float size,
               Color color) {
    TextStyle style;
    style.family = FontFamily::icon;
    style.size = size;
    style.color = color;
    style.align = TextAlign::center;
    style.center_vertical = true;
    const std::wstring value(1, glyph);
    canvas.draw_text(rect, value, style);
}

Color hover_color(const Theme& theme) {
    return theme.palette.text.with_alpha(0.09f);
}

float text_width(Canvas& canvas, std::wstring_view text, float size,
                 FontWeight weight = FontWeight::normal) {
    TextStyle style;
    style.family = FontFamily::sans;
    style.size = size;
    style.weight = weight;
    return canvas.measure_text(text, style);
}

std::wstring display_text(const TextInputState& state,
                          const TextInputStyle& style) {
    if (!style.password) return state.text;
    return std::wstring(state.text.size(), L'\x2022');
}

void clamp_state(TextInputState& state) {
    if (state.text.empty()) {
        state.caret = 0;
        state.anchor = 0;
        return;
    }
    state.caret = std::min(state.caret, state.text.size());
    state.anchor = std::min(state.anchor, state.text.size());
}

void erase_selection(TextInputState& state) {
    if (state.caret == state.anchor) return;
    const std::size_t first = std::min(state.caret, state.anchor);
    const std::size_t last = std::max(state.caret, state.anchor);
    state.text.erase(first, last - first);
    state.caret = first;
    state.anchor = first;
}

bool word_char(wchar_t character) {
    return std::iswalnum(character) || character == L'_';
}

std::size_t previous_word(const std::wstring& text, std::size_t caret) {
    std::size_t index = std::min(caret, text.size());
    while (index > 0 && !word_char(text[index - 1])) --index;
    while (index > 0 && word_char(text[index - 1])) --index;
    return index;
}

std::size_t next_word(const std::wstring& text, std::size_t caret) {
    std::size_t index = std::min(caret, text.size());
    while (index < text.size() && word_char(text[index])) ++index;
    while (index < text.size() && !word_char(text[index])) ++index;
    return index;
}

std::size_t caret_at(Canvas& canvas, std::wstring_view text, float x,
                     float size) {
    float best = 0.0f;
    std::size_t best_index = 0;
    for (std::size_t i = 0; i <= text.size(); ++i) {
        const float width = text_width(canvas, text.substr(0, i), size);
        const float distance = std::abs(width - x);
        if (i == 0 || distance < best) {
            best = distance;
            best_index = i;
        }
    }
    return best_index;
}

}  // namespace

// ---------------------------------------------------------------------------
// Text input
// ---------------------------------------------------------------------------

void paint_text_input(Canvas& canvas, const Rect& rect,
                      const TextInputState& state, const Theme& theme,
                      const TextInputStyle& style) {
    const Color danger = Color(0.77f, 0.17f, 0.11f);
    canvas.fill_rect(rect, style.enabled ? theme.palette.bg
                                         : theme.palette.surface);
    canvas.stroke_rect(rect,
                       !style.enabled
                           ? theme.palette.border
                           : (style.error ? danger
                                          : (state.focused
                                                 ? theme.palette.accent
                                                 : theme.palette.border)));

    const Rect inner{rect.left + theme.px(10.0f),
                     rect.top + theme.px(3.0f),
                     rect.right - theme.px(8.0f),
                     rect.bottom - theme.px(3.0f)};
    if (inner.width() <= 0.0f || inner.height() <= 0.0f) return;

    const std::wstring visible = display_text(state, style);
    const float font_size = theme.px(12.0f);
    const float composition_width =
        state.composing ? text_width(canvas, state.composition, font_size)
                        : 0.0f;
    const float caret_x =
        text_width(canvas, std::wstring_view(visible).substr(0, state.caret),
                   font_size) +
        composition_width;
    const float anchor_x =
        text_width(canvas, std::wstring_view(visible).substr(0, state.anchor),
                   font_size);
    const float max_scroll = std::max(
        0.0f,
        text_width(canvas, visible, font_size) + composition_width -
            inner.width());
    float scroll = state.scroll;
    if (state.focused) {
        if (caret_x - scroll > inner.width() - 2.0f) {
            scroll = caret_x - inner.width() + 2.0f;
        }
        if (caret_x - scroll < 0.0f) scroll = caret_x;
    } else {
        scroll = 0.0f;
    }
    scroll = std::clamp(scroll, 0.0f, max_scroll);

    canvas.push_clip(inner);
    if (state.focused && state.caret != state.anchor) {
        const float left = inner.left + std::min(caret_x, anchor_x) - scroll;
        const float right = inner.left + std::max(caret_x, anchor_x) - scroll;
        canvas.fill_rect(Rect{left, inner.top, right, inner.bottom},
                         theme.palette.accent.with_alpha(0.32f));
    }
    draw_text(canvas,
              Rect{inner.left - scroll, rect.top, inner.right - scroll,
                   rect.bottom},
              visible, font_size,
              style.enabled
                  ? (style.error ? danger : theme.palette.text)
                  : theme.palette.secondary.with_alpha(0.5f));
    if (state.composing && !state.composition.empty()) {
        const float composition_left =
            inner.left + text_width(canvas, visible, font_size) - scroll;
        draw_text(canvas,
                  Rect{composition_left, rect.top, inner.right - scroll,
                       rect.bottom},
                  state.composition, font_size, theme.palette.accent);
        canvas.fill_rect(
            Rect{composition_left, rect.bottom - theme.px(4.0f),
                 composition_left + composition_width,
                 rect.bottom - theme.px(2.0f)},
            theme.palette.accent);
    }
    if (state.focused && style.enabled) {
        const float caret = inner.left + caret_x - scroll;
        canvas.fill_rect(
            Rect{caret, rect.top + theme.px(6.0f), caret + theme.px(1.0f),
                 rect.bottom - theme.px(6.0f)},
            theme.palette.text);
    }
    canvas.pop_clip();
}

void input_set_caret(TextInputState& state, std::size_t caret, bool extend) {
    state.caret = std::min(caret, state.text.size());
    if (!extend) state.anchor = state.caret;
}

void input_select_all(TextInputState& state) {
    state.anchor = 0;
    state.caret = state.text.size();
}

std::wstring input_selected_text(const TextInputState& state) {
    if (state.caret == state.anchor) return {};
    const std::size_t first = std::min(state.caret, state.anchor);
    const std::size_t last = std::max(state.caret, state.anchor);
    return state.text.substr(first, last - first);
}

void input_select_word_at(TextInputState& state, std::size_t index) {
    if (state.text.empty()) {
        state.caret = state.anchor = 0;
        return;
    }
    index = std::min(index, state.text.size() - 1);
    const bool word = word_char(state.text[index]);
    std::size_t first = index;
    std::size_t last = index + 1;
    while (first > 0 && word_char(state.text[first - 1]) == word) --first;
    while (last < state.text.size() &&
           word_char(state.text[last]) == word) {
        ++last;
    }
    state.anchor = first;
    state.caret = last;
}

bool input_cut(TextInputState& state, std::wstring* clipboard) {
    if (state.caret == state.anchor) return false;
    const std::wstring selected = input_selected_text(state);
    if (clipboard != nullptr) *clipboard = selected;
    erase_selection(state);
    return true;
}

bool input_paste(TextInputState& state, std::wstring_view text) {
    return input_insert(state, text);
}

bool input_insert(TextInputState& state, std::wstring_view text) {
    if (text.empty()) return false;
    erase_selection(state);
    state.text.insert(state.caret, text);
    state.caret += text.size();
    state.anchor = state.caret;
    state.composition.clear();
    state.composing = false;
    return true;
}

void input_composition(TextInputState& state, std::wstring_view text,
                       bool committed) {
    if (committed) {
        input_insert(state, text);
        return;
    }
    state.composition.assign(text);
    state.composing = !text.empty();
}

bool input_char(TextInputState& state, wchar_t character) {
    if (character < 0x20) return false;
    return input_insert(state, std::wstring_view(&character, 1));
}

bool input_key(TextInputState& state, int key, bool shift, bool ctrl) {
    clamp_state(state);
    switch (key) {
        case VK_LEFT:
            state.caret = ctrl ? previous_word(state.text, state.caret)
                               : (state.caret > 0 ? state.caret - 1 : 0);
            if (!shift) state.anchor = state.caret;
            return true;
        case VK_RIGHT:
            state.caret = ctrl ? next_word(state.text, state.caret)
                               : std::min(state.caret + 1, state.text.size());
            if (!shift) state.anchor = state.caret;
            return true;
        case VK_HOME:
            state.caret = 0;
            if (!shift) state.anchor = 0;
            return true;
        case VK_END:
            state.caret = state.text.size();
            if (!shift) state.anchor = state.caret;
            return true;
        case VK_BACK:
            if (state.caret != state.anchor) {
                erase_selection(state);
            } else if (state.caret > 0) {
                const std::size_t first =
                    ctrl ? previous_word(state.text, state.caret)
                         : state.caret - 1;
                state.text.erase(first, state.caret - first);
                state.caret = first;
                state.anchor = first;
            }
            return true;
        case VK_DELETE:
            if (state.caret != state.anchor) {
                erase_selection(state);
            } else if (state.caret < state.text.size()) {
                const std::size_t last =
                    ctrl ? next_word(state.text, state.caret)
                         : state.caret + 1;
                state.text.erase(state.caret, last - state.caret);
            }
            return true;
        case 'A':
            if (ctrl) {
                input_select_all(state);
                return true;
            }
            break;
        default:
            break;
    }
    return false;
}

bool input_click(Canvas& canvas, TextInputState& state, const Rect& rect,
                 float x, float scale, bool extend) {
    const float font_size = 12.0f * scale;
    const std::wstring visible = state.text;
    const float local = x - rect.left - 10.0f * scale + state.scroll;
    input_set_caret(state, caret_at(canvas, visible, local, font_size),
                    extend);
    state.focused = true;
    return true;
}

void input_drag_begin(Canvas& canvas, TextInputState& state, const Rect& rect,
                      float x, float scale) {
    input_click(canvas, state, rect, x, scale);
    state.dragging = true;
}

void input_drag_to(Canvas& canvas, TextInputState& state, const Rect& rect,
                   float x, float scale) {
    if (!state.dragging) return;
    input_click(canvas, state, rect, x, scale, true);
}

void input_drag_end(TextInputState& state) { state.dragging = false; }

bool input_right_click(Canvas& canvas, TextInputState& state,
                       const Rect& rect, float x, float scale) {
    if (!state.focused || state.caret == state.anchor) {
        input_click(canvas, state, rect, x, scale);
    }
    state.focused = true;
    state.dragging = false;
    return true;
}

// ---------------------------------------------------------------------------
// Navigation
// ---------------------------------------------------------------------------

void paint_nav_row(Canvas& canvas, const Rect& rect, std::wstring_view label,
                   wchar_t glyph, const Theme& theme, const NavState& state) {
    if (state.selected) {
        canvas.fill_rect(rect, theme.palette.surface);
        canvas.fill_rect(
            Rect{rect.left, rect.top, rect.left + theme.px(2.0f), rect.bottom},
            theme.palette.accent);
    } else if (state.hover) {
        canvas.fill_rect(rect, hover_color(theme));
    }
    const float icon_size = theme.px(16.0f);
    const float icon_top =
        rect.top + std::floor((rect.height() - icon_size) * 0.5f);
    draw_icon(canvas,
              Rect{rect.left + theme.px(12.0f), icon_top,
                   rect.left + theme.px(12.0f) + icon_size,
                   icon_top + icon_size},
              glyph, icon_size,
              state.selected ? theme.palette.accent : theme.palette.secondary);
    draw_text(canvas,
              Rect{rect.left + theme.px(40.0f), rect.top,
                   rect.right - theme.px(12.0f), rect.bottom},
              label, theme.px(14.0f),
              state.selected ? theme.palette.accent
                             : (state.hover ? theme.palette.text
                                            : theme.palette.secondary));
}

void paint_nav_category(Canvas& canvas, const Rect& rect,
                        std::wstring_view label, const Theme& theme,
                        const NavState& state) {
    if (state.selected) canvas.fill_rect(rect, theme.palette.surface);
    else if (state.hover) canvas.fill_rect(rect, hover_color(theme));
    canvas.fill_rect(
        Rect{rect.left, rect.top, rect.left + theme.px(1.0f), rect.bottom},
        state.selected ? theme.palette.accent : theme.palette.border);
    draw_text(canvas,
              Rect{rect.left + theme.px(16.0f), rect.top,
                   rect.right - theme.px(8.0f), rect.bottom},
              label, theme.px(13.0f),
              state.selected ? theme.palette.accent : theme.palette.secondary);
}

// ---------------------------------------------------------------------------
// Splitter
// ---------------------------------------------------------------------------

void paint_splitter(Canvas& canvas, const Rect& area,
                    const SplitterState& state, const Theme& theme) {
    const float line_width = state.dragging || state.hover ? theme.px(2.0f)
                                                           : theme.px(1.0f);
    const float center = state.position + theme.px(5.0f);
    canvas.fill_rect(
        Rect{center - line_width / 2.0f, area.top,
             center + (line_width + 1.0f) / 2.0f, area.bottom},
        state.dragging || state.hover ? theme.palette.accent
                                      : theme.palette.border);
}

bool splitter_press(SplitterState& state, float x, float hit_width) {
    if (x < state.position || x >= state.position + hit_width) return false;
    state.dragging = true;
    state.drag_start_x = x;
    state.drag_start_position = state.position;
    return true;
}

void splitter_move(SplitterState& state, float x) {
    if (!state.dragging) return;
    state.position =
        std::clamp(state.drag_start_position + (x - state.drag_start_x),
                   state.min_position, state.max_position);
}

void splitter_release(SplitterState& state) { state.dragging = false; }

void splitter_reset(SplitterState& state, float default_position) {
    state.position = std::clamp(default_position, state.min_position,
                                state.max_position);
}

// ---------------------------------------------------------------------------
// Scrollbar
// ---------------------------------------------------------------------------

ScrollbarGeometry paint_scrollbar(Canvas& canvas, const Rect& viewport,
                                  const ScrollState& state,
                                  const Theme& theme) {
    ScrollbarGeometry geometry;
    const float max_scroll = state.content_height - state.viewport_height;
    if (max_scroll <= 0.0f || viewport.height() <= 0.0f) return geometry;

    geometry.track =
        Rect{viewport.right - theme.px(14.0f), viewport.top,
             viewport.right - theme.px(4.0f), viewport.bottom};
    const float track_height = geometry.track.height();
    float thumb_height =
        std::max(theme.px(24.0f), track_height * track_height /
                                      std::max(1.0f, track_height + max_scroll));
    thumb_height = std::min(thumb_height, track_height);
    const float available = std::max(0.0f, track_height - thumb_height);
    const float progress =
        std::clamp(state.offset / max_scroll, 0.0f, 1.0f);
    geometry.thumb =
        Rect{geometry.track.left, geometry.track.top + available * progress,
             geometry.track.right,
             geometry.track.top + available * progress + thumb_height};
    geometry.visible = true;

    canvas.fill_rect(geometry.track, theme.palette.surface);
    canvas.stroke_rect(geometry.track, theme.palette.border);
    canvas.fill_rect(geometry.thumb,
                     state.dragging
                         ? theme.palette.accent
                         : (state.hover ? theme.palette.text
                                        : theme.palette.secondary));
    return geometry;
}

bool scrollbar_press(ScrollState& state, const ScrollbarGeometry& geometry,
                     float x, float y) {
    if (!geometry.visible || !geometry.track.contains(x, y)) return false;
    if (geometry.thumb.contains(x, y)) {
        state.dragging = true;
        state.drag_offset = y - geometry.thumb.top;
        return true;
    }
    const float max_scroll = state.content_height - state.viewport_height;
    const float page = std::max(48.0f, geometry.track.height() / 2.0f);
    state.offset += (y < geometry.thumb.top ? -page : page);
    state.offset = std::clamp(state.offset, 0.0f, max_scroll);
    return true;
}

void scrollbar_move(ScrollState& state, const ScrollbarGeometry& geometry,
                    float y) {
    if (!state.dragging || !geometry.visible) return;
    const float available = std::max(
        0.0f, geometry.track.height() - geometry.thumb.height());
    if (available <= 0.0f) return;
    const float top = std::clamp(y - state.drag_offset, geometry.track.top,
                                 geometry.track.top + available);
    const float max_scroll = state.content_height - state.viewport_height;
    state.offset = std::clamp(
        (top - geometry.track.top) / available * max_scroll, 0.0f,
        max_scroll);
}

void scrollbar_release(ScrollState& state) { state.dragging = false; }

void scroll_wheel(ScrollState& state, int delta) {
    const float max_scroll = std::max(0.0f, state.content_height -
                                               state.viewport_height);
    const float step = 48.0f * static_cast<float>(delta) / WHEEL_DELTA;
    state.offset = std::clamp(state.offset - step, 0.0f, max_scroll);
}

// ---------------------------------------------------------------------------
// Selection controls
// ---------------------------------------------------------------------------

void paint_segment(Canvas& canvas, const Rect& rect, std::wstring_view label,
                   const Theme& theme, bool selected, bool hover) {
    if (selected) canvas.fill_rect(rect, theme.palette.surface);
    else if (hover) canvas.fill_rect(rect, hover_color(theme));
    canvas.stroke_rect(rect, selected ? theme.palette.accent
                                      : theme.palette.border);
    draw_text(canvas, rect, label, theme.px(12.0f),
              selected ? theme.palette.accent : theme.palette.secondary,
              FontWeight::normal, TextAlign::center);
}

void paint_toggle(Canvas& canvas, const Rect& rect, std::wstring_view label,
                  const Theme& theme, bool on, bool hover) {
    if (hover) canvas.fill_rect(rect, hover_color(theme));
    canvas.stroke_rect(rect, on ? theme.palette.accent : theme.palette.border);
    draw_text(canvas, rect, label, theme.px(12.0f),
              on ? theme.palette.accent : theme.palette.secondary,
              FontWeight::normal, TextAlign::center);
}

void paint_dropdown(Canvas& canvas, const Rect& rect, std::wstring_view value,
                    const Theme& theme, bool hover, bool open) {
    canvas.fill_rect(rect, theme.palette.bg);
    canvas.stroke_rect(rect, hover || open ? theme.palette.accent
                                           : theme.palette.border);
    draw_text(canvas,
              Rect{rect.left + theme.px(12.0f), rect.top,
                   rect.right - theme.px(28.0f), rect.bottom},
              value, theme.px(13.0f), theme.palette.text);
    draw_icon(canvas,
              Rect{rect.right - theme.px(22.0f), rect.top,
                   rect.right - theme.px(8.0f), rect.bottom},
              static_cast<wchar_t>(0xEA4E), theme.px(12.0f),
              theme.palette.secondary);
}

void paint_tooltip(Canvas& canvas, const Rect& anchor, std::wstring_view text,
                   const Theme& theme, bool visible, const Rect& bounds) {
    if (!visible || text.empty()) return;
    const float font_size = theme.px(11.0f);
    const float width = text_width(canvas, text, font_size) + theme.px(16.0f);
    const float height = theme.px(24.0f);
    float left = anchor.left;
    float top = anchor.top - height - theme.px(4.0f);
    if (top < 0.0f) top = anchor.bottom + theme.px(4.0f);
    if (bounds.width() > 0.0f) {
        left = std::clamp(left, bounds.left, bounds.right - width);
        top = std::clamp(top, bounds.top, bounds.bottom - height);
    }
    const Rect panel{left, top, left + width, top + height};
    canvas.fill_rect(panel, theme.palette.surface);
    canvas.stroke_rect(panel, theme.palette.border);
    draw_text(canvas, panel, text, font_size, theme.palette.secondary,
              FontWeight::normal, TextAlign::center);
}

PopupLayout layout_popup(Canvas& canvas, const Rect& anchor,
                         const std::vector<PopupItem>& items,
                         const Theme& theme, const Rect& bounds) {
    PopupLayout layout;
    float width = theme.px(120.0f);
    float height = 0.0f;
    for (const PopupItem& item : items) {
        if (item.divider) {
            height += theme.px(1.0f);
        } else {
            width = std::max(width, text_width(canvas, item.label,
                                               theme.px(12.0f)) +
                                       theme.px(28.0f));
            height += theme.px(30.0f);
        }
    }
    float left = anchor.left;
    float top = anchor.bottom;
    if (bounds.width() > 0.0f) {
        left = std::clamp(left, bounds.left, bounds.right - width);
        if (top + height > bounds.bottom) {
            top = anchor.top - height;
        }
        top = std::clamp(top, bounds.top, bounds.bottom - height);
    }
    layout.panel = Rect{left, top, left + width, top + height};
    float y = top;
    for (const PopupItem& item : items) {
        const float item_height = item.divider ? theme.px(1.0f)
                                               : theme.px(30.0f);
        layout.items.push_back(
            Rect{left, y, left + width, y + item_height});
        y += item_height;
    }
    return layout;
}

void paint_popup(Canvas& canvas, const PopupLayout& layout,
                 const std::vector<PopupItem>& items, const Theme& theme,
                 int hover) {
    canvas.fill_rect(layout.panel, theme.palette.surface);
    canvas.stroke_rect(layout.panel, theme.palette.border);
    for (std::size_t i = 0; i < items.size() && i < layout.items.size(); ++i) {
        const PopupItem& item = items[i];
        if (item.divider) {
            canvas.fill_rect(layout.items[i], theme.palette.border);
            continue;
        }
        if (static_cast<int>(i) == hover && item.enabled) {
            canvas.fill_rect(layout.items[i], hover_color(theme));
        }
        draw_text(canvas,
                  Rect{layout.items[i].left + theme.px(12.0f),
                       layout.items[i].top,
                       layout.items[i].right - theme.px(12.0f),
                       layout.items[i].bottom},
                  item.label, theme.px(12.0f),
                  item.enabled ? theme.palette.text
                               : theme.palette.secondary.with_alpha(0.5f));
    }
}

int popup_at(const PopupLayout& layout, float x, float y) {
    for (std::size_t i = 0; i < layout.items.size(); ++i) {
        if (layout.items[i].contains(x, y)) return static_cast<int>(i);
    }
    return -1;
}

void paint_focus_rect(Canvas& canvas, const Rect& rect, const Theme& theme) {
    canvas.stroke_rect(rect, theme.palette.accent, theme.px(1.0f));
}

void paint_menu_bar(Canvas& canvas, const Rect& rect,
                    const std::vector<std::wstring>& labels,
                    int open_index, int hover_index, const Theme& theme) {
    canvas.fill_rect(rect, theme.palette.bg);
    canvas.fill_rect(Rect{rect.left, rect.bottom - theme.px(1.0f), rect.right,
                          rect.bottom},
                     theme.palette.border);
    float left = rect.left + theme.px(8.0f);
    for (std::size_t i = 0; i < labels.size(); ++i) {
        const float width =
            text_width(canvas, labels[i], theme.px(12.0f)) + theme.px(24.0f);
        const Rect item{left, rect.top, left + width, rect.bottom};
        const bool active = static_cast<int>(i) == open_index;
        if (active) canvas.fill_rect(item, theme.palette.surface);
        else if (static_cast<int>(i) == hover_index) {
            canvas.fill_rect(item, hover_color(theme));
        }
        draw_text(canvas, item, labels[i], theme.px(12.0f),
                  active ? theme.palette.text : theme.palette.secondary,
                  FontWeight::normal, TextAlign::center);
        left += width;
    }
}

void paint_popup_panel(Canvas& canvas, const Rect& rect,
                       const Theme& theme) {
    canvas.fill_rect(rect, theme.palette.surface);
    canvas.stroke_rect(rect, theme.palette.border);
}

void paint_popup_row(Canvas& canvas, const Rect& rect,
                     std::wstring_view text, const Theme& theme,
                     bool hover, bool divider) {
    if (hover) canvas.fill_rect(rect, hover_color(theme));
    if (divider) {
        canvas.fill_rect(Rect{rect.left, rect.top, rect.right,
                              rect.top + theme.px(1.0f)},
                         theme.palette.border);
        return;
    }
    draw_text(canvas,
              Rect{rect.left + theme.px(12.0f), rect.top,
                   rect.right - theme.px(12.0f), rect.bottom},
              text, theme.px(12.0f), theme.palette.text);
}

// ---------------------------------------------------------------------------
// Larger surfaces
// ---------------------------------------------------------------------------

DialogFrame paint_dialog(Canvas& canvas, const Rect& client,
                         std::wstring_view title, const Theme& theme,
                         float width, float height) {
    canvas.fill_rect(client, Color(0.0f, 0.0f, 0.0f, 0.62f));
    const float dialog_width = theme.px(width);
    const float dialog_height = theme.px(height);
    const float left = (client.width() - dialog_width) / 2.0f;
    const float top = (client.height() - dialog_height) / 2.0f;
    DialogFrame frame;
    frame.panel = Rect{left, top, left + dialog_width, top + dialog_height};
    canvas.fill_rect(frame.panel, theme.palette.surface);
    canvas.stroke_rect(frame.panel, theme.palette.border);
    frame.title = Rect{frame.panel.left + theme.px(24.0f),
                       frame.panel.top + theme.px(20.0f),
                       frame.panel.right - theme.px(24.0f),
                       frame.panel.top + theme.px(48.0f)};
    frame.body = Rect{frame.panel.left + theme.px(24.0f),
                      frame.panel.top + theme.px(60.0f),
                      frame.panel.right - theme.px(24.0f),
                      frame.panel.bottom - theme.px(70.0f)};
    frame.footer = Rect{frame.panel.left + theme.px(24.0f),
                        frame.panel.bottom - theme.px(54.0f),
                        frame.panel.right - theme.px(24.0f),
                        frame.panel.bottom - theme.px(20.0f)};
    draw_text(canvas, frame.title, title, theme.px(17.0f),
              theme.palette.text, FontWeight::semi_bold);
    return frame;
}

void paint_dialog_buttons(Canvas& canvas, const DialogFrame& frame,
                          std::wstring_view cancel,
                          std::wstring_view confirm,
                          const Theme& theme, bool confirm_enabled) {
    const float height = theme.px(32.0f);
    const float top = frame.footer.bottom - height;
    const Rect cancel_rect{frame.footer.right - theme.px(224.0f), top,
                           frame.footer.right - theme.px(124.0f),
                           top + height};
    const Rect confirm_rect{frame.footer.right - theme.px(100.0f), top,
                            frame.footer.right, top + height};
    ButtonStyle style;
    style.bordered = true;
    paint_button(canvas, cancel_rect, cancel, theme, Interaction{}, style);
    style.primary = true;
    Interaction confirm_state;
    confirm_state.enabled = confirm_enabled;
    paint_button(canvas, confirm_rect, confirm, theme, confirm_state, style);
}

void paint_status_bar(Canvas& canvas, const Rect& rect,
                      std::wstring_view left, std::wstring_view right,
                      const Theme& theme) {
    canvas.fill_rect(rect, theme.palette.bg);
    canvas.fill_rect(Rect{rect.left, rect.top, rect.right,
                          rect.top + theme.px(1.0f)},
                     theme.palette.border);
    draw_text(canvas,
              Rect{rect.left + theme.px(12.0f), rect.top,
                   rect.right - theme.px(100.0f), rect.bottom},
              left, theme.px(12.0f), theme.palette.secondary);
    draw_text(canvas,
              Rect{rect.right - theme.px(92.0f), rect.top,
                   rect.right - theme.px(12.0f), rect.bottom},
              right, theme.px(12.0f), theme.palette.secondary,
              FontWeight::normal, TextAlign::trailing);
}

void paint_progress_bar(Canvas& canvas, const Rect& rect, float ratio,
                        const Theme& theme) {
    canvas.fill_rect(rect, theme.palette.border);
    const float clamped = std::clamp(ratio, 0.0f, 1.0f);
    canvas.fill_rect(Rect{rect.left, rect.top,
                          rect.left + rect.width() * clamped, rect.bottom},
                     theme.palette.accent);
}

void paint_card(Canvas& canvas, const Rect& rect, const Theme& theme) {
    canvas.fill_rect(rect, theme.palette.surface);
    canvas.stroke_rect(rect, theme.palette.border);
}

std::vector<Rect> paint_card_row(Canvas& canvas, const Rect& rect,
                                 std::wstring_view title,
                                 std::wstring_view subtitle,
                                 const std::vector<std::wstring>& actions,
                                 const Theme& theme, bool selected,
                                 bool hover) {
    if (selected) canvas.fill_rect(rect, theme.palette.surface);
    else if (hover) canvas.fill_rect(rect, hover_color(theme));
    const float action_height = theme.px(30.0f);
    const float gap = theme.px(8.0f);
    const float action_width = theme.px(72.0f);
    const float right = rect.right - theme.px(16.0f);
    const float total = actions.empty()
                            ? 0.0f
                            : static_cast<float>(actions.size()) *
                                      action_width +
                                  static_cast<float>(actions.size() - 1) * gap;
    draw_text(canvas,
              Rect{rect.left + theme.px(16.0f), rect.top + theme.px(8.0f),
                   right - total - theme.px(16.0f),
                   rect.top + theme.px(26.0f)},
              title, theme.px(14.0f),
              selected ? theme.palette.accent : theme.palette.text);
    if (!subtitle.empty()) {
        draw_text(canvas,
                  Rect{rect.left + theme.px(16.0f),
                       rect.top + theme.px(28.0f),
                       right - total - theme.px(16.0f),
                       rect.top + theme.px(46.0f)},
                  subtitle, theme.px(11.0f), theme.palette.secondary);
    }
    std::vector<Rect> result;
    result.reserve(actions.size());
    float left = right - total;
    ButtonStyle style;
    style.bordered = true;
    for (const std::wstring& action : actions) {
        const Rect button{left, rect.top + (rect.height() - action_height) / 2.0f,
                          left + action_width,
                          rect.top + (rect.height() + action_height) / 2.0f};
        paint_button(canvas, button, action, theme, Interaction{}, style);
        result.push_back(button);
        left += action_width + gap;
    }
    return result;
}

// ---------------------------------------------------------------------------
// Field layout
// ---------------------------------------------------------------------------

float field_height(const FieldSpec& spec, const Theme& theme) {
    const float minimum = spec.hint.empty() ? 42.0f : 60.0f;
    return std::max(minimum, spec.control_height + 12.0f) * theme.scale;
}

FieldLayout layout_field(const Rect& row, const FieldSpec& spec,
                         const Theme& theme) {
    FieldLayout layout;
    layout.row = row;
    layout.height = row.height();
    const float label_height = theme.px(24.0f);
    const float hint_height = theme.px(20.0f);
    const float block =
        spec.hint.empty() ? label_height : label_height + hint_height;
    const float top = row.top + (row.height() - block) / 2.0f;
    layout.label = Rect{row.left + theme.px(16.0f), top,
                        row.right - theme.px(16.0f), top + label_height};
    if (!spec.hint.empty()) {
        layout.hint = Rect{row.left + theme.px(16.0f), top + label_height,
                           row.right - theme.px(16.0f), top + block};
    }
    const float control_height = theme.px(spec.control_height);
    if (spec.full_width) {
        layout.control =
            Rect{row.left + theme.px(16.0f),
                 row.top + (row.height() - control_height) / 2.0f,
                 row.right - theme.px(16.0f),
                 row.top + (row.height() + control_height) / 2.0f};
    } else if (spec.control_width > 0.0f) {
        const float width = theme.px(spec.control_width);
        layout.control =
            Rect{row.right - theme.px(16.0f) - width,
                 row.top + (row.height() - control_height) / 2.0f,
                 row.right - theme.px(16.0f),
                 row.top + (row.height() + control_height) / 2.0f};
    } else {
        layout.control = row;
    }
    return layout;
}

}  // namespace jo::draftui
