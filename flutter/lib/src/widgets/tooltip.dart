// 悬停文字提示（自己画，不用 Flutter 自带的 Tooltip）。
//
// 只做一件事：鼠标停在上面 [wait] 之后，在它旁边浮一行小字；移开就收。
// 不抢焦点、不吃点击（浮层上包了 IgnorePointer）。

import 'dart:async';

import 'package:flutter/widgets.dart';

import '../tokens.dart';

class JoTooltip extends StatefulWidget {
  const JoTooltip({
    super.key,
    required this.text,
    required this.child,
    this.wait = const Duration(milliseconds: 350),
  });

  final String text;
  final Widget child;

  /// 停多久才浮出来。太快会跟着鼠标划过一路闪。
  final Duration wait;

  @override
  State<JoTooltip> createState() => _JoTooltipState();
}

class _JoTooltipState extends State<JoTooltip> {
  Timer? _timer;
  OverlayEntry? _entry;

  @override
  void dispose() {
    _timer?.cancel();
    _remove();
    super.dispose();
  }

  void _remove() {
    _entry?.remove();
    _entry = null;
  }

  void _cancel() {
    _timer?.cancel();
    _timer = null;
    if (_entry != null) {
      _remove();
      setState(() {});
    }
  }

  void _schedule() {
    _timer?.cancel();
    _timer = Timer(widget.wait, _show);
  }

  TextStyle _style(BuildContext context, JoPalette p) =>
      JoTheme.meta(context).copyWith(color: p.text, fontSize: 10);

  void _show() {
    if (!mounted || _entry != null) return;
    final RenderObject? ro = context.findRenderObject();
    if (ro is! RenderBox || !ro.hasSize) return;
    final RenderBox box = ro;
    final OverlayState overlay = Overlay.of(context, rootOverlay: true);

    final JoPalette p = JoTheme.of(context);
    final TextStyle style = _style(context, p);

    final TextPainter tp = TextPainter(
      text: TextSpan(text: widget.text, style: style),
      textDirection: Directionality.of(context),
      textScaler: MediaQuery.textScalerOf(context),
      maxLines: 1,
    )..layout();
    final double w = tp.width + JoSpace.s2 * 2;

    final Size screen = MediaQuery.sizeOf(context);
    final Offset at = box.localToGlobal(Offset.zero);
    const double gap = 6;
    const double edge = 6;

    double top = at.dy - (tp.height + JoSpace.s1 * 2) - gap;
    if (top < edge) top = at.dy + box.size.height + gap;
    double left = at.dx + box.size.width / 2 - w / 2;
    left = left.clamp(
      edge,
      (screen.width - w - edge).clamp(edge, double.infinity),
    );

    _entry = OverlayEntry(
      builder: (BuildContext c) => Positioned(
        left: left,
        top: top,
        child: IgnorePointer(
          child: Container(
            padding: const EdgeInsets.symmetric(
              horizontal: JoSpace.s2,
              vertical: JoSpace.s1,
            ),
            decoration: BoxDecoration(
              color: p.surface,
              border: JoShape.border(p.border),
              borderRadius: JoShape.br,
            ),
            child: Text(
              widget.text,
              style: style,
              softWrap: false,
              textScaler: MediaQuery.textScalerOf(context),
              overflow: TextOverflow.visible,
            ),
          ),
        ),
      ),
    );
    overlay.insert(_entry!);
    setState(() {});
  }

  @override
  Widget build(BuildContext context) {
    return MouseRegion(
      onEnter: (_) => _schedule(),
      onExit: (_) => _cancel(),
      child: widget.child,
    );
  }
}
