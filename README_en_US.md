# JO-DraftUI

[中文](./README.md) | English

> Paper and line.

An interface language for desktop tools.

This repository describes the **design language itself**; Flutter is the first binding, under `flutter/`.

- **Flat** — nothing floats, no shadows; everything sits on the same plane
- **Square** — almost right angles, 2px
- **Thin** — 1px lines divide everything: columns, rows, blocks
- **Quiet** — restrained color; the accent appears only on "current" and "actionable"

Two palettes: **Paper** (warm white, serif body text) and **Machine** (charcoal, tabular figures).
Both share the same lines and spacing.

## Layout

```
spec/       Implementation-neutral: color, spacing, shape, type, states
flutter/    Flutter binding (Dart package jo_draftui)
example/    Gallery
```

## Usage

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
  child: JoButton(text: 'Save', onTap: save),
)
```

Fonts ship with the package; consumers do not declare them:

- `NotoSansSC` (Noto Sans SC, variable weight)
- `NotoSerifSC` (Noto Serif SC)
- `NotoSansMono` (monospace, code only)

Flutter exposes package fonts as `packages/jo_draftui/<family>`; `JoFonts` already
carries those names, so application code does not deal with the prefix.

To use different fonts, set the family names on `JoSettings`:

```dart
JoTheme(
  settings: const JoSettings(sansFamily: 'MySans', serifFamily: 'MySerif'),
  ...
)
```

## Widgets

| Widget | Description |
|---|---|
| `JoTap` | hover / pressed state primitive |
| `JoTooltip` | self-drawn hover hint |
| `JoButton` | thin-line button |
| `JoIconAction` | icon action (tooltip required) |
| `JoSegment` | segmented selection |
| `JoSection` | section container (title + hairline frame) |
| `JoField` | settings row (label left, control right) |
| `JoToggle` | text toggle |
| `JoSplitter` | draggable column divider |
| `JoMenuBar` | menu bar (flyouts, separators, disabled items) |
| `JoDropdown` | dropdown selection |
| `JoListRow` | list row (hover, selected, subtitle) |
| `JoStatusBar` | status bar |
| `JoPopupPanel` / `JoPopupRow` | popup panel and rows |

## Principle

**Grow on demand** — a widget used by only one project stays in that project; it moves into
this package when a second project needs it.

## License

Apache License 2.0. See [LICENSE](./LICENSE) and [NOTICE](./NOTICE).

Icons are from Remix Icon (Remix Icon License v1.0); the full text is in
`assets/licenses/RemixIcon-License-1.0.txt`.
