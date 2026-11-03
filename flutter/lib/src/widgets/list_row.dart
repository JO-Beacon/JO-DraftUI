import 'package:flutter/widgets.dart';

import '../tokens.dart';
import 'tap.dart';

/// 列表行：悬停、选中、点击。
///
/// - 选中：底色用强调色 14%，文字用强调色，左侧 2px 强调色竖线
/// - 悬停：底色用正文色 9%
/// - 有 subtitle 时高 42，否则 30
class JoListRow extends StatelessWidget {
  const JoListRow({
    super.key,
    required this.title,
    this.subtitle,
    this.leading,
    this.selected = false,
    this.onTap,
    this.height,
  });

  final String title;
  final String? subtitle;
  final Widget? leading;
  final bool selected;
  final VoidCallback? onTap;

  /// 默认按有无副标题自动取 30 / 42。
  final double? height;

  @override
  Widget build(BuildContext context) {
    final JoPalette p = JoTheme.of(context);
    final bool enabled = onTap != null;
    return JoTap(
      onTap: onTap,
      builder: (BuildContext context, bool hover, bool down) {
        final Color? bg = selected
            ? p.accent.withValues(alpha: 0.14)
            : (hover ? p.text.withValues(alpha: 0.09) : null);
        return Container(
          height: height ?? (subtitle == null ? 30 : 42),
          padding: const EdgeInsets.symmetric(horizontal: JoSpace.s3),
          decoration: BoxDecoration(
            color: bg,
            border: Border(
              left: BorderSide(
                color: selected ? p.accent : const Color(0x00000000),
                width: 2,
              ),
            ),
          ),
          child: Row(
            children: [
              if (leading != null) ...[
                leading!,
                const SizedBox(width: JoSpace.s2),
              ],
              Expanded(
                child: Column(
                  mainAxisAlignment: MainAxisAlignment.center,
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text(
                      title,
                      overflow: TextOverflow.ellipsis,
                      style: JoTheme.ui(context).copyWith(
                        fontSize: 13,
                        color: !enabled
                            ? p.secondary.withValues(alpha: 0.5)
                            : (selected ? p.accent : p.text),
                      ),
                    ),
                    if (subtitle != null)
                      Text(
                        subtitle!,
                        overflow: TextOverflow.ellipsis,
                        style: JoTheme.meta(context).copyWith(color: p.secondary),
                      ),
                  ],
                ),
              ),
            ],
          ),
        );
      },
    );
  }
}
