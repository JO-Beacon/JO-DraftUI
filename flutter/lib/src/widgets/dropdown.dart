import 'package:flutter/widgets.dart';

import '../tokens.dart';
import 'popup.dart';
import 'tap.dart';

/// 下拉选择：点开选项列表，选中后收起。
class JoDropdown<T> extends StatefulWidget {
  const JoDropdown({
    super.key,
    required this.items,
    required this.value,
    required this.onChanged,
    this.placeholder = '请选择',
  });

  final List<({String label, T value})> items;
  final T? value;
  final ValueChanged<T> onChanged;
  final String placeholder;

  @override
  State<JoDropdown<T>> createState() => _JoDropdownState<T>();
}

class _JoDropdownState<T> extends State<JoDropdown<T>> {
  final GlobalKey _key = GlobalKey();
  OverlayEntry? _entry;

  @override
  void dispose() {
    // dispose 期间不能 setState，这里只收 OverlayEntry。
    _entry?.remove();
    _entry = null;
    super.dispose();
  }

  void _close() {
    _entry?.remove();
    _entry = null;
    if (mounted) setState(() {});
  }

  void _open() {
    _close();
    final RenderBox? box = _key.currentContext?.findRenderObject() as RenderBox?;
    if (box == null) return;

    _entry = buildJoPopup(
      anchor: box,
      width: box.size.width,
      close: _close,
      child: JoPopupPanel(
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            for (final item in widget.items)
              JoPopupRow(
                label: item.label,
                dense: true,
                selected: item.value == widget.value,
                onTap: () {
                  _close();
                  widget.onChanged(item.value);
                },
              ),
          ],
        ),
      ),
    );
    Overlay.of(context).insert(_entry!);
    setState(() {});
  }

  @override
  Widget build(BuildContext context) {
    final JoPalette p = JoTheme.of(context);
    final bool open = _entry != null;
    final String? current = widget.items
        .where((e) => e.value == widget.value)
        .map((e) => e.label)
        .firstOrNull;

    return JoTap(
      key: _key,
      onTap: _open,
      builder: (BuildContext context, bool hover, bool down) {
        return Container(
          height: 30,
          padding: const EdgeInsets.symmetric(horizontal: JoSpace.s3),
          decoration: BoxDecoration(
            color: hover || open ? p.surface : null,
            border: JoShape.border(open || hover ? p.accent : p.border),
            borderRadius: JoShape.br,
          ),
          child: Row(
            children: [
              Expanded(
                child: Text(
                  current ?? widget.placeholder,
                  overflow: TextOverflow.ellipsis,
                  style: JoTheme.ui(context).copyWith(
                    fontSize: 13,
                    color: current == null ? p.secondary : p.text,
                  ),
                ),
              ),
              const SizedBox(width: JoSpace.s2),
              CustomPaint(
                size: const Size(7, 4),
                painter: _CaretPainter(open ? p.accent : p.secondary),
              ),
            ],
          ),
        );
      },
    );
  }
}

class _CaretPainter extends CustomPainter {
  _CaretPainter(this.color);

  final Color color;

  @override
  void paint(Canvas canvas, Size size) {
    final Paint paint = Paint()..color = color;
    final Path path = Path()
      ..moveTo(0, 0)
      ..lineTo(size.width, 0)
      ..lineTo(size.width / 2, size.height)
      ..close();
    canvas.drawPath(path, paint);
  }

  @override
  bool shouldRepaint(covariant _CaretPainter oldDelegate) =>
      oldDelegate.color != color;
}
