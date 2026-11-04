import 'package:flutter/cupertino.dart';
import 'package:flutter/foundation.dart';

/// 中文的 Cupertino 文案，只改输入框右键菜单相关的几条。
class JoCupertinoLocalizations extends DefaultCupertinoLocalizations {
  const JoCupertinoLocalizations();

  @override
  String get cutButtonLabel => '剪切';

  @override
  String get copyButtonLabel => '复制';

  @override
  String get pasteButtonLabel => '粘贴';

  @override
  String get selectAllButtonLabel => '全选';

  @override
  String get lookUpButtonLabel => '查询';

  @override
  String get searchWebButtonLabel => '在网页中搜索';

  @override
  String get shareButtonLabel => '共享';
}

/// 挂在 `localizationsDelegates` 里，任何语言都用这份中文。
class JoCupertinoLocalizationsDelegate
    extends LocalizationsDelegate<CupertinoLocalizations> {
  const JoCupertinoLocalizationsDelegate();

  @override
  bool isSupported(Locale locale) => true;

  @override
  Future<CupertinoLocalizations> load(Locale locale) =>
      SynchronousFuture<CupertinoLocalizations>(
        const JoCupertinoLocalizations(),
      );

  @override
  bool shouldReload(JoCupertinoLocalizationsDelegate old) => false;
}
