# JO-DraftUI

中文 | [English](./README_en_US.md)

> 纸与线。

一套为桌面工具而作的界面语言。

本仓库描述**设计语言本身**；Flutter 只是第一种适配实现，放在 `flutter/`。

- **平**：没有漂浮、没有阴影，所有东西贴在同一个平面上
- **直**：几乎直角，2px
- **细**：1px 的线分隔一切——栏、行、块
- **静**：颜色克制，强调色只给“当前”和“可操作”

明暗两套：**纸**（暖白、宋体正文），**机器**（炭黑、等宽数字）。
两种都来自同一套线条和间距。

## 仓库结构

```
spec/       与实现无关：颜色、间距、形状、字体、状态（唯一事实来源）
flutter/    Flutter 适配（Dart 包 jo_draftui）
cpp/        C++ / Win32 / Direct2D 适配
example/    展厅
```

## 用法

```yaml
dependencies:
  jo_draftui:
    path: ../JO-DraftUI/flutter
```

```dart
import 'package:jo_draftui/jo_draftui.dart';

JoTheme(
  settings: const JoSettings(),
  palette: JoPalette.dark,
  child: JoButton(text: '保存', onTap: save),
)
```

字体已随包提供，使用方不用再声明：

- `NotoSansSC`（思源黑体，可变字重）
- `NotoSerifSC`（思源宋体）
- `NotoSansMono`（等宽，只给代码用）

Flutter 会把包内字体暴露为 `packages/jo_draftui/<family>`；`JoFonts` 里已经写好了，
业务代码不需要关心前缀。

需要换字体时，在 `JoSettings` 里填自己的 family：

```dart
JoTheme(
  settings: const JoSettings(sansFamily: 'MySans', serifFamily: 'MySerif'),
  ...
)
```

## 已有控件

| 控件 | 说明 |
|---|---|
| `JoTap` | hover / 按下状态基元 |
| `JoTooltip` | 自绘悬停提示 |
| `JoButton` | 细线小按钮 |
| `JoIconAction` | 图标动作（带必填提示） |
| `JoSegment` | 分段选择 |
| `JoSection` | 分区容器（标题 + 细线框） |
| `JoField` | 设置行（左标签右控件） |
| `JoToggle` | 文字开关 |
| `JoSplitter` | 可拖动分栏 |
| `JoMenuBar` | 菜单栏（下拉、分隔线、禁用项） |
| `JoDropdown` | 下拉选择 |
| `JoListRow` | 列表行（悬停、选中、副标题） |
| `JoStatusBar` | 状态栏 |
| `JoPopupPanel` / `JoPopupRow` | 弹出面板与面板行 |

## 原则

**按需生长**：只被一个项目用到的控件留在项目里；第二个项目要用时再抽进来。

## 许可

Apache License 2.0。见 [LICENSE](./LICENSE) 与 [NOTICE](./NOTICE)。

图标来自 Remix Icon（Remix Icon License v1.0），许可全文在
`assets/licenses/RemixIcon-License-1.0.txt`。
