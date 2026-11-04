#pragma once

#include <filesystem>
#include <memory>
#include <string_view>

#include <windows.h>

#include "color.hpp"
#include "theme.hpp"

namespace jo::draftui {

struct Rect {
    float left = 0.0f;
    float top = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;

    constexpr float width() const { return right - left; }
    constexpr float height() const { return bottom - top; }
    constexpr bool contains(float x, float y) const {
        return x >= left && x < right && y >= top && y < bottom;
    }
};

enum class FontFamily { sans, serif, mono, icon };
enum class FontWeight { normal, semi_bold, bold };
enum class TextAlign { leading, center, trailing };

struct TextStyle {
    FontFamily family = FontFamily::sans;
    float size = 12.0f;
    Color color{};
    FontWeight weight = FontWeight::normal;
    TextAlign align = TextAlign::leading;
    bool center_vertical = false;
    bool wrap = false;
};

/// Drawing surface used by JO-DraftUI controls. Coordinates are physical
/// pixels; callers multiply logical tokens through Theme::px.
class Canvas {
public:
    virtual ~Canvas() = default;

    virtual void clear(Color color) = 0;
    virtual void fill_rect(const Rect& rect, Color color) = 0;
    virtual void fill_rounded_rect(const Rect& rect, float radius,
                                   Color color) = 0;
    virtual void stroke_rect(const Rect& rect, Color color,
                             float width = 1.0f) = 0;
    virtual void stroke_rounded_rect(const Rect& rect, float radius, Color color,
                                     float width = 1.0f) = 0;
    virtual void draw_line(float x1, float y1, float x2, float y2, Color color,
                           float width = 1.0f) = 0;
    virtual void draw_text(const Rect& rect, std::wstring_view text,
                           const TextStyle& style) = 0;
    virtual float measure_text(std::wstring_view text,
                               const TextStyle& style) = 0;
    virtual void push_clip(const Rect& rect) = 0;
    virtual void pop_clip() = 0;
    virtual void push_translate(float x, float y) = 0;
    virtual void pop_transform() = 0;
};

/// Direct2D + DirectWrite implementation. It owns the render target for one
/// HWND and must be resized when the client area changes.
class D2DCanvas final : public Canvas {
public:
    explicit D2DCanvas(HWND window);
    ~D2DCanvas() override;

    D2DCanvas(const D2DCanvas&) = delete;
    D2DCanvas& operator=(const D2DCanvas&) = delete;

    bool add_font_file(const std::filesystem::path& path, FontFamily family);
    bool resize(UINT width, UINT height);
    bool begin_draw();
    void end_draw();
    bool valid() const;

    void clear(Color color) override;
    void fill_rect(const Rect& rect, Color color) override;
    void fill_rounded_rect(const Rect& rect, float radius,
                           Color color) override;
    void stroke_rect(const Rect& rect, Color color,
                     float width = 1.0f) override;
    void stroke_rounded_rect(const Rect& rect, float radius, Color color,
                             float width = 1.0f) override;
    void draw_line(float x1, float y1, float x2, float y2, Color color,
                   float width = 1.0f) override;
    void draw_text(const Rect& rect, std::wstring_view text,
                   const TextStyle& style) override;
    float measure_text(std::wstring_view text,
                       const TextStyle& style) override;
    void push_clip(const Rect& rect) override;
    void pop_clip() override;
    void push_translate(float x, float y) override;
    void pop_transform() override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace jo::draftui
