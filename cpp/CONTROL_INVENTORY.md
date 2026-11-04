# JO-DraftUI C++ 展厅控件清单

这份清单先回答“应该有什么”，再谈“有没有对齐”。

## 三个来源，不能混成一个

1. **JOL 原生控件**
   来源：`JO-Launcher-Pro/platform/win32/app.cpp`
   目标：C++ 展厅必须覆盖，尺寸、状态、命中区域以 JOL 为准。

2. **Flutter 版 JO-DraftUI 控件**
   来源：`JO-DraftUI/flutter/lib/src/widgets/`
   目标：C++ 版应提供对应控件，但与 JOL 视觉不一致时单独标注为 Flutter 扩展，不强行改成 JOL 样式。

3. **页面级组合**
   例如下载页分类导航、版本列表卡片、运行卡片、任务进度。
   这些是用基础控件拼出来的页面内容，不是控件库条目，不进入展厅。
   展厅只展示控件本身及其状态，不复制 JOL 页面布局。

## JOL 原生控件

| 控件 | JOL 来源 | C++ 状态 |
| --- | --- | --- |
| 按钮 | `paint_button` | 已有基础样式，缺少尺寸和状态变体 |
| 导航行 | `paint_nav` | 已有 |
| 分类行 | `paint_nav_category` | 已有基础样式，缺少展开/收起 |
| 左侧栏 | `paint_rail` | 已有布局，缺少完整状态 |
| 分栏 | `paint_splitter` | 可拖动，缺少双击复位和完整命中区 |
| 滚动条 | `paint_scrollbar` | 可拖动，缺少与内容滚动同步 |
| 输入框 | `paint_input` | 基础编辑可用，缺少选区、剪贴板和 IME |
| 字段分区 | `paint_sections` | 近似实现，未逐项核对几何 |
| 图标 | `draw_icon` | 已有 |
| 面板/文本 | `draw_panel` / `draw_text` | 已有 |
| 对话框 | 通用遮罩、标题、字段、按钮 | 已有基础示例 |

## Flutter 版已有、C++ 版还缺的控件

| Flutter 控件 | 用途 | C++ 状态 |
| --- | --- | --- |
| `JoIconAction` | 图标动作 + tooltip | 未做 |
| `JoTooltip` | 悬停提示 | 未做 |
| `JoMenuBar` / `JoMenu` / `JoMenuItem` | 菜单栏和弹出菜单 | 只有菜单文字 |
| `JoPopupPanel` / `JoPopupRow` | 弹出面板 | 未做 |
| `JoMarkdown` | Markdown 文本 | 未做 |
| `JoTextInput` | 完整输入框 | 只有基础编辑 |
| `JoTap` | 点击/焦点行为 | 未单独抽象 |
| `JoSplitter` | 分栏拖动 | 已有拖动，缺少双击复位 |
| `JoStatusBar` | 状态栏 | 已有 |
| `JoSection` / `JoField` | 分区/字段 | 已有近似实现 |
| `JoSegment` / `JoToggle` | 分段/开关 | 已有 |
| `JoDropdown` | 下拉 | 已有 |
| `JoListRow` | 列表行 | 已有 |
| `JoButton` | 按钮 | 已有基础样式 |

## 每个控件必须覆盖的状态

- 普通
- hover
- 按下
- 聚焦
- 选中
- 禁用
- 加载/进行中
- 错误

## 完成标准

一个控件只有同时满足以下条件，才能从“近似”改为“完成”：

1. 使用 JOL 或 Flutter 的同一份几何/颜色来源，而不是手工抄数值。
2. 与原始控件在相同 DPI、相同内容原点下做像素对比。
3. 所有状态都有截图基线。
4. 命中区域有自动化点击验证。
5. 未验证的状态明确标为未验证，不能写成“已完成”。

## 当前结论

展厅是控件库，不是 JOL 页面缩略图。页面级组合不进入展厅；JOL 原生控件和 Flutter 控件按清单逐项实现。当前仍缺少若干基础控件和完整状态，具体以表格中的“未做/近似”为准。
