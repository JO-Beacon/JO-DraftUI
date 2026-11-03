import 'package:flutter/widgets.dart';
import 'package:jo_draftui/jo_draftui.dart';

void main() {
  runApp(const GalleryApp());
}

class GalleryApp extends StatelessWidget {
  const GalleryApp({super.key});

  @override
  Widget build(BuildContext context) {
    return WidgetsApp(
      title: 'JO-DraftUI',
      color: JoPalette.dark.bg,
      builder: (context, child) => JoTheme(
        settings: const JoSettings(),
        palette: JoPalette.dark,
        child: const ColoredBox(
          color: Color(0xFF0E0F12),
          child: Gallery(),
        ),
      ),
    );
  }
}

class Gallery extends StatefulWidget {
  const Gallery({super.key});

  @override
  State<Gallery> createState() => _GalleryState();
}

class _GalleryState extends State<Gallery> {
  bool toggle = true;
  int segment = 0;
  double sideWidth = 260;
  String world = 'overworld';
  int selected = 2;
  String status = '就绪';

  static const worlds = [
    (label: 'overworld', value: 'overworld'),
    (label: 'corridor', value: 'corridor'),
  ];

  static const items = ['基元', '按钮', '选择', '输入', '容器', '反馈'];

  @override
  Widget build(BuildContext context) {
    final p = JoTheme.of(context);
    return Column(
      children: [
        JoMenuBar(menus: [
          JoMenu('文件', [
            JoMenuItem('打开世界…', onTap: () => setState(() => status = '打开世界')),
            JoMenuItem('保存', onTap: () => setState(() => status = '已保存')),
            const JoMenuItem('', divider: true),
            const JoMenuItem('退出'),
          ]),
          JoMenu('编辑', [
            const JoMenuItem('撤销'),
            const JoMenuItem('重做'),
          ]),
          JoMenu('帮助', [
            JoMenuItem('关于', onTap: () => setState(() => status = 'JO-DraftUI 0.1.0')),
          ]),
        ]),
        Expanded(
          child: Row(
            children: [
              _left(p),
              JoSplitter(
                palette: p,
                width: sideWidth,
                minWidth: 200,
                maxWidth: 420,
                defaultWidth: 260,
                growsRight: false,
                onResize: (w) => setState(() => sideWidth = w),
              ),
              Expanded(child: _center(p)),
            ],
          ),
        ),
        JoStatusBar(left: status, right: '120 项'),
      ],
    );
  }

  Widget _left(JoPalette p) {
    return Container(
      width: sideWidth,
      decoration: BoxDecoration(
        color: p.surface,
        border: Border(right: JoShape.side(p.border)),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          Padding(
            padding: const EdgeInsets.all(JoSpace.s3),
            child: JoDropdown<String>(
              items: worlds,
              value: world,
              onChanged: (v) => setState(() => world = v),
            ),
          ),
          Container(height: 1, color: p.border),
          for (int i = 0; i < items.length; i++)
            JoListRow(
              title: items[i],
              selected: selected == i,
              onTap: () => setState(() => selected = i),
            ),
        ],
      ),
    );
  }

  Widget _center(JoPalette p) {
    return SingleChildScrollView(
      padding: const EdgeInsets.all(JoSpace.s5),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          JoSection(
            title: '按钮',
            children: [
              JoField(
                label: '文字按钮',
                hint: '细线、极小圆角',
                child: Row(
                  children: [
                    JoButton(text: '保存', onTap: () {}),
                    const SizedBox(width: JoSpace.s2),
                    JoButton(text: '取消', onTap: () {}),
                    const SizedBox(width: JoSpace.s2),
                    JoButton(text: '删除', danger: true, onTap: () {}),
                  ],
                ),
              ),
              JoField(
                label: '图标动作',
                hint: '悬停有提示',
                child: Row(
                  children: [
                    JoIconAction(icon: JoIcons.add, tip: '新建', onTap: () {}),
                    JoIconAction(icon: JoIcons.close, tip: '关闭', onTap: () {}),
                    JoIconAction(
                      icon: JoIcons.check,
                      tip: '主行动',
                      inverted: true,
                      onTap: () {},
                    ),
                  ],
                ),
              ),
            ],
          ),
          const SizedBox(height: JoSpace.s5),
          JoSection(
            title: '选择',
            children: [
              JoField(
                label: '分段',
                child: JoSegment<int>(
                  options: const [
                    (label: '全部', value: 0),
                    (label: '地形', value: 1),
                    (label: '道具', value: 2),
                  ],
                  value: segment,
                  onChanged: (v) => setState(() => segment = v),
                ),
              ),
              JoField(
                label: '开关',
                hint: '文字样式，不用滑块',
                child: JoToggle(
                  on: toggle,
                  onChanged: (v) => setState(() => toggle = v),
                ),
              ),
              JoField(
                label: '下拉',
                child: SizedBox(
                  width: 200,
                  child: JoDropdown<String>(
                    items: worlds,
                    value: world,
                    onChanged: (v) => setState(() => world = v),
                  ),
                ),
              ),
            ],
          ),
          const SizedBox(height: JoSpace.s5),
          JoSection(
            title: '列表行',
            children: [
              JoListRow(
                title: 'overworld',
                subtitle: '32×24',
                selected: true,
                onTap: () {},
              ),
              JoListRow(title: 'corridor', subtitle: '32×12', onTap: () {}),
              const JoListRow(title: '不可用'),
            ],
          ),
          const SizedBox(height: JoSpace.s5),
          JoSection(
            title: '色板',
            children: [
              JoField(
                label: '暗 · 机器',
                child: Row(
                  children: [
                    _swatch(p.bg),
                    _swatch(p.surface),
                    _swatch(p.border),
                    _swatch(p.text),
                    _swatch(p.secondary),
                    _swatch(p.accent),
                  ],
                ),
              ),
              JoField(
                label: '明 · 纸',
                child: Row(
                  children: [
                    _swatch(JoPalette.light.bg),
                    _swatch(JoPalette.light.surface),
                    _swatch(JoPalette.light.border),
                    _swatch(JoPalette.light.text),
                    _swatch(JoPalette.light.secondary),
                    _swatch(JoPalette.light.accent),
                  ],
                ),
              ),
            ],
          ),
        ],
      ),
    );
  }

  Widget _swatch(Color color) {
    return Container(
      width: 28,
      height: 20,
      margin: const EdgeInsets.only(right: JoSpace.s1),
      decoration: BoxDecoration(
        color: color,
        border: Border.all(color: const Color(0x33FFFFFF), width: 1),
      ),
    );
  }
}
