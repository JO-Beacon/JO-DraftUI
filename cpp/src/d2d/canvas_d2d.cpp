#include "jo_draftui/canvas.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

#include <d2d1.h>
#include <dwrite.h>
#include <dwrite_3.h>
#include <wrl/client.h>

namespace jo::draftui {
namespace {

using Microsoft::WRL::ComPtr;

D2D1_COLOR_F to_d2d(Color color) {
    return D2D1::ColorF(color.r, color.g, color.b, color.a);
}

D2D1_RECT_F to_d2d(const Rect& rect) {
    return D2D1::RectF(rect.left, rect.top, rect.right, rect.bottom);
}

DWRITE_FONT_WEIGHT to_dwrite(FontWeight weight) {
    switch (weight) {
        case FontWeight::semi_bold: return DWRITE_FONT_WEIGHT_SEMI_BOLD;
        case FontWeight::bold: return DWRITE_FONT_WEIGHT_BOLD;
        case FontWeight::normal: break;
    }
    return DWRITE_FONT_WEIGHT_NORMAL;
}

DWRITE_TEXT_ALIGNMENT to_dwrite(TextAlign align) {
    switch (align) {
        case TextAlign::center: return DWRITE_TEXT_ALIGNMENT_CENTER;
        case TextAlign::trailing: return DWRITE_TEXT_ALIGNMENT_TRAILING;
        case TextAlign::leading: break;
    }
    return DWRITE_TEXT_ALIGNMENT_LEADING;
}

const wchar_t* family_name(FontFamily family) {
    switch (family) {
        case FontFamily::serif: return tokens::serif_family;
        case FontFamily::mono: return tokens::mono_family;
        case FontFamily::icon: return L"Segoe UI Symbol";
        case FontFamily::sans: break;
    }
    return tokens::sans_family;
}

const wchar_t* loaded_family_name(FontFamily family) {
    switch (family) {
        case FontFamily::serif: return L"Noto Serif SC";
        case FontFamily::mono: return L"Noto Sans Mono";
        case FontFamily::icon: return L"remixicon";
        case FontFamily::sans: break;
    }
    return L"Noto Sans SC";
}

std::size_t family_index(FontFamily family) {
    switch (family) {
        case FontFamily::serif: return 1;
        case FontFamily::mono: return 2;
        case FontFamily::icon: return 3;
        case FontFamily::sans: break;
    }
    return 0;
}

}  // namespace

struct D2DCanvas::Impl {
    HWND window = nullptr;
    ComPtr<ID2D1Factory> factory;
    ComPtr<ID2D1HwndRenderTarget> target;
    ComPtr<ID2D1SolidColorBrush> brush;
    ComPtr<IDWriteFactory> write_factory;
    std::array<ComPtr<IDWriteFontCollection1>, 4> collections;
    std::array<ComPtr<IDWriteInMemoryFontFileLoader>, 4> loaders;
    std::array<std::vector<BYTE>, 4> font_data;
    std::vector<D2D1_MATRIX_3X2_F> transforms;
};

D2DCanvas::D2DCanvas(HWND window) : impl_(std::make_unique<Impl>()) {
    if (window == nullptr) throw std::runtime_error("D2DCanvas needs a window");
    impl_->window = window;

    const HRESULT factory_result = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED, impl_->factory.GetAddressOf());
    if (FAILED(factory_result)) {
        throw std::runtime_error("Could not create the Direct2D factory");
    }

    const HRESULT write_result = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(
            impl_->write_factory.GetAddressOf()));
    if (FAILED(write_result)) {
        throw std::runtime_error("Could not create the DirectWrite factory");
    }
}

D2DCanvas::~D2DCanvas() = default;

