import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart' show SelectionChangedCause, TextSelectionToolbar;

import '../tokens.dart';

/// 输入框：`CupertinoTextField` + 提示文字，不挂 Material 的 `TextField`。
///
/// 选字、双击选词、右键菜单都由 Cupertino 那一层补齐；菜单外观换成
/// 本库的方角样式。底色、描边、圆角由外层 `Container` 提供。
class JoTextField extends StatefulWidget {
  /// 当前有几个输入框持有焦点。编辑器用它判断是否该让出全局快捷键。
  static int _focused = 0;
  static bool get isEditing => _focused > 0;

  const JoTextField({
    super.key,
    required this.controller,
    required this.style,
    this.hint,
    this.hintStyle,
    this.obscureText = false,
    this.autofocus = false,
    this.onChanged,
    this.onSubmitted,
    this.maxLines = 1,
    this.minLines,
    this.focusNode,
    this.verticalPadding = 4,
  });

  final TextEditingController controller;
  final TextStyle style;
  final String? hint;
  final TextStyle? hintStyle;
  final bool obscureText;
  final bool autofocus;
  final ValueChanged<String>? onChanged;
  final ValueChanged<String>? onSubmitted;

  /// `null` = 不限行数（多行）。[obscureText] 为真时必须是 1。
  final int? maxLines;
  final int? minLines;
  final FocusNode? focusNode;
  final double verticalPadding;

  @override
  State<JoTextField> createState() => _JoTextFieldState();
}

class _JoTextFieldState extends State<JoTextField> {
  FocusNode? _ownFocus;
  bool _hadFocus = false;

  FocusNode get _focus => widget.focusNode ?? (_ownFocus ??= FocusNode());

  void _onFocusChange() {
    final bool has = _focus.hasFocus;
    if (has && !_hadFocus) JoTextField._focused += 1;
    if (!has && _hadFocus) JoTextField._focused -= 1;
    _hadFocus = has;
  }

  @override
  void initState() {
    super.initState();
    _focus.addListener(_onFocusChange);
    widget.controller.addListener(_sanitize);
    WidgetsBinding.instance.addPostFrameCallback((_) => _sanitize());
  }

  /// 把 `\r` 从输入框里清掉（粘贴的 CRLF 会变成豆腐块）。
  void _sanitize() {
    if (!mounted) return;
    final String t = widget.controller.text;
    if (!t.contains('\r')) return;

    final int sel = widget.controller.selection.baseOffset;
    final int before = sel < 0 ? 0 : sel;
    final int removedBefore = '\r'
        .allMatches(t.substring(0, before.clamp(0, t.length)))
        .length;
    final String cleaned = t.replaceAll('\r\n', '\n').replaceAll('\r', '\n');
    final int caret = (sel - removedBefore).clamp(0, cleaned.length);
    widget.controller.value = TextEditingValue(
      text: cleaned,
      selection: sel < 0
          ? const TextSelection.collapsed(offset: 0)
          : TextSelection.collapsed(offset: caret),
      composing: TextRange.empty,
    );
  }

  @override
  void dispose() {
    if (_hadFocus) JoTextField._focused -= 1;
    _focus.removeListener(_onFocusChange);
    widget.controller.removeListener(_sanitize);
    _ownFocus?.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final JoPalette p = JoTheme.of(context);
    return DefaultSelectionStyle(
      selectionColor: p.accent.withValues(alpha: 0.3),
      cursorColor: p.accent,
      child: CupertinoTextField(
        controller: widget.controller,
        focusNode: _focus,
        autofocus: widget.autofocus,
        maxLines: widget.maxLines,
        minLines: widget.minLines,
        obscureText: widget.obscureText,
        style: widget.style,
        placeholder: widget.hint,
        placeholderStyle:
            widget.hintStyle ?? widget.style.copyWith(color: p.secondary),
        cursorColor: p.accent,
        decoration: const BoxDecoration(),
        padding: EdgeInsets.symmetric(vertical: widget.verticalPadding),
        onChanged: widget.onChanged,
        onSubmitted: widget.onSubmitted,
        contextMenuBuilder: (BuildContext c, EditableTextState st) =>
            _toolbar(st, p),
      ),
    );
  }

  /// 选中文字后的右键菜单：方角、本库底色和描边。
  Widget _toolbar(EditableTextState st, JoPalette p) {
    final TextEditingValue v = widget.controller.value;
    final TextSelection sel = v.selection;
    final bool hasSel = sel.isValid && !sel.isCollapsed;
    final bool hasText = v.text.isNotEmpty;
    final bool allSel = hasText && sel.start == 0 && sel.end == v.text.length;
    final TextSelectionToolbarAnchors a = st.contextMenuAnchors;

    final List<({String label, VoidCallback? onTap})> items =
        <({String label, VoidCallback? onTap})>[
          if (!widget.obscureText)
            (
              label: '剪切',
              onTap: hasSel
                  ? () => st.cutSelection(SelectionChangedCause.toolbar)
                  : null,
            ),
          if (!widget.obscureText)
            (
              label: '复制',
              onTap: hasSel
                  ? () => st.copySelection(SelectionChangedCause.toolbar)
                  : null,
            ),
          (
            label: '粘贴',
            onTap: () => st.pasteText(SelectionChangedCause.toolbar),
          ),
          (
            label: '全选',
            onTap: hasText && !allSel
                ? () => st.selectAll(SelectionChangedCause.toolbar)
                : null,
          ),
        ];

    return TextSelectionToolbar(
      anchorAbove: a.primaryAnchor,
      anchorBelow: a.secondaryAnchor ?? a.primaryAnchor,
      toolbarBuilder: (BuildContext ctx, Widget child) => Container(
        decoration: BoxDecoration(
          color: p.surface,
          border: JoShape.border(p.border),
          borderRadius: JoShape.br,
        ),
        child: child,
      ),
      children: <Widget>[
        for (final ({String label, VoidCallback? onTap}) it in items)
          _JoToolbarButton(palette: p, label: it.label, onTap: it.onTap),
      ],
    );
  }
}

