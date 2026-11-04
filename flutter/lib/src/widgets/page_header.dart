import 'package:flutter/widgets.dart';

import '../icons.dart';
import '../tokens.dart';
import 'buttons.dart';

/// 页面头：返回图标 + 标题 + 可选右侧内容。
class JoPageHeader extends StatelessWidget {
  const JoPageHeader({
    super.key,
    required this.title,
    required this.onBack,
    this.trailing,
  });

  final String title;
  final VoidCallback onBack;
  final Widget? trailing;

  @override
  Widget build(BuildContext context) {
    final JoPalette p = JoTheme.of(context);
    return Container(
      height: 48,
      padding: const EdgeInsets.symmetric(horizontal: JoSpace.s4),
      decoration: BoxDecoration(border: Border(bottom: JoShape.side(p.border))),
      child: Row(
        children: <Widget>[
          JoIconAction(icon: JoIcons.arrowLeft, tip: '返回', onTap: onBack),
          const SizedBox(width: JoSpace.s4),
          Text(
            title,
            style: JoTheme.ui(context).copyWith(
              fontSize: 15,
              fontWeight: FontWeight.w700,
              color: p.text,
            ),
          ),
          const Spacer(),
          ?trailing,
        ],
      ),
    );
  }
}
