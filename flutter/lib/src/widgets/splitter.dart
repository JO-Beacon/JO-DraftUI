// 栏与栏之间那条能拖的缝。
//
// 四条要点：
//   1. **抓得到**：能点的区域比画出来的线宽得多。线平时 1px、hover 2px，
//      但手能抓的地方有 10px。
//   2. **不粘手**：按下那一刻记下“当时的宽度”和“鼠标当时的位置”，之后每次
//      都用 `当前鼠标 − 按下位置` 重算宽度，不是一帧一帧加增量。
//   3. **有边界**：这一栏和旁边那一栏都有最小宽度，中间那块画布不许被挤没。
//   4. **有反馈**：hover 变粗变亮、按住时一直亮着；双击回到默认宽度。

import 'package:flutter/widgets.dart';

import '../tokens.dart';
import 'tap.dart';

/// 拖动条能点的宽度（画出来的线仍然只有 1~2px）。
const double kJoSplitterHit = 10;

/// 分隔条的方向：横向拖改变宽度，纵向拖改变高度。
enum JoSplitterAxis { horizontal, vertical }

/// 两根拖动条一共占掉多少横向空间（算“画布还剩多少”时要用）。
const double kJoSplitterTotal = kJoSplitterHit * 2;

class JoSplitter extends StatefulWidget {
  const JoSplitter({
    super.key,
    required this.palette,
    required this.width,
    required this.minWidth,
    required this.maxWidth,
    required this.defaultWidth,
    required this.growsRight,
    required this.onResize,
    this.onResizeEnd,
    this.axis = JoSplitterAxis.horizontal,
    this.colors,
  });

  final JoPalette palette;

  /// 拖动条负责的那一栏现在的宽度（按下时会被记下来当初值）。
  final double width;

  final double minWidth;

  /// 最大宽度。父级按窗口宽度算好传进来，窗口变小就不会撑破。
  final double maxWidth;

  /// 双击复位用的宽度。
  final double defaultWidth;

  /// 往右拖是变宽（左栏，true）还是变窄（右栏，false）。
  final bool growsRight;

  /// 拖的时候回报“这一栏应该是多宽”。
  final ValueChanged<double> onResize;

  /// 松手时喊一声（存盘用）。
  final VoidCallback? onResizeEnd;

  final JoSplitterAxis axis;

  /// 自定义两侧底色；不传时按面板块和画布块推测。
  final List<Color>? colors;

  @override
  State<JoSplitter> createState() => _JoSplitterState();
}

class _JoSplitterState extends State<JoSplitter> {
  double _startWidth = 0;
  double _startX = 0;

  bool _hover = false;
  bool _active = false;

  double get _clamped =>
      widget.width.clamp(widget.minWidth, _safeMax).toDouble();

  double get _safeMax =>
      widget.maxWidth < widget.minWidth ? widget.minWidth : widget.maxWidth;

  void _apply(double want) =>
      widget.onResize(want.clamp(widget.minWidth, _safeMax));

  void _start(DragStartDetails d) {
    _startWidth = _clamped;
    _startX = widget.axis == JoSplitterAxis.horizontal
        ? d.globalPosition.dx
        : d.globalPosition.dy;
    setState(() => _active = true);
  }

  void _update(DragUpdateDetails d) {
    final double moved = (widget.axis == JoSplitterAxis.horizontal
            ? d.globalPosition.dx
            : d.globalPosition.dy) -
        _startX;
    _apply(widget.growsRight ? _startWidth + moved : _startWidth - moved);
  }

  void _end() {
    setState(() => _active = false);
    widget.onResizeEnd?.call();
  }

  @override
  Widget build(BuildContext context) {
    final JoPalette p = widget.palette;
    final bool lit = _hover || _active;
    final bool h = widget.axis == JoSplitterAxis.horizontal;
    return MouseRegion(
      cursor: h ? SystemMouseCursors.resizeColumn : SystemMouseCursors.resizeRow,
      onEnter: (_) => setState(() => _hover = true),
      onExit: (_) => setState(() => _hover = false),
      child: GestureDetector(
        behavior: HitTestBehavior.opaque,
        onHorizontalDragStart: _start,
        onHorizontalDragUpdate: _update,
        onHorizontalDragEnd: (_) => _end(),
        onHorizontalDragCancel: _end,
        onVerticalDragStart: _start,
        onVerticalDragUpdate: _update,
        onVerticalDragEnd: (_) => _end(),
        onVerticalDragCancel: _end,
        onDoubleTap: () => _apply(widget.defaultWidth),
        child: DecoratedBox(
          decoration: BoxDecoration(
            gradient: LinearGradient(
              begin: h ? Alignment.centerLeft : Alignment.topCenter,
              end: h ? Alignment.centerRight : Alignment.bottomCenter,
              colors: widget.colors ??
                  (widget.growsRight
                      ? <Color>[p.surface, p.bg]
                      : <Color>[p.bg, p.surface]),
            ),
          ),
          child: SizedBox(
            width: h ? kJoSplitterHit : double.infinity,
            height: h ? double.infinity : kJoSplitterHit,
            child: Center(
              child: AnimatedContainer(
                duration: kJoTapMs,
                curve: Curves.easeOut,
                width: h ? (lit ? 2 : 1) : double.infinity,
                height: h ? double.infinity : (lit ? 2 : 1),
                color: lit ? p.accent : p.border,
              ),
            ),
          ),
        ),
      ),
    );
  }
}
