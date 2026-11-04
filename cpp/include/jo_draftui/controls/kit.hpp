#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "jo_draftui/canvas.hpp"
#include "jo_draftui/controls/basic.hpp"
#include "jo_draftui/theme.hpp"

namespace jo::draftui {

// ---------------------------------------------------------------------------
// Text input
// ---------------------------------------------------------------------------

struct TextInputState {
    std::wstring text;
    std::wstring composition;
    std::size_t caret = 0;
    std::size_t anchor = 0;
    float scroll = 0.0f;
    bool focused = false;
    bool composing = false;
    bool dragging = false;
};

struct TextInputStyle {
    bool password = false;
    bool enabled = true;
    bool error = false;
};

/// Draws the input box and its caret/selection. The input remains a pure
/// paint operation; state changes go through the helpers below.
void paint_text_input(Canvas& canvas, const Rect& rect,
                      const TextInputState& state, const Theme& theme,
                      const TextInputStyle& style = {});

void input_set_caret(TextInputState& state, std::size_t caret,
                     bool extend = false);
void input_select_all(TextInputState& state);
std::wstring input_selected_text(const TextInputState& state);
void input_select_word_at(TextInputState& state, std::size_t index);
bool input_cut(TextInputState& state, std::wstring* clipboard = nullptr);
bool input_paste(TextInputState& state, std::wstring_view text);
void input_composition(TextInputState& state, std::wstring_view text,
                       bool committed);
bool input_insert(TextInputState& state, std::wstring_view text);
bool input_char(TextInputState& state, wchar_t character);
bool input_key(TextInputState& state, int key, bool shift = false,
               bool ctrl = false);
bool input_click(Canvas& canvas, TextInputState& state, const Rect& rect,
                 float x, float scale, bool extend = false);
void input_drag_begin(Canvas& canvas, TextInputState& state, const Rect& rect,
                      float x, float scale);
void input_drag_to(Canvas& canvas, TextInputState& state, const Rect& rect,
                   float x, float scale);
void input_drag_end(TextInputState& state);
bool input_right_click(Canvas& canvas, TextInputState& state,
                       const Rect& rect, float x, float scale);

enum class TextCommand { cut, copy, paste, select_all };
struct TextMenuState {
    bool cut = false;
    bool copy = false;
    bool paste = false;
    bool select_all = false;
};

// ---------------------------------------------------------------------------
// Navigation
// ---------------------------------------------------------------------------

struct NavState {
    bool selected = false;
    bool hover = false;
};

/// A 37 px navigation row with a 16 px leading icon and a 2 px accent rail.
void paint_nav_row(Canvas& canvas, const Rect& rect, std::wstring_view label,
                   wchar_t glyph, const Theme& theme, const NavState& state);

/// A 34 px indented category row with a 1 px leading border.
void paint_nav_category(Canvas& canvas, const Rect& rect,
                        std::wstring_view label, const Theme& theme,
                        const NavState& state);

/// A list card row with a leading icon, title and optional subtitle.
void paint_list_card(Canvas& canvas, const Rect& rect,
                     std::wstring_view title, std::wstring_view subtitle,
                     wchar_t glyph, const Theme& theme,
                     const Interaction& state, bool selected = false);

/// A bordered result row with a title, optional subtitle and optional
/// right-aligned trailing text. `trailing_width` is in logical pixels.
void paint_result_row(Canvas& canvas, const Rect& rect,
                      std::wstring_view title, std::wstring_view subtitle,
                      std::wstring_view trailing, float trailing_width,
                      const Theme& theme, bool trailing_right = true,
                      float card_bottom_inset = 0.0f);

// ---------------------------------------------------------------------------
// Splitter
// ---------------------------------------------------------------------------

struct SplitterState {
    float position = 232.0f;
    float min_position = 200.0f;
    float max_position = 360.0f;
    float drag_start_x = 0.0f;
    float drag_start_position = 0.0f;
    bool dragging = false;
    bool hover = false;
};

/// Paints the 1 px idle / 2 px active line at state.position.
void paint_splitter(Canvas& canvas, const Rect& area,
                    const SplitterState& state, const Theme& theme);
bool splitter_press(SplitterState& state, float x, float hit_width = 10.0f);
void splitter_move(SplitterState& state, float x);
void splitter_release(SplitterState& state);
void splitter_reset(SplitterState& state, float default_position);

// ---------------------------------------------------------------------------
// Scrollbar
// ---------------------------------------------------------------------------

struct ScrollState {
    float offset = 0.0f;
    float content_height = 0.0f;
    float viewport_height = 0.0f;
    float drag_offset = 0.0f;
    bool dragging = false;
    bool hover = false;
};

struct ScrollbarGeometry {
    Rect track{};
    Rect thumb{};
    bool visible = false;
};

ScrollbarGeometry paint_scrollbar(Canvas& canvas, const Rect& viewport,
                                  const ScrollState& state,
                                  const Theme& theme);
bool scrollbar_press(ScrollState& state, const ScrollbarGeometry& geometry,
                     float x, float y);
void scrollbar_move(ScrollState& state, const ScrollbarGeometry& geometry,
                    float y);
void scrollbar_release(ScrollState& state);
void scroll_wheel(ScrollState& state, int delta);

// ---------------------------------------------------------------------------
// Selection controls (Flutter extensions, not part of JOL's settings page)
// ---------------------------------------------------------------------------

void paint_segment(Canvas& canvas, const Rect& rect, std::wstring_view label,
                   const Theme& theme, bool selected, bool hover = false);
void paint_toggle(Canvas& canvas, const Rect& rect, std::wstring_view label,
                  const Theme& theme, bool on, bool hover = false);
void paint_dropdown(Canvas& canvas, const Rect& rect, std::wstring_view value,
                    const Theme& theme, bool hover = false,
                    bool open = false);

/// A small surface panel above (or below, when there is no room) an anchor.
void paint_tooltip(Canvas& canvas, const Rect& anchor, std::wstring_view text,
                   const Theme& theme, bool visible = true,
                   const Rect& bounds = {});

struct PopupItem {
    std::wstring label;
    bool divider = false;
    bool enabled = true;
};

struct PopupLayout {
    Rect panel{};
    std::vector<Rect> items;
};

PopupLayout layout_popup(Canvas& canvas, const Rect& anchor,
                         const std::vector<PopupItem>& items,
                         const Theme& theme, const Rect& bounds = {});
void paint_popup(Canvas& canvas, const PopupLayout& layout,
                 const std::vector<PopupItem>& items, const Theme& theme,
                 int hover = -1);
int popup_at(const PopupLayout& layout, float x, float y);

/// A one-pixel accent outline for keyboard focus. Callers keep ownership of
/// the focus chain.
void paint_focus_rect(Canvas& canvas, const Rect& rect, const Theme& theme);

void paint_menu_bar(Canvas& canvas, const Rect& rect,
                    const std::vector<std::wstring>& labels,
                    int open_index, int hover_index, const Theme& theme);
void paint_popup_panel(Canvas& canvas, const Rect& rect,
                       const Theme& theme);
void paint_popup_row(Canvas& canvas, const Rect& rect,
                     std::wstring_view text, const Theme& theme,
                     bool hover = false, bool divider = false);

// ---------------------------------------------------------------------------
// Larger surfaces
// ---------------------------------------------------------------------------

struct DialogFrame {
    Rect panel{};
    Rect title{};
    Rect body{};
    Rect footer{};
};

/// Dims the client and paints a centred dialog panel. Content is left to the
/// caller; this function only owns the shared frame geometry.
DialogFrame paint_dialog(Canvas& canvas, const Rect& client,
                         std::wstring_view title, const Theme& theme,
                         float width = 460.0f, float height = 330.0f);

/// Paints a standard action button row inside a dialog panel.
void paint_dialog_buttons(Canvas& canvas, const DialogFrame& frame,
                          std::wstring_view cancel,
                          std::wstring_view confirm,
                          const Theme& theme, bool confirm_enabled = true);

void paint_status_bar(Canvas& canvas, const Rect& rect,
                      std::wstring_view left, std::wstring_view right,
                      const Theme& theme);

void paint_progress_bar(Canvas& canvas, const Rect& rect, float ratio,
                        const Theme& theme);

/// A settings-style card: title above a surface with a 1 px border.
void paint_card(Canvas& canvas, const Rect& rect, const Theme& theme);

/// A card row with a title, optional subtitle, and optional action buttons.
/// Returns the action rectangles in the same order as `actions`.
std::vector<Rect> paint_card_row(Canvas& canvas, const Rect& rect,
                                 std::wstring_view title,
                                 std::wstring_view subtitle,
                                 const std::vector<std::wstring>& actions,
                                 const Theme& theme, bool selected = false,
                                 bool hover = false);

// ---------------------------------------------------------------------------
// Field layout
// ---------------------------------------------------------------------------

struct FieldSpec {
    std::wstring_view label;
    std::wstring_view hint;
    float control_width = 0.0f;
    float control_height = 30.0f;
    bool full_width = false;
};

struct FieldLayout {
    Rect row{};
    Rect label{};
    Rect hint{};
    Rect control{};
    float height = 0.0f;
};

/// Height is derived from the content, never trusted from the caller: a hint
/// needs a second text line, so it gets at least 60 px while plain rows get 42.
float field_height(const FieldSpec& spec, const Theme& theme);
FieldLayout layout_field(const Rect& row, const FieldSpec& spec,
                         const Theme& theme);

}  // namespace jo::draftui
