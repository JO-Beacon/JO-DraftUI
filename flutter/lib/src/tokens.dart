import 'package:flutter/widgets.dart';

/// 设计令牌：颜色、字体、间距、形状。
/// 观感不继承任何现成设计系统——颜色与排版全部自己定。
@immutable
class JoPalette {
  const JoPalette({
    required this.bg,
    required this.surface,
    required this.border,
    required this.text,
    required this.secondary,
    required this.icon,
    required this.accent,
    required this.isDark,
  });

  final Color bg;
  final Color surface;
  final Color border;
  final Color text;
  final Color secondary;

  /// 图标与结构件（分隔、表头那类）专用的一档。
  final Color icon;

  final Color accent;
  final bool isDark;

  /// 暗 · 机器
  static const JoPalette dark = JoPalette(
    bg: Color(0xFF0E0F12),
    surface: Color(0xFF16181D),
    border: Color(0xFF262A31),
    text: Color(0xFFE6E3DC),
    secondary: Color(0xFF8A8F98),
    icon: Color(0xFF8A8F98),
    accent: Color(0xFF8AA0B8),
    isDark: true,
  );

  /// 明 · 纸
  static const JoPalette light = JoPalette(
    bg: Color(0xFFF5F2EC),
    surface: Color(0xFFFBFAF6),
    border: Color(0xFFE0DAD0),
    text: Color(0xFF1C1B19),
    secondary: Color(0xFF6E6A62),
    icon: Color(0xFF6E6A62),
    accent: Color(0xFF4A6B8A),
    isDark: false,
  );

  /// 明暗过渡用：逐字段插值。
  /// isDark 不做插值，过中点时翻过来（它只当开关用）。
  static JoPalette lerp(JoPalette a, JoPalette b, double t) {
    if (t <= 0) return a;
    if (t >= 1) return b;
    return JoPalette(
      bg: Color.lerp(a.bg, b.bg, t)!,
      surface: Color.lerp(a.surface, b.surface, t)!,
      border: Color.lerp(a.border, b.border, t)!,
      text: Color.lerp(a.text, b.text, t)!,
      secondary: Color.lerp(a.secondary, b.secondary, t)!,
      icon: Color.lerp(a.icon, b.icon, t)!,
      accent: Color.lerp(a.accent, b.accent, t)!,
      isDark: t < 0.5 ? a.isDark : b.isDark,
    );
  }
}

/// 字体族。
///
/// 三个：思源黑体（Noto Sans SC）、思源宋体（Noto Serif SC）、
/// 等宽（Noto Sans Mono，只给代码用）。
///
/// 字体文件由使用方在 pubspec.yaml 里声明，family 名必须与这里一致。
class JoFonts {
  /// 包内字体在 Flutter 里会带 [packages/<包名>/] 前缀。
  static const String _pkg = 'packages/jo_draftui/';

  /// 逻辑族名（spec 里写的就是它）在 Flutter 里的实际名字。
  static const String sans = '${_pkg}NotoSansSC';
  static const String serif = '${_pkg}NotoSerifSC';
  static const String mono = '${_pkg}NotoSansMono';

  /// 数字等宽。数字宽度一致、列不会跳。字体不支持就忽略。
  static const List<FontFeature> tabular = <FontFeature>[
    FontFeature.tabularFigures(),
  ];

  /// 界面 / 标题（黑体）
  static const TextStyle ui = TextStyle(fontFamily: sans);

  /// 叙事正文（默认宋体；字号 / 字体由设置决定）。
  static const TextStyle body = TextStyle(
    fontFamily: serif,
    height: 1.75,
    fontSize: 16,
    decoration: TextDecoration.none,
  );

  /// 元数据 / 标签（小一号的黑体 + 数字等宽）
  static const TextStyle meta = TextStyle(
    fontFamily: sans,
    fontFeatures: tabular,
    height: 1.4,
    fontSize: 12,
    letterSpacing: 0.4,
  );

  /// 代码（行内代码 / 代码块）。
  static const TextStyle code = TextStyle(
    fontFamily: mono,
    fontFamilyFallback: <String>[sans],
    fontFeatures: tabular,
    height: 1.5,
  );
}

