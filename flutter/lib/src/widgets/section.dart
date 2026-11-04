import 'package:flutter/widgets.dart';

import '../tokens.dart';
import 'tap.dart';

/// 分段选择（一排细线小格）。
class JoSegment<T> extends StatelessWidget {
  const JoSegment({
    super.key,
    required this.options,
    required this.value,
    required this.onChanged,
  });

  final List<({String label, T value})> options;
  final T value;
  final ValueChanged<T> onChanged;

  @override
  Widget build(BuildContext context) {
    final JoPalette p = JoTheme.of(context);
    return Wrap(
      spacing: JoSpace.s2,
      runSpacing: JoSpace.s2,
      children: <Widget>[
        for (int i = 0; i < options.length; i++) _cell(p, options[i]),
      ],
    );
  }

  Widget _cell(JoPalette p, ({String label, T value}) o) {
    final bool on = o.value == value;
    return JoTap(
      onTap: () => onChanged(o.value),
      opaque: false,
      builder: (BuildContext context, bool hover, bool down) {
        return AnimatedContainer(
          duration: kJoTapMs,
          curve: Curves.easeOut,
          padding: const EdgeInsets.symmetric(
            horizontal: JoSpace.s3,
            vertical: JoSpace.s1 + 2,
          ),
          decoration: BoxDecoration(
            color: on
                ? p.surface
                : (hover ? p.surface.withValues(alpha: 0.6) : null),
            border: JoShape.border(on || hover ? p.accent : p.border),
            borderRadius: JoShape.br,
          ),
          child: AnimatedDefaultTextStyle(
            duration: kJoTapMs,
            curve: Curves.easeOut,
            style: JoTheme.meta(context).copyWith(color: on ? p.accent : p.secondary),
            child: Text(o.label),
          ),
        );
      },
    );
  }
}

/// 设置分区：小标题 + 内容。
class JoSection extends StatelessWidget {
  const JoSection({super.key, required this.title, required this.children});

  final String title;
  final List<Widget> children;

  @override
  Widget build(BuildContext context) {
    final JoPalette p = JoTheme.of(context);
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: <Widget>[
        Padding(
          padding: const EdgeInsets.only(bottom: JoSpace.s2),
          child: Text(
            title,
            style: JoTheme.meta(context).copyWith(
              color: p.secondary,
              letterSpacing: 1.2,
            ),
          ),
        ),
        Container(
          decoration: BoxDecoration(
            color: p.surface,
            border: JoShape.border(p.border),
            borderRadius: JoShape.br,
          ),
          child: Column(
            children: <Widget>[
              for (int i = 0; i < children.length; i++) ...<Widget>[
                if (i > 0) Container(height: 1, color: p.border),
                children[i],
              ],
            ],
          ),
        ),
      ],
    );
  }
}

/// 设置行：左标签，右控件。
class JoField extends StatelessWidget {
  const JoField({
    super.key,
    required this.label,
    this.hint,
    required this.child,
  });

  final String label;
  final String? hint;
  final Widget child;

  @override
  Widget build(BuildContext context) {
    final JoPalette p = JoTheme.of(context);
    return Padding(
      padding: const EdgeInsets.symmetric(
        horizontal: JoSpace.s4,
        vertical: JoSpace.s3,
      ),
      child: Row(
        children: <Widget>[
          Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: <Widget>[
              Text(
                label,
                style: JoTheme.ui(context).copyWith(fontSize: 14, color: p.text),
              ),
              if (hint != null)
                Padding(
                  padding: const EdgeInsets.only(top: 2),
                  child: Text(
                    hint!,
                    style: JoTheme.meta(context).copyWith(color: p.secondary),
                  ),
                ),
            ],
          ),
          const Spacer(),
          child,
        ],
      ),
    );
  }
}

/// 开关：文字样式（`启用` / `禁用`），不用圆点滑块。
class JoToggle extends StatelessWidget {
  const JoToggle({
    super.key,
    required this.on,
    required this.onChanged,
    this.onLabel = '启用',
    this.offLabel = '禁用',
  });

  final bool on;
  final ValueChanged<bool> onChanged;
  final String onLabel;
  final String offLabel;

  @override
  Widget build(BuildContext context) {
    final JoPalette p = JoTheme.of(context);
    return JoTap(
      onTap: () => onChanged(!on),
      builder: (BuildContext context, bool hover, bool down) {
        return Container(
          padding: const EdgeInsets.symmetric(
            horizontal: JoSpace.s3,
            vertical: JoSpace.s1 + 2,
          ),
          decoration: BoxDecoration(
            color: on ? p.surface : null,
            border: JoShape.border(on ? p.accent : p.border),
            borderRadius: JoShape.br,
          ),
          child: Text(
            on ? onLabel : offLabel,
            style: JoTheme.meta(context).copyWith(color: on ? p.accent : p.secondary),
          ),
        );
      },
    );
  }
}
