# C++ 适配

目标：Windows 桌面，**C++ / Win32 / Direct2D**。

状态：基础骨架已可构建。当前使用 GCC 16.2 / MinGW-w64 + Direct2D；
已包含 token、主题、D2D 画布、Win32 窗口、按钮和列表行。

## 约束

- 令牌以 `spec/tokens.json` 为准，不在这里另定一套值
- 字体与图标使用 `flutter/assets/fonts/` 下的同一批文件：
  - NotoSansSC-VF.ttf / NotoSerifSC-VF.ttf / NotoSansMono-VF.ttf（DirectWrite 加载）
  - remixicon.ttf（图标字体）
- 交互状态按 `spec/states.md` 实现，与 Flutter 版一致
- 许可文件随实现保留

## 构建

```powershell
cmake -S . -B build/gcc -G Ninja `
  -DCMAKE_MAKE_PROGRAM=.../ninja.exe `
  -DCMAKE_C_COMPILER=.../gcc.exe `
  -DCMAKE_CXX_COMPILER=.../g++.exe `
  -DCMAKE_RC_COMPILER=.../windres.exe
cmake --build build/gcc
```

示例在 `build/gcc/example/win32/jo_draftui_example.exe`。

## 后续

- 第一阶段的输入框、滚动区、分栏、菜单和弹出层
- 用同一套 `spec/` 与 Flutter 适配做行为对照
- 逐步接回 `JO-Launcher`，删除启动器里重复的 D2D 基元
