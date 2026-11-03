import 'package:flutter/widgets.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:jo_draftui/jo_draftui.dart';

Widget wrap(Widget child) => Directionality(
      textDirection: TextDirection.ltr,
      child: JoTheme(
        settings: const JoSettings(),
        palette: JoPalette.dark,
        child: MediaQuery(
          data: const MediaQueryData(size: Size(800, 600)),
          child: Overlay(initialEntries: [
            OverlayEntry(builder: (_) => Center(child: child)),
          ]),
        ),
      ),
    );

void main() {
  testWidgets('JoButton 渲染文字并可点击', (tester) async {
    var tapped = false;
    await tester.pumpWidget(wrap(JoButton(text: '保存', onTap: () => tapped = true)));
    expect(find.text('保存'), findsOneWidget);
    await tester.tap(find.text('保存'));
    expect(tapped, isTrue);
  });

  testWidgets('JoToggle 切换文字', (tester) async {
    var on = false;
    await tester.pumpWidget(wrap(StatefulBuilder(
      builder: (context, setState) => JoToggle(
        on: on,
        onChanged: (v) => setState(() => on = v),
      ),
    )));
    expect(find.text('禁用'), findsOneWidget);
    await tester.tap(find.text('禁用'));
    await tester.pumpAndSettle();
    expect(find.text('启用'), findsOneWidget);
  });

  test('调色板明暗两套字段完整', () {
    expect(JoPalette.dark.isDark, isTrue);
    expect(JoPalette.light.isDark, isFalse);
    expect(JoShape.radius, 2);
  });
}
