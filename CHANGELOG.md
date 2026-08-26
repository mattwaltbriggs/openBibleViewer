# Changelog

## 0.9.1 (2026-08-26) — macOS Apple Silicon Port

This release ports openBibleViewer to compile and run natively on
Apple Silicon (arm64) Macs using Qt 6.

### Build system

- Migrated CMake build from Qt 5 / QtWebKit to Qt 6 / QtWebEngineWidgets
- Added `Core5Compat` bridge for QTextCodec and other removed APIs
- Replaced CLucene full-text search engine with Xapian 1.4
- Created `cmake/FindXapian.cmake` for Xapian discovery
- Created `src/core/xapian_wrapper.h` to isolate Xapian headers from
  Qt keyword conflicts (signals/slots)
- Disabled `CMAKE_AUTOUIC` to coexist with explicit `qt6_wrap_ui()`
- Added `package.sh` for one-command macOS packaging (build, macdeployqt,
  Xapian bundling, QtWebEngineProcess helper setup, ad-hoc codesigning,
  DMG creation)

### Qt 5 → Qt 6 API changes

- QScriptEngine → QJSEngine (webdictionary, webcommentary)
- QTextCodec removed from Qt6Core; using Qt6::Core5Compat
- QWebEnginePage API: NavigateBack/Forward → Back/Forward
- setUserStyleSheetUrl() removed; replaced with QWebEngineScript
  JavaScript injection for custom CSS
- QRegExp → QRegularExpression (verseurl, biblelink, webform)
- QStringRef → QStringView (zefania parser, 5 module files)
- qSort → std::sort (search results, bible/dictionary/commentary forms)
- QModelIndex::child() removed; using sourceModel()->index()
- QSortFilterProxyModel::filterRegExp → filterRegularExpression
- QHash::unite() → insert() (advancedinterface)
- Qt::CTRL+Qt::SHIFT → Qt::CTRL|Qt::SHIFT (keyboard shortcuts)
- QMapIterator: use constBegin/constEnd (windowmanager)
- QWebEngineView forward declaration fix (simplenotes.h)
- QSet construction from QList: use iterator constructor
- QSet::toList() → .values()
- QLayout::setMargin → setContentsMargins
- qOverload needed for overloaded signal connections

### Bug fixes

- Book selector now correctly maps combo box index to book ID
  (QHash::keys/values ordering not guaranteed in Qt6)
- Verse highlighting in search results: m_lastTextRanges set before
  showText() so scroll-to-anchor works
- Deferred scrollToAnchor to loadFinished signal (Qt6 setHtml is async)
- QtWebEngineProcess helper: relative symlinks to main bundle Frameworks/
  for proper library resolution on Apple Silicon

### Packaging

- Hardcoded openBibleViewer.icns icon in Info.plist.in
- Icon upscaled from 124x124 source to all macOS icon sizes via sips/iconutil
- Ad-hoc codesigning for all binaries (required on Apple Silicon)
- DMG created via hdiutil with UDZO compression

### Platform notes

- Xapian's xapian.h defines `slots`/`signals` macros that conflict with
  Qt; resolved by undefining before including Qt headers
- QtWebEngineProcess helper needs library symlinks (7 levels relative)
  to find Qt frameworks at runtime
- Apple Silicon requires ad-hoc code signing on all executables and
  frameworks after any modification
