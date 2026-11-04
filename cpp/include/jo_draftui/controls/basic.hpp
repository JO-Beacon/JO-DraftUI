#pragma once

#include <string_view>

#include "jo_draftui/canvas.hpp"
#include "jo_draftui/theme.hpp"

namespace jo::draftui {

struct Interaction {
    bool hover = false;
    bool pressed = false;
    bool enabled = true;
};

struct ButtonStyle {
    bool bordered = true;
    bool primary = false;
    bool danger = false;
};

/// A compact 1px-line button. Height is driven by the caller; text is centered.
void paint_button(Canvas& canvas, const Rect& rect, std::wstring_view text,
                  const Theme& theme, const Interaction& state,
                  const ButtonStyle& style = {});

/// A text icon action. `glyph` is expected to be a glyph from the icon font.
void paint_icon_action(Canvas& canvas, const Rect& rect,
                       std::wstring_view glyph, const Theme& theme,
                       const Interaction& state, bool inverted = false);

/// List row: optional subtitle, hover, selected, and a 2px selected rail.
void paint_list_row(Canvas& canvas, const Rect& rect, std::wstring_view title,
                    std::wstring_view subtitle, const Theme& theme,
                    const Interaction& state, bool selected = false);

/// Page header: 20px title at 58 px and 12px subtitle at 90 px.
void paint_page_header(Canvas& canvas, const Rect& area,
                       std::wstring_view title, std::wstring_view subtitle,
                       const Theme& theme);

/// Section box: a small label above a surface panel with a 1px border.
void paint_section_header(Canvas& canvas, const Rect& rect,
                          std::wstring_view title, const Theme& theme);

/// Settings row: left label/hint, right control area.
void paint_field(Canvas& canvas, const Rect& rect, std::wstring_view label,
                 std::wstring_view hint, const Theme& theme);

}  // namespace jo::draftui
