add_executable(jo_draftui_example WIN32 main.cpp resources.rc)
target_link_libraries(jo_draftui_example PRIVATE jo_draftui)

target_compile_definitions(jo_draftui_example PRIVATE UNICODE _UNICODE NOMINMAX)

if (MSVC)
    target_compile_options(jo_draftui_example PRIVATE /utf-8)
    set_property(TARGET jo_draftui_example PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif (MINGW)
    target_link_options(jo_draftui_example PRIVATE -municode)
endif()
