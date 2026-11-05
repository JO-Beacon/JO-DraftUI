import 'package:flutter/widgets.dart';

import '../tokens.dart';

/// 列表区：标题、操作按钮、分隔线、可滚动列表。
///
/// 左右栏的列表都用它，避免各写一套边距。
class JoListSection extends StatelessWidget {
  const JoListSection({
    super.key,
    required this.title,
    this.actions = const <Widget>[],
    required this.child,
    this.titlePadding,
    this.actionsPadding,
  });

  final String title;
  final List<Widget> actions;

  /// 通常是 ListView。
  final Widget child;

  final EdgeInsetsGeometry? titlePadding;
  final EdgeInsetsGeometry? actionsPadding;

  @override
  Widget build(BuildContext context) {
    final JoPalette p = JoTheme.of(context);
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: <Widget>[
        Padding(
          padding: titlePadding ??
              const EdgeInsets.fromLTRB(
                JoSpace.s3,
                JoSpace.s3,
                JoSpace.s3,
                JoSpace.s1,
              ),
          child: Text(
            title,
            style: JoTheme.meta(context).copyWith(
              color: p.secondary,
              letterSpacing: 1.2,
            ),
          ),
        ),
        if (actions.isNotEmpty) ...[
          Padding(
            padding: actionsPadding ??
                const EdgeInsets.fromLTRB(
                  JoSpace.s2,
                  0,
                  JoSpace.s2,
                  JoSpace.s2,
                ),
            child: Row(children: actions),
          ),
        ],
        Expanded(child: child),
      ],
    );
  }
}
