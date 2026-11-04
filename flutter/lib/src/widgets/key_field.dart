import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';

import '../tokens.dart';

/// 快捷键输入框：点一下进入捕获，然后按组合键。
///
/// - Esc 清除这一条（不是记下 Esc）
/// - 只按修饰键不会结束捕获
class JoKeyField extends StatefulWidget {
  const JoKeyField({super.key, required this.spec, required this.onChanged});

  /// 当前的键位写法，如 `Ctrl+E`。空 = 没设。
  final String spec;
  final ValueChanged<String> onChanged;

  @override
  State<JoKeyField> createState() => _JoKeyFieldState();
}

class _JoKeyFieldState extends State<JoKeyField> {
  final FocusNode _focus = FocusNode();
  bool _capturing = false;

  @override
  void dispose() {
    _focus.dispose();
    super.dispose();
  }

  /// 把按下的键译成 `Ctrl+E` 这种写法。只按了修饰键就返回 null（继续等）。
  String? _specOf(KeyEvent e) {
    final LogicalKeyboardKey k = e.logicalKey;
    if (k == LogicalKeyboardKey.controlLeft ||
        k == LogicalKeyboardKey.controlRight ||
        k == LogicalKeyboardKey.shiftLeft ||
        k == LogicalKeyboardKey.shiftRight ||
        k == LogicalKeyboardKey.altLeft ||
        k == LogicalKeyboardKey.altRight ||
        k == LogicalKeyboardKey.metaLeft ||
        k == LogicalKeyboardKey.metaRight) {
      return null;
    }
    final HardwareKeyboard kb = HardwareKeyboard.instance;
    final List<String> parts = <String>[];
    if (kb.isControlPressed) parts.add('Ctrl');
    if (kb.isShiftPressed) parts.add('Shift');
    if (kb.isAltPressed) parts.add('Alt');
    final String label = k.keyLabel;
    if (label.isEmpty) return null;
    parts.add(label);
    return parts.join('+');
  }

  @override
  Widget build(BuildContext context) {
    final JoPalette p = JoTheme.of(context);
    return MouseRegion(
      cursor: SystemMouseCursors.click,
      child: GestureDetector(
        onTap: () {
          setState(() => _capturing = true);
          _focus.requestFocus();
        },
        child: Focus(
          focusNode: _focus,
          onFocusChange: (bool has) {
            if (!has && _capturing) setState(() => _capturing = false);
          },
          onKeyEvent: (FocusNode node, KeyEvent e) {
            if (_capturing && e.logicalKey == LogicalKeyboardKey.escape) {
              if (e is KeyDownEvent) {
                widget.onChanged('');
                setState(() => _capturing = false);
              }
              return KeyEventResult.handled;
            }
            if (!_capturing) return KeyEventResult.ignored;
            if (e is! KeyDownEvent) return KeyEventResult.handled;
            final String? spec = _specOf(e);
            if (spec == null) return KeyEventResult.handled;
            widget.onChanged(spec);
            setState(() => _capturing = false);
            return KeyEventResult.handled;
          },
          child: Container(
            height: 32,
            alignment: Alignment.centerLeft,
            padding: const EdgeInsets.symmetric(horizontal: JoSpace.s3),
            decoration: BoxDecoration(
              color: p.surface,
              border: JoShape.border(_capturing ? p.accent : p.border),
            ),
            child: Text(
              _capturing
                  ? '按组合键……'
                  : (widget.spec.trim().isEmpty ? '未设置' : widget.spec),
              style: JoTheme.ui(context).copyWith(
                fontSize: 13,
                color: _capturing
                    ? p.accent
                    : (widget.spec.trim().isEmpty ? p.secondary : p.text),
              ),
            ),
          ),
        ),
      ),
    );
  }
}
