import 'package:flutter/widgets.dart';

/// 动效时长（统一一处，好调）。
const Duration kJoTapMs = Duration(milliseconds: 120);

/// 可点区域的动效基元：hover / 按下。builder 拿到状态自己画。
/// 极简风格用得很克制：120ms、只改底色和描边。
class JoTap extends StatefulWidget {
  const JoTap({
    super.key,
    required this.onTap,
    required this.builder,
    this.cursor = SystemMouseCursors.click,
    this.opaque = true,
  });

  final VoidCallback? onTap;
  final Widget Function(BuildContext context, bool hover, bool down) builder;
  final MouseCursor cursor;
  final bool opaque;

  @override
  State<JoTap> createState() => _JoTapState();
}

class _JoTapState extends State<JoTap> {
  bool _hover = false;
  bool _down = false;

  @override
  Widget build(BuildContext context) {
    final bool enabled = widget.onTap != null;
    return MouseRegion(
      cursor: enabled ? widget.cursor : SystemMouseCursors.basic,
      onEnter: (_) => setState(() => _hover = true),
      onExit: (_) => setState(() {
        _hover = false;
        _down = false;
      }),
      child: GestureDetector(
        behavior:
            widget.opaque ? HitTestBehavior.opaque : HitTestBehavior.deferToChild,
        onTapDown: enabled ? (_) => setState(() => _down = true) : null,
        onTapUp: enabled ? (_) => setState(() => _down = false) : null,
        onTapCancel: () => setState(() => _down = false),
        onTap: widget.onTap,
        child: widget.builder(context, _hover, _down),
      ),
    );
  }
}
