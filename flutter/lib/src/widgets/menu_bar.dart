import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';

import '../tokens.dart';
import 'popup.dart';
import 'tap.dart';

/// 一个菜单项。divider 为真时画一条分隔线，label 忽略。
class JoMenuItem {
  const JoMenuItem(this.label, {this.onTap, this.divider = false});

  final String label;
  final VoidCallback? onTap;
  final bool divider;
}

/// 顶部菜单栏的一项。
class JoMenu {
  const JoMenu(this.title, this.items);

  final String title;
  final List<JoMenuItem> items;
}

/// 菜单栏：点标题弹出下拉，点别处或选中后收起。
class JoMenuBar extends StatefulWidget {
  const JoMenuBar({super.key, required this.menus});

  final List<JoMenu> menus;

  @override
  State<JoMenuBar> createState() => _JoMenuBarState();
}

class _JoMenuBarState extends State<JoMenuBar> {
  final List<GlobalKey> _keys = [];
  OverlayEntry? _entry;
  int? _open;
  final FocusNode _focus = FocusNode();

  @override
  void initState() {
    super.initState();
    _keys.addAll(List.generate(widget.menus.length, (_) => GlobalKey()));
  }

  @override
  void didUpdateWidget(covariant JoMenuBar oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (oldWidget.menus.length != widget.menus.length) {
      _close();
      _keys
        ..clear()
        ..addAll(List.generate(widget.menus.length, (_) => GlobalKey()));
    }
  }

  @override
  void dispose() {
    // dispose 期间不能 setState，这里只收 OverlayEntry。
    _entry?.remove();
    _entry = null;
    _focus.dispose();
    super.dispose();
  }

  KeyEventResult _onKey(FocusNode node, KeyEvent event) {
    if (event is! KeyDownEvent) return KeyEventResult.ignored;

    if (event.logicalKey == LogicalKeyboardKey.escape && _open != null) {
      _close();
      return KeyEventResult.handled;
    }

    if (_open != null && widget.menus.isNotEmpty) {
      final int delta = event.logicalKey == LogicalKeyboardKey.arrowLeft
          ? -1
          : (event.logicalKey == LogicalKeyboardKey.arrowRight ? 1 : 0);
      if (delta != 0) {
        _toggle((_open! + delta + widget.menus.length) % widget.menus.length);
        return KeyEventResult.handled;
      }
    }
    return KeyEventResult.ignored;
  }

  void _close() {
    _entry?.remove();
    _entry = null;
    if (mounted && _open != null) setState(() => _open = null);
  }

  void _toggle(int index) {
    if (_open == index) {
      _close();
      return;
    }
    _close();
    final RenderBox? box =
        _keys[index].currentContext?.findRenderObject() as RenderBox?;
    if (box == null) return;

    _entry = buildJoPopup(
      anchor: box,
      width: 200,
      close: _close,
      barrierTop: 30,
      child: JoPopupPanel(
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            for (final item in widget.menus[index].items)
              if (item.divider)
                Container(height: 1, color: JoTheme.of(context).border)
              else
                JoPopupRow(
                  label: item.label,
                  onTap: item.onTap == null
                      ? null
                      : () {
                          _close();
                          item.onTap!();
                        },
                ),
          ],
        ),
      ),
    );
    Overlay.of(context).insert(_entry!);
    setState(() => _open = index);
    _focus.requestFocus();
  }

  @override
  Widget build(BuildContext context) {
    final p = JoTheme.of(context);
    return Focus(
      focusNode: _focus,
      onKeyEvent: _onKey,
      child: Container(
        height: 30,
        padding: const EdgeInsets.symmetric(horizontal: JoSpace.s2),
        decoration: BoxDecoration(
          color: p.surface,
          border: Border(bottom: JoShape.side(p.border)),
        ),
        child: MouseRegion(
          onHover: (PointerHoverEvent event) {
            if (_open == null) return;
            for (int i = 0; i < _keys.length; i++) {
              final RenderBox? box =
                  _keys[i].currentContext?.findRenderObject() as RenderBox?;
              if (box == null) continue;
              final Rect rect = box.localToGlobal(Offset.zero) & box.size;
              if (rect.contains(event.position) && _open != i) {
                _toggle(i);
                return;
              }
            }
          },
          child: Row(
            children: [
              for (int i = 0; i < widget.menus.length; i++)
                JoTap(
                  key: _keys[i],
                  onTap: () => _toggle(i),
                  opaque: false,
                  builder: (BuildContext context, bool hover, bool down) {
                    final bool lit = _open == i || hover;
                    return MouseRegion(
                      onEnter: (_) {
                        if (_open != null && _open != i) _toggle(i);
                      },
                      child: Container(
                        padding: const EdgeInsets.symmetric(
                          horizontal: JoSpace.s3,
                        ),
                        alignment: Alignment.center,
                        color: lit
                            ? p.text.withValues(alpha: _open == i ? 0.12 : 0.09)
                            : null,
                        child: Text(
                          widget.menus[i].title,
                          style: JoTheme.ui(context).copyWith(
                            fontSize: 13,
                            color: lit ? p.text : p.secondary,
                          ),
                        ),
                      ),
                    );
                  },
                ),
            ],
          ),
        ),
      ),
    );
  }
}
