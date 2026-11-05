import 'package:flutter/widgets.dart';

import '../tokens.dart';

/// 底部状态栏：左边一句话，右边一句补充。
class JoStatusBar extends StatelessWidget {
  const JoStatusBar({
    super.key,
    required this.left,
    this.right,
    this.error = false,
  });

  final String left;
  final String? right;

  /// 错误状态：左边文字用警示色。
  final bool error;

  @override
  Widget build(BuildContext context) {
    final JoPalette p = JoTheme.of(context);
    return Container(
      height: 26,
      padding: const EdgeInsets.symmetric(horizontal: JoSpace.s3),
      decoration: BoxDecoration(
        color: p.surface,
        border: Border(top: JoShape.side(p.border)),
      ),
      child: Row(
        children: [
          Expanded(
            child: Text(
              left,
              overflow: TextOverflow.ellipsis,
              style: JoTheme.meta(context).copyWith(
                color: error ? const Color(0xFFC42B1C) : p.secondary,
              ),
            ),
          ),
          if (right != null)
            Text(
              right!,
              style: JoTheme.meta(context).copyWith(color: p.secondary),
            ),
        ],
      ),
    );
  }
}
