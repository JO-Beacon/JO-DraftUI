import 'package:flutter/widgets.dart';

import '../tokens.dart';
import 'tap.dart';
import 'tooltip.dart';

/// 一个只有图标的动作（工具栏、返回键）。
///
/// 悬停 / 按下：底色浮起（正文色 9% / 20%），图标从次要色亮到正文色。
/// 不做描边——那是“每个图标一个小盒子”，界面里别的工具栏不是那样。
///
/// **tip 是必填的**：界面上的图标一律要能悬停看出名字。
///
/// [inverted] 是实底强调色 + 底色图标，给主行动用（比如发送）。
class JoIconAction extends StatelessWidget {
  const JoIconAction({
    super.key,
    required this.icon,
    required this.tip,
    required this.onTap,
    this.size = 16,
    this.inverted = false,
  });

  final IconData icon;

  /// 悬停时浮出来的名字。必填，不许留空。
  final String tip;

  final VoidCallback? onTap;
  final double size;

  /// 反色：实底强调色 + 底色图标。主行动用。
  final bool inverted;

  @override
  Widget build(BuildContext context) {
    final JoPalette p = JoTheme.of(context);
    final bool enabled = onTap != null;
    return JoTooltip(
      text: tip,
      child: JoTap(
        onTap: onTap,
        opaque: false,
        builder: (BuildContext c, bool hover, bool down) {
          final double a = down ? 0.20 : (hover ? 0.09 : 0.0);
          final Color bgColor = inverted
              ? Color.alphaBlend(
                  p.text.withValues(alpha: hover || down ? 0.28 : 0.0),
                  p.accent,
                )
              : p.icon.withValues(alpha: a);
          final Color fgColor = !enabled
              ? p.secondary.withValues(alpha: 0.5)
              : inverted
                  ? p.bg
                  : ((hover || down) ? p.text : p.icon);
          return AnimatedContainer(
            duration: kJoTapMs,
            curve: Curves.easeOut,
            padding: const EdgeInsets.all(6),
            decoration: BoxDecoration(color: bgColor),
            child: Icon(icon, size: size, color: fgColor),
          );
        },
      ),
    );
  }
}

/// 细线小按钮。
class JoButton extends StatelessWidget {
  const JoButton({
    super.key,
    required this.text,
    required this.onTap,
    this.color,
    this.bordered = false,
    this.tip,
    this.danger = false,
  });

  final String text;
  final VoidCallback? onTap;
  final Color? color;
  final bool bordered;

  /// 悬停文字提示。给“×”、“↺”这类只有个符号的按钮用。
  final String? tip;

  /// 危险动作（删除等）：悬停时用警示色。
  final bool danger;

  @override
  Widget build(BuildContext context) {
    final JoPalette p = JoTheme.of(context);
    final bool enabled = onTap != null;
    final Color base = !enabled
        ? p.secondary.withValues(alpha: 0.5)
        : (danger ? const Color(0xFFC42B1C) : (color ?? p.secondary));
    final Widget button = JoTap(
      onTap: onTap,
      opaque: false,
      builder: (BuildContext context, bool hover, bool down) {
        final double a = down ? 0.20 : (hover ? 0.09 : 0.0);
        return AnimatedContainer(
          duration: kJoTapMs,
          curve: Curves.easeOut,
          padding: const EdgeInsets.symmetric(
            horizontal: JoSpace.s3,
            vertical: JoSpace.s1 + 2,
          ),
          decoration: BoxDecoration(
            color: a == 0 ? null : base.withValues(alpha: a),
            border: bordered
                ? JoShape.border(hover || down ? base : p.border)
                : null,
            borderRadius: JoShape.br,
          ),
          child: AnimatedDefaultTextStyle(
            duration: kJoTapMs,
            curve: Curves.easeOut,
            style: JoTheme.meta(context).copyWith(
              color: hover || down ? (enabled ? (danger ? base : p.text) : base) : base,
            ),
            child: Text(text),
          ),
        );
      },
    );
    return tip == null ? button : JoTooltip(text: tip!, child: button);
  }
}
