#include "jo_draftui/controls/markdown.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace jo::draftui {
namespace {

TextStyle text_style(float size, Color color, FontWeight weight) {
    TextStyle style;
    style.family = FontFamily::sans;
    style.size = size;
    style.color = color;
    style.weight = weight;
    style.center_vertical = true;
    return style;
}

std::vector<std::wstring> wrap_line(Canvas& canvas, std::wstring_view text,
                                    float width, float font_size,
                                    FontWeight weight) {
    std::vector<std::wstring> lines;
    std::wstring current;
    TextStyle measure;
    measure.family = FontFamily::sans;
    measure.size = font_size;
    measure.weight = weight;
    for (wchar_t character : text) {
        if (character == L'\r') continue;
        if (character == L'\n') {
            lines.push_back(std::move(current));
            current.clear();
            continue;
        }
        current.push_back(character);
        if (canvas.measure_text(current, measure) > width && current.size() > 1) {
            const wchar_t last = current.back();
            current.pop_back();
            lines.push_back(std::move(current));
            current.clear();
            current.push_back(last);
        }
    }
    if (!current.empty() || lines.empty()) lines.push_back(std::move(current));
    return lines;
}

}  // namespace

float paint_markdown(Canvas& canvas, const Rect& rect, std::wstring_view text,
                     const Theme& theme) {
    if (text.empty() || rect.width() <= 0.0f) return 0.0f;
    const float body_size = theme.px(12.0f);
    const float body_line = theme.px(20.0f);
    const float heading_size = theme.px(16.0f);
    const float heading_line = theme.px(28.0f);
    float y = rect.top;

    std::size_t start = 0;
    while (start <= text.size()) {
        const std::size_t end = text.find(L'\n', start);
        const std::wstring_view raw =
            text.substr(start, end == std::wstring_view::npos
                                   ? text.size() - start
                                   : end - start);
        std::wstring_view line = raw;
        if (!line.empty() && line.back() == L'\r') line.remove_suffix(1);

        bool heading = false;
        bool bullet = false;
        if (line.starts_with(L"## ")) {
            line.remove_prefix(3);
            heading = true;
        } else if (line.starts_with(L"# ")) {
            line.remove_prefix(2);
            heading = true;
        } else if (line.starts_with(L"- ") || line.starts_with(L"* ")) {
            line.remove_prefix(2);
            bullet = true;
        }

        if (line.empty()) {
            y += body_line * 0.45f;
        } else {
            const float font = heading ? heading_size : body_size;
            const FontWeight weight =
                heading ? FontWeight::bold : FontWeight::normal;
            const float left = rect.left + (bullet ? theme.px(14.0f) : 0.0f);
            const float line_height = heading ? heading_line : body_line;
            const auto lines = wrap_line(canvas, line, rect.right - left, font,
                                         weight);
            if (bullet && y + line_height <= rect.bottom) {
                canvas.draw_text(Rect{rect.left, y, rect.left + theme.px(12.0f),
                                      y + line_height},
                                 L"•", text_style(font, theme.palette.secondary,
                                                  weight));
            }
            for (const std::wstring& wrapped : lines) {
                if (y + line_height > rect.bottom) return y - rect.top;
                canvas.draw_text(
                    Rect{left, y, rect.right, y + line_height}, wrapped,
                    text_style(font, heading ? theme.palette.text
                                             : theme.palette.secondary,
                               weight));
                y += line_height;
            }
        }
        if (end == std::wstring_view::npos) break;
        start = end + 1;
    }
    return y - rect.top;
}

}  // namespace jo::draftui
