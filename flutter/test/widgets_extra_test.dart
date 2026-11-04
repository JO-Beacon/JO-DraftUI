import 'package:flutter/rendering.dart';
import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:jo_draftui/jo_draftui.dart';

Widget _wrapCore(JoSettings settings, Widget child) => Localizations(
  locale: const Locale('zh'),
  delegates: const [
        DefaultWidgetsLocalizations.delegate,
        JoCupertinoLocalizationsDelegate(),
      ],
  child: Directionality(
    textDirection: TextDirection.ltr,
    child: JoTheme(
      settings: settings,
      palette: JoPalette.dark,
      child: MediaQuery(
        data: const MediaQueryData(size: Size(800, 600)),
        child: Overlay(
          initialEntries: [
            OverlayEntry(
              builder: (_) => Center(child: SizedBox(width: 300, child: child)),
            ),
          ],
        ),
      ),
    ),
  ),
);

Widget wrap(Widget child) => _wrapCore(const JoSettings(), child);

Widget wrapWith(JoSettings settings, Widget child) =>
    _wrapCore(settings, child);

void main() {
  testWidgets('JoDropdown 展开、选中、收起', (tester) async {
    String? picked;
    await tester.pumpWidget(
      wrap(
        JoDropdown<String>(
          items: const [
            (label: 'overworld', value: 'overworld'),
            (label: 'corridor', value: 'corridor'),
          ],
          value: 'overworld',
          onChanged: (v) => picked = v,
        ),
      ),
    );

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
    await tester.pumpWidget(
      wrap(
        JoMenuBar(
          menus: [
            JoMenu('文件', [
              JoMenuItem('保存', onTap: () => saved = true),
              const JoMenuItem('', divider: true),
              const JoMenuItem('退出'),
            ]),
          ],
        ),
      ),
    );

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
    await tester.pumpWidget(
      wrap(
        JoListRow(
          title: 'overworld',
          subtitle: '32×24',
          selected: true,
          onTap: () => tapped = true,
        ),
      ),
    );

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
    await tester.pumpWidget(
      wrap(const JoStatusBar(left: '就绪', right: '32×24')),
    );
    expect(find.text('就绪'), findsOneWidget);
    expect(find.text('32×24'), findsOneWidget);
  });

  testWidgets('字体族可被 JoSettings 覆盖', (tester) async {
    await tester.pumpWidget(
      wrapWith(
        const JoSettings(sansFamily: 'MySans', serifFamily: 'MySerif'),
        JoButton(text: '保存', onTap: () {}),
      ),
    );
    final RenderParagraph paragraph = tester.renderObject<RenderParagraph>(
      find.text('保存'),
    );
    expect(paragraph.text.style?.fontFamily, 'MySans');
  });

  testWidgets('JoTextInput 显示初值并回调输入', (tester) async {
    String got = '';
    await tester.pumpWidget(
      wrapWith(
        const JoSettings(),
        JoTextInput(initial: 'overworld', onChanged: (v) => got = v),
      ),
    );
    expect(find.text('overworld'), findsOneWidget);
    await tester.enterText(find.byType(EditableText), 'corridor');
    await tester.pump();
    expect(got, 'corridor');
  });

  testWidgets('菜单展开后 Esc 关闭', (tester) async {
    await tester.pumpWidget(
      wrap(
        JoMenuBar(
          menus: [
            JoMenu('文件', [JoMenuItem('保存', onTap: () {})]),
          ],
        ),
      ),
    );

    await tester.tap(find.text('文件'));
    await tester.pumpAndSettle();
    expect(find.text('保存'), findsOneWidget);

    await tester.sendKeyEvent(LogicalKeyboardKey.escape);
    await tester.pumpAndSettle();
    expect(find.text('保存'), findsNothing);
  });

  testWidgets('JoMarkdown 渲染标题与粗体', (tester) async {
    await tester.pumpWidget(
      wrap(JoMarkdown(data: '# 标题\n\n正文 **加粗** 结束', style: JoFonts.meta)),
    );
    expect(find.text('标题', findRichText: true), findsOneWidget);
    expect(find.textContaining('加粗', findRichText: true), findsOneWidget);
  });

  testWidgets('JoMarkdown 告警块与表格', (tester) async {
    await tester.pumpWidget(
      wrap(
        JoMarkdown(
          data: '> [!NOTE]\n> 注意内容\n\n| A | B |\n|---|---|\n| 1 | 2 |',
          style: JoFonts.meta,
        ),
      ),
    );
    expect(find.text('注意'), findsOneWidget);
    expect(find.text('A', findRichText: true), findsOneWidget);
    expect(find.text('1', findRichText: true), findsOneWidget);
  });

  testWidgets('JoPageHeader 渲染标题并响应返回', (tester) async {
    var back = false;
    await tester.pumpWidget(wrap(JoPageHeader(
      title: '设置',
      onBack: () => back = true,
    )));
    expect(find.text('设置'), findsOneWidget);
    await tester.tap(find.byIcon(JoIcons.arrowLeft));
    expect(back, isTrue);
  });

  testWidgets('JoKeyField 显示当前键位并进入捕获', (tester) async {
    await tester.pumpWidget(wrap(JoKeyField(
      spec: 'Ctrl+E',
      onChanged: (_) {},
    )));
    expect(find.text('Ctrl+E'), findsOneWidget);
    await tester.tap(find.text('Ctrl+E'));
    await tester.pump();
    expect(find.text('按组合键……'), findsOneWidget);
  });
}
