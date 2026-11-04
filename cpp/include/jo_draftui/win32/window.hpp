#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>

#include <windows.h>

#include "jo_draftui/canvas.hpp"

namespace jo::draftui::win32 {

/// A small Win32 host for the Direct2D canvas. It intentionally does not
/// implement an application framework: callers own layout and state.
class Window {
public:
    using PaintCallback =
        std::function<void(Canvas& canvas, const Rect& client, float scale)>;
    using MouseMoveCallback = std::function<void(float x, float y)>;
    using MouseButtonCallback =
        std::function<void(float x, float y, bool down)>;
    using LeaveCallback = std::function<void()>;
    using CanvasInitCallback = std::function<void(D2DCanvas&)>;
    using KeyCallback = std::function<bool(UINT)>;
    using CharCallback = std::function<bool(wchar_t)>;
    using WheelCallback = std::function<void(int)>;
    using DoubleClickCallback = std::function<void(float x, float y)>;
    using CursorCallback = std::function<HCURSOR()>;
    using ImeCallback = std::function<void(std::wstring_view text,
                                          bool committed)>;
    using TimerCallback = std::function<void()>;

    Window();
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool create(const std::wstring& title, int width, int height);
    int run();
    void invalidate();

    HWND hwnd() const;
    float scale() const;

    void set_dark_titlebar(bool dark);
    void set_titlebar_theme(bool dark, Color background, Color border,
                            Color text);
    void set_canvas_init_callback(CanvasInitCallback callback);
    void set_key_callback(KeyCallback callback);
    void set_char_callback(CharCallback callback);
    void set_wheel_callback(WheelCallback callback);
    void set_paint_callback(PaintCallback callback);
    void set_mouse_move_callback(MouseMoveCallback callback);
    void set_mouse_button_callback(MouseButtonCallback callback);
    void set_middle_button_callback(MouseButtonCallback callback);
    void set_right_button_callback(MouseButtonCallback callback);
    void set_double_click_callback(DoubleClickCallback callback);
    void set_cursor_callback(CursorCallback callback);
    void set_ime_callback(ImeCallback callback);
    void set_ime_position(const Rect& caret);
    void set_timer(UINT id, UINT milliseconds, TimerCallback callback);
    void kill_timer(UINT id);
    void set_mouse_leave_callback(LeaveCallback callback);

private:
    static LRESULT CALLBACK window_proc(HWND window, UINT message,
                                        WPARAM wparam, LPARAM lparam);
    LRESULT handle_message(HWND window, UINT message, WPARAM wparam,
                           LPARAM lparam);
    void paint(HWND window);
    void ensure_canvas(HWND window, UINT width, UINT height);
    float dpi_scale(HWND window) const;

    HWND window_ = nullptr;
    std::unique_ptr<D2DCanvas> canvas_;
    UINT canvas_width_ = 0;
    UINT canvas_height_ = 0;
    CanvasInitCallback canvas_init_callback_;
    KeyCallback key_callback_;
    CharCallback char_callback_;
    WheelCallback wheel_callback_;
    PaintCallback paint_callback_;
    MouseMoveCallback mouse_move_callback_;
    MouseButtonCallback mouse_button_callback_;
    MouseButtonCallback middle_button_callback_;
    MouseButtonCallback right_button_callback_;
    DoubleClickCallback double_click_callback_;
    CursorCallback cursor_callback_;
    ImeCallback ime_callback_;
    std::map<UINT, TimerCallback> timer_callbacks_;
    LeaveCallback mouse_leave_callback_;
    bool tracking_leave_ = false;
    bool dark_titlebar_ = false;
    bool titlebar_theme_set_ = false;
    Color titlebar_bg_{};
    Color titlebar_border_{};
    Color titlebar_text_{};
    void apply_titlebar_theme();
    bool canvas_initialized_ = false;
};

}  // namespace jo::draftui::win32
