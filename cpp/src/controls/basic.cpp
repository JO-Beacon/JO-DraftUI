#include "jo_draftui/controls/basic.hpp"

#include <algorithm>
#include <cmath>

namespace jo::draftui {
namespace {

Color overlay_color(const Theme& theme, const Interaction& state) {
    if (state.pressed) return theme.pressed_overlay();
    if (state.hover) return theme.hover_overlay();
    return Color(0.0f, 0.0f, 0.0f, 0.0f);
}

Color disabled_color(const Theme& theme) {
    return theme.palette.secondary.with_alpha(0.5f);
}

}  // namespace

void paint_button(Canvas& canvas, const Rect& rect, std::wstring_view text,
                  const Theme& theme, const Interaction& state,
                  const ButtonStyle& style) {
    const float radius = 0.0f;
    const Color base = !state.enabled
                           ? disabled_color(theme)
                           : (style.danger ? Color(0.77f, 0.17f, 0.11f)
                                           : (style.primary ? theme.palette.accent
                                                            : theme.palette.secondary));
    const Color overlay = state.enabled ? overlay_color(theme, state)
                                        : Color(0.0f, 0.0f, 0.0f, 0.0f);
    if (overlay.a > 0.0f) {
        canvas.fill_rounded_rect(rect, radius, overlay);
    }

    if (style.bordered) {
        const Color border = state.enabled && (state.hover || state.pressed)
                                 ? theme.palette.accent
                                 : theme.palette.border;
        canvas.stroke_rounded_rect(rect, radius, border,
                                   theme.px(tokens::border_width));
    }

    TextStyle style_text;
    style_text.family = FontFamily::sans;
    style_text.size = theme.px(theme.type.meta_size);
    style_text.color = !state.enabled
                           ? disabled_color(theme)
                           : style.primary ? theme.palette.accent : base;
    style_text.align = TextAlign::center;
    style_text.center_vertical = true;
    canvas.draw_text(rect, text, style_text);
}

void paint_icon_action(Canvas& canvas, const Rect& rect,
                       std::wstring_view glyph, const Theme& theme,
                       const Interaction& state, bool inverted) {
    const Color overlay = state.enabled ? overlay_color(theme, state)
                                        : Color(0.0f, 0.0f, 0.0f, 0.0f);
    const Color background = inverted
                                 ? alpha_blend(theme.palette.accent, overlay)
                                 : overlay;
    if (background.a > 0.0f) {
        canvas.fill_rect(rect, background);
    }
    TextStyle text;
    text.family = FontFamily::icon;
    text.size = theme.px(16.0f);
    text.align = TextAlign::center;
    text.center_vertical = true;
    text.color = !state.enabled
                     ? disabled_color(theme)
                     : inverted ? theme.palette.bg
                                : (state.hover || state.pressed
                                       ? theme.palette.text
                                       : theme.palette.icon);
    canvas.draw_text(rect, glyph, text);
}

void paint_list_row(Canvas& canvas, const Rect& rect, std::wstring_view title,
                    std::wstring_view subtitle, const Theme& theme,
                    const Interaction& state, bool selected) {
    if (selected) {
        canvas.fill_rect(rect, theme.palette.surface);
        canvas.fill_rect(Rect{rect.left, rect.top, rect.left + theme.px(2.0f),
                              rect.bottom},
                         theme.palette.accent);
    } else if (state.hover && state.enabled) {
        canvas.fill_rect(rect, state.pressed ? theme.pressed_overlay()
                                             : theme.hover_overlay());
    }

    const float left = rect.left + theme.px(tokens::s3);
    const float right = rect.right - theme.px(tokens::s3);
    const float title_top = subtitle.empty()
                                ? rect.top + theme.px(6.0f)
                                : rect.top + theme.px(4.0f);
    TextStyle title_style;
    title_style.family = FontFamily::sans;
    title_style.size = theme.px(13.0f);
    title_style.color = !state.enabled
                            ? disabled_color(theme)
                            : selected ? theme.palette.accent
                                       : theme.palette.text;
    canvas.draw_text(Rect{left, title_top, right, title_top + theme.px(18.0f)},
                     title, title_style);

    if (!subtitle.empty()) {
        TextStyle subtitle_style;
        subtitle_style.family = FontFamily::sans;
        subtitle_style.size = theme.px(theme.type.meta_size);
        subtitle_style.color = theme.palette.secondary;
        canvas.draw_text(
            Rect{left, title_top + theme.px(17.0f), right,
                 title_top + theme.px(32.0f)},
            subtitle, subtitle_style);
    }
}

void paint_page_header(Canvas& canvas, const Rect& area,
                       std::wstring_view title, std::wstring_view subtitle,
                       const Theme& theme) {
    TextStyle title_style;
    title_style.family = FontFamily::sans;
    title_style.size = theme.px(20.0f);
    title_style.color = theme.palette.text;
    title_style.weight = FontWeight::bold;
    canvas.draw_text(
        Rect{area.left, theme.px(58.0f), area.right, theme.px(88.0f)},
        title, title_style);
    if (!subtitle.empty()) {
        TextStyle subtitle_style;
        subtitle_style.family = FontFamily::sans;
        subtitle_style.size = theme.px(12.0f);
        subtitle_style.color = theme.palette.secondary;
        canvas.draw_text(
            Rect{area.left, theme.px(90.0f), area.right, theme.px(112.0f)},
            subtitle, subtitle_style);
    }
}

void paint_section_header(Canvas& canvas, const Rect& rect,
                          std::wstring_view title, const Theme& theme) {
    TextStyle style;
    style.family = FontFamily::sans;
    style.size = theme.px(theme.type.meta_size);
    style.color = theme.palette.secondary;
    style.center_vertical = true;
    canvas.draw_text(rect, title, style);
}

void paint_field(Canvas& canvas, const Rect& rect, std::wstring_view label,
                 std::wstring_view hint, const Theme& theme) {
    const float left = rect.left + theme.px(tokens::s4);
    const float right = rect.right - theme.px(tokens::s4);
    const float label_height = theme.px(24.0f);
    const float hint_height = theme.px(20.0f);
    const float block = hint.empty() ? label_height : label_height + hint_height;
    const float top =
        rect.top + std::floor((rect.height() - block) * 0.5f);
    TextStyle label_style;
    label_style.family = FontFamily::sans;
    label_style.size = theme.px(14.0f);
    label_style.color = theme.palette.text;
    label_style.center_vertical = true;
    canvas.draw_text(Rect{left, top, right, top + label_height}, label,
                     label_style);

    if (!hint.empty()) {
        TextStyle hint_style;
        hint_style.family = FontFamily::sans;
        hint_style.size = theme.px(theme.type.meta_size);
        hint_style.color = theme.palette.secondary;
        hint_style.center_vertical = true;
        canvas.draw_text(Rect{left, top + label_height, right, top + block},
                         hint, hint_style);
    }
}

}  // namespace jo::draftui
