#include "jo_draftui/win32/window.hpp"

#include <algorithm>

#include <windowsx.h>
#include <dwmapi.h>
#include <imm.h>

namespace jo::draftui::win32 {
namespace {
constexpr wchar_t kWindowClass[] = L"JoDraftUiWindow";

COLORREF to_colorref(Color color) {
    const auto channel = [](float value) {
        return static_cast<BYTE>(std::clamp(value, 0.0f, 1.0f) * 255.0f);
    };
    return RGB(channel(color.r), channel(color.g), channel(color.b));
}

#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#endif
#ifndef DWMSBT_NONE
#define DWMSBT_NONE 1
#endif
}

Window::Window() = default;
Window::~Window() {
    if (window_ != nullptr) {
        SetWindowLongPtrW(window_, GWLP_USERDATA, 0);
        DestroyWindow(window_);
        window_ = nullptr;
    }
}

bool Window::create(const std::wstring& title, int width, int height) {
    HINSTANCE instance = GetModuleHandleW(nullptr);
    WNDCLASSEXW cls{};
    cls.cbSize = sizeof(cls);
    cls.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    cls.lpfnWndProc = &Window::window_proc;
    cls.hInstance = instance;
    cls.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    cls.hbrBackground = nullptr;
    cls.lpszClassName = kWindowClass;
    if (RegisterClassExW(&cls) == 0 &&
        GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        return false;
    }

    window_ = CreateWindowExW(
        0, kWindowClass, title.c_str(), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height, nullptr, nullptr,
        instance, this);
    if (window_ == nullptr) return false;
    apply_titlebar_theme();
    ShowWindow(window_, SW_SHOW);
    UpdateWindow(window_);
    return true;
}

int Window::run() {
    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return static_cast<int>(message.wParam);
}

void Window::invalidate() {
    if (window_ != nullptr) InvalidateRect(window_, nullptr, FALSE);
}

HWND Window::hwnd() const {
    return window_;
}

float Window::scale() const {
    return dpi_scale(window_);
}

void Window::set_dark_titlebar(bool dark) {
    dark_titlebar_ = dark;
    if (window_ != nullptr) {
        const BOOL value = dark ? TRUE : FALSE;
        DwmSetWindowAttribute(window_, 20, &value, sizeof(value));
    }
}

void Window::set_titlebar_theme(bool dark, Color background, Color border,
                                Color text) {
    dark_titlebar_ = dark;
    titlebar_bg_ = background;
    titlebar_border_ = border;
    titlebar_text_ = text;
    titlebar_theme_set_ = true;
    apply_titlebar_theme();
}

void Window::apply_titlebar_theme() {
    if (window_ == nullptr) return;
    const BOOL dark = dark_titlebar_ ? TRUE : FALSE;
    DwmSetWindowAttribute(window_, 20, &dark, sizeof(dark));
    if (titlebar_theme_set_) {
        const COLORREF caption = to_colorref(titlebar_bg_);
        const COLORREF border = to_colorref(titlebar_border_);
        const COLORREF text = to_colorref(titlebar_text_);
        DwmSetWindowAttribute(window_, 35, &caption, sizeof(caption));
        DwmSetWindowAttribute(window_, 34, &border, sizeof(border));
        DwmSetWindowAttribute(window_, 36, &text, sizeof(text));
    }
    const int backdrop = DWMSBT_NONE;
    DwmSetWindowAttribute(window_, DWMWA_SYSTEMBACKDROP_TYPE, &backdrop,
                          sizeof(backdrop));
}

void Window::set_canvas_init_callback(CanvasInitCallback callback) {
    canvas_init_callback_ = std::move(callback);
}

void Window::set_key_callback(KeyCallback callback) {
    key_callback_ = std::move(callback);
}

void Window::set_char_callback(CharCallback callback) {
    char_callback_ = std::move(callback);
}

void Window::set_wheel_callback(WheelCallback callback) {
    wheel_callback_ = std::move(callback);
}

void Window::set_paint_callback(PaintCallback callback) {
    paint_callback_ = std::move(callback);
}

void Window::set_mouse_move_callback(MouseMoveCallback callback) {
    mouse_move_callback_ = std::move(callback);
}

void Window::set_mouse_button_callback(MouseButtonCallback callback) {
    mouse_button_callback_ = std::move(callback);
}

void Window::set_middle_button_callback(MouseButtonCallback callback) {
    middle_button_callback_ = std::move(callback);
}

void Window::set_right_button_callback(MouseButtonCallback callback) {
    right_button_callback_ = std::move(callback);
}

void Window::set_double_click_callback(DoubleClickCallback callback) {
    double_click_callback_ = std::move(callback);
}

void Window::set_cursor_callback(CursorCallback callback) {
    cursor_callback_ = std::move(callback);
}

void Window::set_ime_callback(ImeCallback callback) {
    ime_callback_ = std::move(callback);
}

void Window::set_ime_position(const Rect& caret) {
    if (window_ == nullptr) return;
    HIMC context = ImmGetContext(window_);
    if (context == nullptr) return;
    POINT point{static_cast<LONG>(caret.left), static_cast<LONG>(caret.top)};
    ClientToScreen(window_, &point);
    COMPOSITIONFORM composition{};
    composition.dwStyle = CFS_POINT;
    composition.ptCurrentPos = point;
    ImmSetCompositionWindow(context, &composition);
    CANDIDATEFORM candidate{};
    candidate.dwStyle = CFS_CANDIDATEPOS;
    candidate.ptCurrentPos = point;
    ImmSetCandidateWindow(context, &candidate);
    LOGFONTW font{};
    font.lfHeight = -MulDiv(12, static_cast<int>(GetDpiForWindow(window_)), 96);
    font.lfCharSet = DEFAULT_CHARSET;
    wcscpy_s(font.lfFaceName, L"Noto Sans SC");
    ImmSetCompositionFontW(context, &font);
    ImmReleaseContext(window_, context);
}

void Window::set_timer(UINT id, UINT milliseconds, TimerCallback callback) {
    timer_callbacks_[id] = std::move(callback);
    if (window_ != nullptr) SetTimer(window_, id, milliseconds, nullptr);
}

void Window::kill_timer(UINT id) {
    timer_callbacks_.erase(id);
    if (window_ != nullptr) KillTimer(window_, id);
}

void Window::set_mouse_leave_callback(LeaveCallback callback) {
    mouse_leave_callback_ = std::move(callback);
}

float Window::dpi_scale(HWND window) const {
    if (window == nullptr) return 1.0f;
    const UINT dpi = GetDpiForWindow(window);
    return dpi == 0 ? 1.0f : static_cast<float>(dpi) / 96.0f;
}

void Window::ensure_canvas(HWND window, UINT width, UINT height) {
    if (width == 0 || height == 0) return;
    if (!canvas_) {
        try {
            canvas_ = std::make_unique<D2DCanvas>(window);
        } catch (...) {
            canvas_.reset();
            return;
        }
    }
    if (canvas_width_ == width && canvas_height_ == height &&
        canvas_->valid()) {
        return;
    }
    if (!canvas_->resize(width, height)) {
        canvas_.reset();
        canvas_width_ = 0;
        canvas_height_ = 0;
        canvas_initialized_ = false;
        return;
    }
    canvas_width_ = width;
    canvas_height_ = height;
    if (!canvas_initialized_ && canvas_) {
        canvas_initialized_ = true;
        if (canvas_init_callback_) canvas_init_callback_(*canvas_);
    }
}

void Window::paint(HWND window) {
    RECT client{};
    GetClientRect(window, &client);
    const UINT width = static_cast<UINT>(std::max<LONG>(0, client.right));
    const UINT height = static_cast<UINT>(std::max<LONG>(0, client.bottom));
    ensure_canvas(window, width, height);
    if (!canvas_ || !canvas_->begin_draw()) return;

    if (paint_callback_) {
        paint_callback_(*canvas_,
                        Rect{0.0f, 0.0f, static_cast<float>(width),
                             static_cast<float>(height)},
                        dpi_scale(window));
    } else {
        canvas_->clear(Color::from_argb(0xFF0E0F12u));
    }
    canvas_->end_draw();
}

LRESULT CALLBACK Window::window_proc(HWND window, UINT message, WPARAM wparam,
                                     LPARAM lparam) {
    Window* self = reinterpret_cast<Window*>(
        GetWindowLongPtrW(window, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        auto* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
        self = static_cast<Window*>(create->lpCreateParams);
        SetWindowLongPtrW(window, GWLP_USERDATA,
                          reinterpret_cast<LONG_PTR>(self));
    }
    return self != nullptr
               ? self->handle_message(window, message, wparam, lparam)
               : DefWindowProcW(window, message, wparam, lparam);
}

LRESULT Window::handle_message(HWND window, UINT message, WPARAM wparam,
                               LPARAM lparam) {
    switch (message) {
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT paint_struct{};
            BeginPaint(window, &paint_struct);
            paint(window);
            EndPaint(window, &paint_struct);
            return 0;
        }
        case WM_SIZE:
            invalidate();
            return 0;
        case WM_DPICHANGED: {
            const RECT* suggested = reinterpret_cast<RECT*>(lparam);
            SetWindowPos(window, nullptr, suggested->left, suggested->top,
                         suggested->right - suggested->left,
                         suggested->bottom - suggested->top,
                         SWP_NOZORDER | SWP_NOACTIVATE);
            invalidate();
            return 0;
        }
        case WM_MOUSEMOVE: {
            if (!tracking_leave_) {
                TRACKMOUSEEVENT track{sizeof(track), TME_LEAVE, window, 0};
                TrackMouseEvent(&track);
                tracking_leave_ = true;
            }
            if (mouse_move_callback_) {
                mouse_move_callback_(static_cast<float>(GET_X_LPARAM(lparam)),
                                     static_cast<float>(GET_Y_LPARAM(lparam)));
            }
            return 0;
        }
        case WM_MOUSELEAVE:
            tracking_leave_ = false;
            if (mouse_leave_callback_) mouse_leave_callback_();
            return 0;
        case WM_TIMER: {
            const auto it = timer_callbacks_.find(static_cast<UINT>(wparam));
            if (it != timer_callbacks_.end() && it->second) {
                it->second();
                return 0;
            }
            break;
        }
        case WM_KEYDOWN:
            if (key_callback_ && key_callback_(static_cast<UINT>(wparam))) {
                return 0;
            }
            break;
        case WM_CHAR:
            if (char_callback_ &&
                char_callback_(static_cast<wchar_t>(wparam))) {
                return 0;
            }
            break;
        case WM_MOUSEWHEEL:
            if (wheel_callback_) {
                wheel_callback_(GET_WHEEL_DELTA_WPARAM(wparam));
                return 0;
            }
            break;
        case WM_SETCURSOR:
            if (LOWORD(lparam) == HTCLIENT && cursor_callback_) {
                SetCursor(cursor_callback_());
                return TRUE;
            }
            break;
        case WM_IME_COMPOSITION: {
            if (ime_callback_) {
                HIMC context = ImmGetContext(window);
                if (context != nullptr) {
                    if (lparam & GCS_RESULTSTR) {
                        const LONG bytes = ImmGetCompositionStringW(
                            context, GCS_RESULTSTR, nullptr, 0);
                        if (bytes > 0) {
                            std::wstring text(
                                static_cast<std::size_t>(bytes) /
                                    sizeof(wchar_t),
                                L'\0');
                            const LONG copied = ImmGetCompositionStringW(
                                context, GCS_RESULTSTR, text.data(), bytes);
                            if (copied > 0) {
                                text.resize(static_cast<std::size_t>(copied) /
                                            sizeof(wchar_t));
                                ime_callback_(text, true);
                            }
                        }
                    } else if (lparam & GCS_COMPSTR) {
                        const LONG bytes = ImmGetCompositionStringW(
                            context, GCS_COMPSTR, nullptr, 0);
                        if (bytes > 0) {
                            std::wstring text(
                                static_cast<std::size_t>(bytes) /
                                    sizeof(wchar_t),
                                L'\0');
                            const LONG copied = ImmGetCompositionStringW(
                                context, GCS_COMPSTR, text.data(), bytes);
                            if (copied > 0) {
                                text.resize(static_cast<std::size_t>(copied) /
                                            sizeof(wchar_t));
                                ime_callback_(text, false);
                            }
                        }
                    }
                    ImmReleaseContext(window, context);
                }
            }
            break;
        }
        case WM_LBUTTONDOWN:
            SetCapture(window);
            if (mouse_button_callback_) {
                mouse_button_callback_(static_cast<float>(GET_X_LPARAM(lparam)),
                                       static_cast<float>(GET_Y_LPARAM(lparam)),
                                       true);
            }
            return 0;
        case WM_LBUTTONUP:
            ReleaseCapture();
            if (mouse_button_callback_) {
                mouse_button_callback_(static_cast<float>(GET_X_LPARAM(lparam)),
                                       static_cast<float>(GET_Y_LPARAM(lparam)),
                                       false);
            }
            return 0;
        case WM_LBUTTONDBLCLK:
            if (double_click_callback_) {
                double_click_callback_(
                    static_cast<float>(GET_X_LPARAM(lparam)),
                    static_cast<float>(GET_Y_LPARAM(lparam)));
            }
            return 0;
        case WM_MBUTTONDOWN:
            SetCapture(window);
            if (middle_button_callback_) {
                middle_button_callback_(
                    static_cast<float>(GET_X_LPARAM(lparam)),
                    static_cast<float>(GET_Y_LPARAM(lparam)), true);
            }
            return 0;
        case WM_MBUTTONUP:
            ReleaseCapture();
            if (middle_button_callback_) {
                middle_button_callback_(
                    static_cast<float>(GET_X_LPARAM(lparam)),
                    static_cast<float>(GET_Y_LPARAM(lparam)), false);
            }
            return 0;
        case WM_RBUTTONDOWN:
            SetCapture(window);
            if (right_button_callback_) {
                right_button_callback_(
                    static_cast<float>(GET_X_LPARAM(lparam)),
                    static_cast<float>(GET_Y_LPARAM(lparam)), true);
            }
            return 0;
        case WM_RBUTTONUP:
            ReleaseCapture();
            if (right_button_callback_) {
                right_button_callback_(
                    static_cast<float>(GET_X_LPARAM(lparam)),
                    static_cast<float>(GET_Y_LPARAM(lparam)), false);
            }
            return 0;
        case WM_CLOSE:
            DestroyWindow(window);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            break;
    }
    return DefWindowProcW(window, message, wparam, lparam);
}

}  // namespace jo::draftui::win32
