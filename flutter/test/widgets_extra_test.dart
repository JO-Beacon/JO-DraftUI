import 'package:flutter/rendering.dart';
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
            OverlayEntry(builder: (_) => Center(child: SizedBox(width: 300, child: child))),
          ]),
        ),
      ),
    );

Widget wrapWith(JoSettings settings, Widget child) => Directionality(
      textDirection: TextDirection.ltr,
      child: JoTheme(
        settings: settings,
        palette: JoPalette.dark,
        child: MediaQuery(
          data: const MediaQueryData(size: Size(800, 600)),
          child: Overlay(initialEntries: [
            OverlayEntry(builder: (_) => Center(child: SizedBox(width: 300, child: child))),
          ]),
        ),
      ),
    );

void main() {
  testWidgets('JoDropdown 展开、选中、收起', (tester) async {
    String? picked;
    await tester.pumpWidget(wrap(JoDropdown<String>(
      items: const [
        (label: 'overworld', value: 'overworld'),
        (label: 'corridor', value: 'corridor'),
      ],
      value: 'overworld',
      onChanged: (v) => picked = v,
    )));

    expect(find.text('overworld'), findsOneWidget);
    expect(find.text('corridor'), findsNothing);

    await tester.tap(find.text('overworld'));
    await tester.pumpAndSettle();
    expect(find.text('corridor'), findsOneWidget);

    await tester.tap(find.text('corridor'));
    await tester.pumpAndSettle();
    expect(picked, 'corridor');
    expect(find.text('corridor'), findsNothing);
  });

  testWidgets('JoMenuBar 展开并执行菜单项', (tester) async {
    var saved = false;
    await tester.pumpWidget(wrap(JoMenuBar(menus: [
      JoMenu('文件', [
        JoMenuItem('保存', onTap: () => saved = true),
        const JoMenuItem('', divider: true),
        const JoMenuItem('退出'),
      ]),
    ])));

    expect(find.text('保存'), findsNothing);
    await tester.tap(find.text('文件'));
    await tester.pumpAndSettle();
    expect(find.text('保存'), findsOneWidget);

    await tester.tap(find.text('保存'));
    await tester.pumpAndSettle();
    expect(saved, isTrue);
    expect(find.text('保存'), findsNothing);
  });

  testWidgets('JoListRow 点击、选中态', (tester) async {
    var tapped = false;
    await tester.pumpWidget(wrap(JoListRow(
      title: 'overworld',
      subtitle: '32×24',
      selected: true,
      onTap: () => tapped = true,
    )));

    expect(find.text('overworld'), findsOneWidget);
    expect(find.text('32×24'), findsOneWidget);
    await tester.tap(find.text('overworld'));
    expect(tapped, isTrue);
  });

  testWidgets('JoListRow 禁用时不响应', (tester) async {
    await tester.pumpWidget(wrap(const JoListRow(title: '不可用')));
    await tester.tap(find.text('不可用'));
    // 不抛异常即通过；没有回调可断言。
  });

  testWidgets('JoStatusBar 显示左右两段文字', (tester) async {
    await tester.pumpWidget(wrap(const JoStatusBar(left: '就绪', right: '32×24')));
    expect(find.text('就绪'), findsOneWidget);
    expect(find.text('32×24'), findsOneWidget);
  });

  testWidgets('字体族可被 JoSettings 覆盖', (tester) async {
    await tester.pumpWidget(wrapWith(
      const JoSettings(sansFamily: 'MySans', serifFamily: 'MySerif'),
      JoButton(text: '保存', onTap: () {}),
    ));
    final RenderParagraph paragraph =
        tester.renderObject<RenderParagraph>(find.text('保存'));
    expect(paragraph.text.style?.fontFamily, 'MySans');
  });
}
