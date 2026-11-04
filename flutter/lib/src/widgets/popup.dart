import 'package:flutter/widgets.dart';

import '../tokens.dart';
import 'tap.dart';

/// 在锚点下方弹出一块面板；点面板外任意位置调用 [close]。
///
/// 只负责“造”这个 OverlayEntry，插入与移除由调用方管，
/// 这样每个控件都能在自己的 dispose 里收干净。
OverlayEntry buildJoPopup({
  required RenderBox anchor,
  required double width,
  required Widget child,
  required VoidCallback close,
  double barrierTop = 0,
}) {
  final Offset at = anchor.localToGlobal(Offset.zero);
  return OverlayEntry(
    builder: (BuildContext context) => Stack(
      children: [
        Positioned(
          top: barrierTop,
          left: 0,
          right: 0,
          bottom: 0,
          child: GestureDetector(
            behavior: HitTestBehavior.opaque,
            onTap: close,
          ),
        ),
        Positioned(
          left: at.dx,
          top: at.dy + anchor.size.height,
          child: SizedBox(width: width, child: child),
        ),
      ],
    ),
  );
}

/// 弹出面板的外框：底色、1px 线、2px 圆角、零阴影。
class JoPopupPanel extends StatelessWidget {
  const JoPopupPanel({super.key, required this.child});

  final Widget child;

  @override
  Widget build(BuildContext context) {
    final JoPalette p = JoTheme.of(context);
    return Container(
      padding: const EdgeInsets.symmetric(vertical: 4),
      decoration: BoxDecoration(
        color: p.surface,
        border: JoShape.border(p.border),
        borderRadius: JoShape.br,
      ),
      child: Column(mainAxisSize: MainAxisSize.min, children: [child]),
    );
  }
}

/// 弹出面板里的一行。
class JoPopupRow extends StatelessWidget {
  const JoPopupRow({
    super.key,
    required this.label,
    required this.onTap,
    this.selected = false,
    this.height = 28,
    this.dense = false,
  });

  final String label;
  final VoidCallback? onTap;
  final bool selected;
  final double height;

  /// dense 用于下拉选项，字小一档。
  final bool dense;

  @override
  Widget build(BuildContext context) {
    final JoPalette p = JoTheme.of(context);
    final bool enabled = onTap != null;
    return JoTap(
      onTap: onTap,
      builder: (BuildContext context, bool hover, bool down) {
        final double a = down ? 0.20 : (hover ? 0.09 : 0.0);
        return Container(
          height: height,
          alignment: Alignment.centerLeft,
          padding: const EdgeInsets.symmetric(horizontal: JoSpace.s3),
          color: a == 0 ? null : p.text.withValues(alpha: a),
          child: Text(
            label,
            style: JoTheme.ui(context).copyWith(
              fontSize: dense ? 12 : 13,
              color: !enabled
                  ? p.secondary.withValues(alpha: 0.5)
                  : (selected ? p.accent : p.text),
            ),
          ),
        );
      },
    );
  }
}
