# 规范

这里是与实现无关的部分：颜色、间距、形状、字体、状态。

**规则：`spec/` 是唯一事实来源。**

- 任何平台适配（目前是 `flutter/`）都必须与这里的值一致
- 改颜色、间距、圆角，先改这里，再改所有实现
- `flutter/test/tokens_spec_test.dart` 会读取 `tokens.json`，逐项和 Dart 常量比对；
  两边不一致，测试就会失败

## tokens.json

| 键 | 内容 |
|---|---|
| `shape` | 圆角、线宽 |
| `space` | 间距尺度 s1–s7 |
| `fonts` | 字体族名 |
| `type` | 字号、行高、字距 |
| `palette.dark` | 暗 · 机器 |
| `palette.light` | 明 · 纸 |

颜色写 `#RRGGBB`，不透明。

## 新增一个令牌

1. 在 `tokens.json` 里加
2. 在 `flutter/lib/src/tokens.dart` 里加对应常量
3. 在 `flutter/test/tokens_spec_test.dart` 里加比对
4. 如果它有默认控件用法，更新 `CONTROLS.md`

## 状态

交互状态的定义见 [states.md](./states.md)。各平替实现必须表现一致——
同样的悬停色、按下色、禁用色，不能一个平台一个样。

## 字体在 Flutter 里的名字

规范里写的是逻辑族名（`NotoSansSC` 等）。Flutter 会把包内字体暴露为
`packages/jo_draftui/<族名>`，这层前缀只存在于实现里，不改规范。
