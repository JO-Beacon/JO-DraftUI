cmake_minimum_required(VERSION 3.20)

project(jo_draftui_cpp LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT WIN32)
    message(FATAL_ERROR "The current JO-DraftUI C++ adapter targets Win32/Direct2D.")
endif()

add_library(jo_draftui STATIC
    src/d2d/canvas_d2d.cpp
    src/controls/basic.cpp
    src/controls/kit.cpp
    src/controls/behavior.cpp
    src/controls/markdown.cpp
    src/win32/window.cpp
)

target_include_directories(jo_draftui PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_definitions(jo_draftui PUBLIC
    UNICODE
    _UNICODE
    NOMINMAX
)

target_link_libraries(jo_draftui PUBLIC
    d2d1
    dwrite
    dwmapi
    imm32
    user32
)

if (MSVC)
    target_compile_options(jo_draftui PRIVATE /W4 /permissive- /utf-8)
    set_property(TARGET jo_draftui PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    target_compile_options(jo_draftui PRIVATE -Wall -Wextra)
endif()