bool D2DCanvas::add_font_file(const std::filesystem::path& path,
                              FontFamily family) {
    if (!impl_->write_factory) return false;
    std::ifstream stream(path, std::ios::binary);
    if (!stream) return false;
    std::vector<BYTE> bytes{std::istreambuf_iterator<char>(stream),
                            std::istreambuf_iterator<char>()};
    if (bytes.empty()) return false;

    ComPtr<IDWriteFactory5> factory5;
    if (FAILED(impl_->write_factory.As(&factory5))) return false;
    ComPtr<IDWriteInMemoryFontFileLoader> loader;
    if (FAILED(factory5->CreateInMemoryFontFileLoader(
            loader.GetAddressOf()))) {
        return false;
    }
    if (FAILED(impl_->write_factory->RegisterFontFileLoader(loader.Get()))) {
        return false;
    }
    ComPtr<IDWriteFontFile> file;
    if (FAILED(loader->CreateInMemoryFontFileReference(
            impl_->write_factory.Get(), bytes.data(),
            static_cast<UINT32>(bytes.size()), nullptr,
            file.GetAddressOf()))) {
        return false;
    }

#if defined(__MINGW32__)
    ComPtr<IDWriteFontSetBuilder1> builder;
    if (FAILED(factory5->CreateFontSetBuilder(builder.GetAddressOf()))) {
        return false;
    }
#else
    ComPtr<IDWriteFontSetBuilder> builder_base;
    if (FAILED(factory5->CreateFontSetBuilder(
            builder_base.GetAddressOf()))) {
        return false;
    }
    ComPtr<IDWriteFontSetBuilder1> builder;
    if (FAILED(builder_base.As(&builder))) return false;
#endif
    if (FAILED(builder->AddFontFile(file.Get()))) return false;

    ComPtr<IDWriteFontSet> font_set;
    if (FAILED(builder->CreateFontSet(font_set.GetAddressOf()))) {
        return false;
    }
    ComPtr<IDWriteFontCollection1> collection;
    if (FAILED(factory5->CreateFontCollectionFromFontSet(
            font_set.Get(), collection.GetAddressOf()))) {
        return false;
    }

    const std::size_t index = family_index(family);
    impl_->font_data[index] = std::move(bytes);
    impl_->loaders[index] = loader;
    impl_->collections[index] = collection;
    return true;
}

bool D2DCanvas::resize(UINT width, UINT height) {
    if (width == 0 || height == 0) return false;
    if (impl_->target) {
        const HRESULT result = impl_->target->Resize(
            D2D1::SizeU(width, height));
        if (SUCCEEDED(result)) return true;
        impl_->target.Reset();
        impl_->brush.Reset();
    }

    const D2D1_RENDER_TARGET_PROPERTIES properties =
        D2D1::RenderTargetProperties();
    const D2D1_HWND_RENDER_TARGET_PROPERTIES window_properties =
        D2D1::HwndRenderTargetProperties(impl_->window,
                                         D2D1::SizeU(width, height));
    const HRESULT result = impl_->factory->CreateHwndRenderTarget(
        properties, window_properties, impl_->target.GetAddressOf());
    if (FAILED(result)) return false;

    impl_->target->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    impl_->target->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
    return SUCCEEDED(impl_->target->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::White), impl_->brush.GetAddressOf()));
}

bool D2DCanvas::begin_draw() {
    if (!impl_->target) return false;
    impl_->target->BeginDraw();
    return true;
}

void D2DCanvas::end_draw() {
    if (!impl_->target) return;
    const HRESULT result = impl_->target->EndDraw();
    if (result == D2DERR_RECREATE_TARGET) {
        impl_->target.Reset();
        impl_->brush.Reset();
    }
}

bool D2DCanvas::valid() const {
    return impl_->target != nullptr && impl_->brush != nullptr;
}

void D2DCanvas::clear(Color color) {
    if (!impl_->target) return;
    impl_->target->Clear(to_d2d(color));
}

void D2DCanvas::fill_rect(const Rect& rect, Color color) {
    if (!impl_->brush) return;
    impl_->brush->SetColor(to_d2d(color));
    impl_->target->FillRectangle(to_d2d(rect), impl_->brush.Get());
}

void D2DCanvas::fill_rounded_rect(const Rect& rect, float radius,
                                  Color color) {
    if (!impl_->brush) return;
    impl_->brush->SetColor(to_d2d(color));
    impl_->target->FillRoundedRectangle(
        D2D1::RoundedRect(to_d2d(rect), radius, radius), impl_->brush.Get());
}

void D2DCanvas::stroke_rect(const Rect& rect, Color color, float width) {
    if (!impl_->brush) return;
    impl_->brush->SetColor(to_d2d(color));
    impl_->target->DrawRectangle(to_d2d(rect), impl_->brush.Get(), width);
}