/// 间距尺度。
class JoSpace {
  static const double s1 = 4;
  static const double s2 = 8;
  static const double s3 = 12;
  static const double s4 = 16;
  static const double s5 = 24;
  static const double s6 = 32;
  static const double s7 = 48;
}

/// 形状：直角 / 极小圆角，细线，零阴影。
class JoShape {
  static const double radius = 2;
  static const BorderRadius br = BorderRadius.all(Radius.circular(radius));
  static Border border(Color color) => Border.all(color: color, width: 1);
  static BorderSide side(Color color) => BorderSide(color: color, width: 1);
}

/// 用户可配置项。
@immutable
class JoSettings {
  const JoSettings({
    this.isDark = true,
    this.bodySize = 16,
    this.serifBody = true,
    this.followSystem = false,
    this.sansFamily,
    this.serifFamily,
    this.monoFamily,
  });

  final bool isDark;

  /// 字体族覆盖。留空 = 用包内字体（`packages/jo_draftui/...`）。
  ///
  /// 使用方在自己的 pubspec 里声明了字体时，把 family 名填进来即可，
  /// 不用改任何控件代码。
  final String? sansFamily;
  final String? serifFamily;
  final String? monoFamily;

  /// 跟随系统明暗。开了之后实际用哪一套由平台亮度决定。
  final bool followSystem;

  /// 叙事正文字号。
  final double bodySize;

  /// 正文用宋体（true）还是黑体（false）。
  final bool serifBody;

  String get uiFamily => sansFamily ?? JoFonts.sans;

  String get bodyFamily =>
      serifBody ? (serifFamily ?? JoFonts.serif) : uiFamily;

  String get codeFamily => monoFamily ?? JoFonts.mono;

  TextStyle bodyStyle(Color color) => JoFonts.body.copyWith(
        fontFamily: bodyFamily,
        fontSize: bodySize,
        color: color,
      );

  JoSettings copyWith({
    bool? isDark,
    double? bodySize,
    bool? serifBody,
    bool? followSystem,
    String? sansFamily,
    String? serifFamily,
    String? monoFamily,
  }) {
    return JoSettings(
      isDark: isDark ?? this.isDark,
      bodySize: bodySize ?? this.bodySize,
      serifBody: serifBody ?? this.serifBody,
      followSystem: followSystem ?? this.followSystem,
      sansFamily: sansFamily ?? this.sansFamily,
      serifFamily: serifFamily ?? this.serifFamily,
      monoFamily: monoFamily ?? this.monoFamily,
    );
  }
}

/// 把令牌塞进 Theme，供子组件读取。
@immutable
class JoTheme extends InheritedWidget {
  const JoTheme({
    super.key,
    required this.settings,
    required this.palette,
    required super.child,
  });

  final JoSettings settings;

  /// 当前**实际**用的调色板。明暗切换时这里是插值中的值，不是目标值。
  final JoPalette palette;

  static JoTheme _scope(BuildContext context) {
    final JoTheme? t = context.dependOnInheritedWidgetOfExactType<JoTheme>();
    assert(t != null, 'JoTheme 未挂载');
    return t!;
  }

  static JoPalette of(BuildContext context) => _scope(context).palette;

  static JoSettings settingsOf(BuildContext context) =>
      _scope(context).settings;

  /// 解析过覆盖项的文本样式；控件一律走这里，不直接用 [JoFonts] 常量。
  static TextStyle ui(BuildContext context) =>
      JoFonts.ui.copyWith(fontFamily: settingsOf(context).uiFamily);

  static TextStyle meta(BuildContext context) =>
      JoFonts.meta.copyWith(fontFamily: settingsOf(context).uiFamily);

  static TextStyle body(BuildContext context) =>
      JoFonts.body.copyWith(fontFamily: settingsOf(context).bodyFamily);

  static TextStyle code(BuildContext context) =>
      JoFonts.code.copyWith(fontFamily: settingsOf(context).codeFamily);

  @override
  bool updateShouldNotify(JoTheme oldWidget) =>
      settings != oldWidget.settings || palette != oldWidget.palette;
}
