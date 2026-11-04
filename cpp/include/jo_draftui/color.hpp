#pragma once

#include <cstdint>

namespace jo::draftui {

/// Linear-ish UI color; values are in the usual 0..1 range.
struct Color {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;

    constexpr Color() = default;
    constexpr Color(float red, float green, float blue, float alpha = 1.0f)
        : r(red), g(green), b(blue), a(alpha) {}

    /// 0xAARRGGBB
    static constexpr Color from_argb(std::uint32_t argb) {
        return Color(static_cast<float>((argb >> 16) & 0xFFu) / 255.0f,
                     static_cast<float>((argb >> 8) & 0xFFu) / 255.0f,
                     static_cast<float>(argb & 0xFFu) / 255.0f,
                     static_cast<float>((argb >> 24) & 0xFFu) / 255.0f);
    }

    constexpr Color with_alpha(float alpha) const {
        return Color(r, g, b, alpha);
    }
};

constexpr Color lerp(Color from, Color to, float t) {
    return Color(from.r + (to.r - from.r) * t,
                 from.g + (to.g - from.g) * t,
                 from.b + (to.b - from.b) * t,
                 from.a + (to.a - from.a) * t);
}

/// Composite `overlay` on top of `base` using normal alpha blending.
constexpr Color alpha_blend(Color base, Color overlay) {
    const float a = overlay.a + base.a * (1.0f - overlay.a);
    if (a <= 0.0f) return Color(0.0f, 0.0f, 0.0f, 0.0f);
    return Color((overlay.r * overlay.a + base.r * base.a * (1.0f - overlay.a)) /
                     a,
                 (overlay.g * overlay.a + base.g * base.a * (1.0f - overlay.a)) /
                     a,
                 (overlay.b * overlay.a + base.b * base.a * (1.0f - overlay.a)) /
                     a,
                 a);
}

}  // namespace jo::draftui