void D2DCanvas::stroke_rounded_rect(const Rect& rect, float radius, Color color,
                                    float width) {
    if (!impl_->brush) return;
    impl_->brush->SetColor(to_d2d(color));
    impl_->target->DrawRoundedRectangle(
        D2D1::RoundedRect(to_d2d(rect), radius, radius), impl_->brush.Get(),
        width);
}

void D2DCanvas::draw_line(float x1, float y1, float x2, float y2, Color color,
                          float width) {
    if (!impl_->brush) return;
    impl_->brush->SetColor(to_d2d(color));
    impl_->target->DrawLine(D2D1::Point2F(x1, y1), D2D1::Point2F(x2, y2),
                            impl_->brush.Get(), width);
}

void D2DCanvas::draw_text(const Rect& rect, std::wstring_view text,
                          const TextStyle& style) {
    if (!impl_->target || !impl_->brush || !impl_->write_factory ||
        text.empty()) {
        return;
    }

    const std::size_t index = family_index(style.family);
    IDWriteFontCollection* collection = impl_->collections[index].Get();
    const wchar_t* family = collection != nullptr
                                ? loaded_family_name(style.family)
                                : family_name(style.family);
    ComPtr<IDWriteTextFormat> format;
    const HRESULT format_result = impl_->write_factory->CreateTextFormat(
        family, collection, to_dwrite(style.weight),
        DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, style.size, L"",
        format.GetAddressOf());
    if (FAILED(format_result)) return;
    format->SetTextAlignment(to_dwrite(style.align));
    format->SetParagraphAlignment(style.center_vertical
                                      ? DWRITE_PARAGRAPH_ALIGNMENT_CENTER
                                      : DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
    format->SetWordWrapping(style.wrap ? DWRITE_WORD_WRAPPING_WRAP
                                       : DWRITE_WORD_WRAPPING_NO_WRAP);

    impl_->brush->SetColor(to_d2d(style.color));
    impl_->target->DrawTextW(
        text.data(), static_cast<UINT32>(text.size()), format.Get(),
        to_d2d(rect), impl_->brush.Get(), D2D1_DRAW_TEXT_OPTIONS_CLIP,
        DWRITE_MEASURING_MODE_NATURAL);
}

float D2DCanvas::measure_text(std::wstring_view text,
                              const TextStyle& style) {
    if (!impl_->write_factory || text.empty()) return 0.0f;
    const std::size_t index = family_index(style.family);
    IDWriteFontCollection* collection = impl_->collections[index].Get();
    const wchar_t* family = collection != nullptr
                                ? loaded_family_name(style.family)
                                : family_name(style.family);
    ComPtr<IDWriteTextFormat> format;
    if (FAILED(impl_->write_factory->CreateTextFormat(
            family, collection, to_dwrite(style.weight),
            DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, style.size,
            L"", format.GetAddressOf()))) {
        return 0.0f;
    }
    format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
    ComPtr<IDWriteTextLayout> layout;
    if (FAILED(impl_->write_factory->CreateTextLayout(
            text.data(), static_cast<UINT32>(text.size()), format.Get(),
            100000.0f, style.size * 2.0f, layout.GetAddressOf()))) {
        return 0.0f;
    }
    DWRITE_TEXT_METRICS metrics{};
    if (FAILED(layout->GetMetrics(&metrics))) return 0.0f;
    return metrics.widthIncludingTrailingWhitespace;
}

void D2DCanvas::push_clip(const Rect& rect) {
    if (!impl_->target) return;
    impl_->target->PushAxisAlignedClip(to_d2d(rect),
                                       D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}

void D2DCanvas::pop_clip() {
    if (!impl_->target) return;
    impl_->target->PopAxisAlignedClip();
}

void D2DCanvas::push_translate(float x, float y) {
    if (!impl_->target) return;
    D2D1_MATRIX_3X2_F transform{};
    impl_->target->GetTransform(&transform);
    impl_->transforms.push_back(transform);
    impl_->target->SetTransform(
        D2D1::Matrix3x2F::Translation(x, y) * transform);
}

void D2DCanvas::pop_transform() {
    if (!impl_->target || impl_->transforms.empty()) return;
    impl_->target->SetTransform(impl_->transforms.back());
    impl_->transforms.pop_back();
}

}  // namespace jo::draftui
