#pragma once

#include <string_view>

#include "jo_draftui/canvas.hpp"
#include "jo_draftui/theme.hpp"

namespace jo::draftui {

/// Small markdown renderer for short descriptions: headings, bullet lists,
/// blank lines and wrapped paragraphs. Inline emphasis is rendered as plain
/// text; the layout engine is intentionally not a full CommonMark parser.
float paint_markdown(Canvas& canvas, const Rect& rect, std::wstring_view text,
                     const Theme& theme);

}  // namespace jo::draftui