/// 选中菜单上的文字按钮（自绘外观）。
class _JoToolbarButton extends StatefulWidget {
  const _JoToolbarButton({
    required this.palette,
    required this.label,
    required this.onTap,
  });

  final JoPalette palette;
  final String label;
  final VoidCallback? onTap;

  @override
  State<_JoToolbarButton> createState() => _JoToolbarButtonState();
}

class _JoToolbarButtonState extends State<_JoToolbarButton> {
  bool _hover = false;

  @override
  Widget build(BuildContext context) {
    final JoPalette p = widget.palette;
    final bool on = widget.onTap != null;
    return MouseRegion(
      cursor: on ? SystemMouseCursors.click : SystemMouseCursors.basic,
      onEnter: (_) => setState(() => _hover = true),
      onExit: (_) => setState(() => _hover = false),
      child: GestureDetector(
        behavior: HitTestBehavior.opaque,
        onTap: widget.onTap,
        child: Container(
          padding: const EdgeInsets.symmetric(
            horizontal: JoSpace.s3,
            vertical: 6,
          ),
          color: on && _hover ? p.border.withValues(alpha: 0.5) : null,
          child: Text(
            widget.label,
            style: JoFonts.ui.copyWith(
              fontSize: 12,
              color: on ? p.text : p.secondary,
            ),
          ),
        ),
      ),
    );
  }
}

/// 带外框的单行输入。外面包一层 Container，负责底色、描边和圆角。
class JoInput extends StatefulWidget {
  const JoInput({
    super.key,
    required this.initial,
    required this.onChanged,
    this.hint,
    this.compact = false,
  });

  final String initial;
  final ValueChanged<String> onChanged;
  final String? hint;
  final bool compact;

  @override
  State<JoInput> createState() => _JoInputState();
}

class _JoInputState extends State<JoInput> {
  late final TextEditingController _c;
  late String _sent;

  @override
  void initState() {
    super.initState();
    _c = TextEditingController(text: widget.initial);
    _sent = widget.initial;
  }

  @override
  void didUpdateWidget(JoInput oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (widget.initial != _sent) {
      _sent = widget.initial;
      _c.text = widget.initial;
      _c.selection = TextSelection.collapsed(offset: _c.text.length);
    }
  }

  void _onChanged(String v) {
    _sent = v;
    widget.onChanged(v);
  }

  @override
  void dispose() {
    _c.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final JoPalette p = JoTheme.of(context);
    final double size = widget.compact ? 12 : 13;
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: JoSpace.s3, vertical: 4),
      decoration: BoxDecoration(
        color: p.surface,
        border: JoShape.border(p.border),
        borderRadius: JoShape.br,
      ),
      child: JoTextField(
        controller: _c,
        onChanged: _onChanged,
        style: JoFonts.ui.copyWith(fontSize: size, color: p.text),
        hint: widget.hint,
        hintStyle: JoFonts.ui.copyWith(fontSize: size, color: p.secondary),
      ),
    );
  }
}

/// 编辑器在用的兼容 API：保留原来的参数，内部走 [JoTextField]。
class JoTextInput extends StatefulWidget {
  const JoTextInput({
    super.key,
    this.controller,
    this.initial,
    this.placeholder,
    this.onChanged,
    this.onSubmitted,
    this.height = 30,
  });

  final TextEditingController? controller;
  final String? initial;
  final String? placeholder;
  final ValueChanged<String>? onChanged;
  final ValueChanged<String>? onSubmitted;
  final double height;

  @override
  State<JoTextInput> createState() => _JoTextInputState();
}

class _JoTextInputState extends State<JoTextInput> {
  late final TextEditingController _controller;
  final FocusNode _focus = FocusNode();
  bool _ownController = false;

  @override
  void initState() {
    super.initState();
    if (widget.controller != null) {
      _controller = widget.controller!;
    } else {
      _controller = TextEditingController(text: widget.initial ?? '');
      _ownController = true;
    }
    _focus.addListener(() => setState(() {}));
  }

  @override
  void didUpdateWidget(covariant JoTextInput oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (_ownController &&
        widget.initial != oldWidget.initial &&
        !_focus.hasFocus &&
        widget.initial != _controller.text) {
      _controller.text = widget.initial ?? '';
    }
  }

  @override
  void dispose() {
    _focus.dispose();
    if (_ownController) _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final p = JoTheme.of(context);
    final bool focused = _focus.hasFocus;
    return Container(
      height: widget.height,
      padding: const EdgeInsets.symmetric(horizontal: JoSpace.s3),
      alignment: Alignment.centerLeft,
      decoration: BoxDecoration(
        color: p.surface,
        border: JoShape.border(focused ? p.accent : p.border),
        borderRadius: JoShape.br,
      ),
      child: JoTextField(
        controller: _controller,
        focusNode: _focus,
        style: JoTheme.ui(context).copyWith(fontSize: 13, color: p.text),
        hint: widget.placeholder,
        hintStyle: JoTheme.ui(context).copyWith(
          fontSize: 13,
          color: p.secondary,
        ),
        onChanged: widget.onChanged,
        onSubmitted: widget.onSubmitted,
        verticalPadding: 0,
      ),
    );
  }
}
