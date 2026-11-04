#pragma once

#include <string_view>

#include "color.hpp"
#include "tokens_generated.hpp"

namespace jo::draftui {

struct Palette {
    Color bg;
    Color surface;
    Color border;
    Color text;
    Color secondary;
    Color icon;
    Color accent;
    bool is_dark = true;
    /// Optional exact hover surface. Transparent means derive from text alpha.
    Color hover = Color(0.0f, 0.0f, 0.0f, 0.0f);

    static constexpr Palette dark() {
        return Palette{tokens::dark_bg, tokens::dark_surface,
                       tokens::dark_border, tokens::dark_text,
                       tokens::dark_secondary, tokens::dark_icon,
                       tokens::dark_accent, true};
    }

    static constexpr Palette light() {
        return Palette{tokens::light_bg, tokens::light_surface,
                       tokens::light_border, tokens::light_text,
                       tokens::light_secondary, tokens::light_icon,
                       tokens::light_accent, false};
    }
};

struct Typography {
    std::wstring_view sans = tokens::sans_family;
    std::wstring_view serif = tokens::serif_family;
    std::wstring_view mono = tokens::mono_family;
    float ui_size = 14.0f;
    float meta_size = tokens::meta_size;
    float body_size = tokens::body_size;
    float code_size = 13.0f;
};

struct Theme {
    Palette palette = Palette::dark();
    Typography type{};
    /// Logical pixels to physical pixels. 1.0 at 96 DPI.
    float scale = 1.0f;

    constexpr float px(float logical) const { return logical * scale; }

    constexpr Color hover_overlay() const {
        return palette.hover.a > 0.0f ? palette.hover
                                       : palette.text.with_alpha(0.09f);
    }

    constexpr Color pressed_overlay() const {
        return palette.text.with_alpha(0.20f);
    }

    constexpr Color selected_overlay() const {
        return palette.accent.with_alpha(0.14f);
    }

    constexpr Theme with_scale(float value) const {
        Theme copy = *this;
        copy.scale = value;
        return copy;
    }

    constexpr Theme with_palette(Palette value) const {
        Theme copy = *this;
        copy.palette = value;
        return copy;
    }
};

}  // namespace jo::draftui
