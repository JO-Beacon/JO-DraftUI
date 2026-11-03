import 'dart:convert';
import 'dart:io';

import 'package:flutter/painting.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:jo_draftui/jo_draftui.dart';

/// spec/ 是唯一事实来源；这里逐项比对 Dart 常量，防止两边漂移。
void main() {
  late Map<String, dynamic> spec;

  setUpAll(() {
    final file = File('../spec/tokens.json');
    expect(file.existsSync(), isTrue, reason: '找不到 spec/tokens.json');
    spec = jsonDecode(file.readAsStringSync()) as Map<String, dynamic>;
  });

  Color hex(String s) => Color(
        0xFF000000 |
            int.parse(s.substring(1, 3), radix: 16) << 16 |
            int.parse(s.substring(3, 5), radix: 16) << 8 |
            int.parse(s.substring(5, 7), radix: 16),
      );

  test('形状', () {
    expect(JoShape.radius, spec['shape']['radius']);
  });

  test('间距', () {
    final space = spec['space'] as Map<String, dynamic>;
    expect(JoSpace.s1, space['s1']);
    expect(JoSpace.s2, space['s2']);
    expect(JoSpace.s3, space['s3']);
    expect(JoSpace.s4, space['s4']);
    expect(JoSpace.s5, space['s5']);
    expect(JoSpace.s6, space['s6']);
    expect(JoSpace.s7, space['s7']);
  });

  test('字体族', () {
    final fonts = spec['fonts'] as Map<String, dynamic>;
    const prefix = 'packages/jo_draftui/';
    expect(JoFonts.sans, '$prefix${fonts['sans']}');
    expect(JoFonts.serif, '$prefix${fonts['serif']}');
    expect(JoFonts.mono, '$prefix${fonts['mono']}');
  });

  test('字号与行高', () {
    final type = spec['type'] as Map<String, dynamic>;
    expect(JoFonts.body.fontSize, type['body']['size']);
    expect(JoFonts.body.height, type['body']['height']);
    expect(JoFonts.meta.fontSize, type['meta']['size']);
    expect(JoFonts.meta.height, type['meta']['height']);
    expect(JoFonts.meta.letterSpacing, type['meta']['letterSpacing']);
    expect(JoFonts.code.height, type['code']['height']);
  });

  test('调色板', () {
    void compare(JoPalette palette, Map<String, dynamic> p) {
      expect(palette.bg, hex(p['bg']));
      expect(palette.surface, hex(p['surface']));
      expect(palette.border, hex(p['border']));
      expect(palette.text, hex(p['text']));
      expect(palette.secondary, hex(p['secondary']));
      expect(palette.icon, hex(p['icon']));
      expect(palette.accent, hex(p['accent']));
    }

    compare(JoPalette.dark, spec['palette']['dark'] as Map<String, dynamic>);
    compare(JoPalette.light, spec['palette']['light'] as Map<String, dynamic>);
  });
}
