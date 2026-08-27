# openBibleViewer

openBibleViewer is a Bible study application for reading, comparing, and
searching Bible translations. Originally written by Paul Walger in C++/Qt,
this is a fork that ports the application to **macOS on Apple Silicon**
using **Qt 6**.

## What this fork does

The original openBibleViewer targeted Linux and Windows using Qt 5,
QtWebKit, and CLucene. None of these are available on Apple Silicon
macOS. This fork replaces the entire dependency stack:

| Component | Original | This fork |
|-----------|----------|-----------|
| UI framework | Qt 5 | Qt 6.6+ |
| Web rendering | QtWebKit | Qt6WebEngineWidgets |
| Full-text search | CLucene | Xapian 1.4 |
| Build system | CMake + qmake | CMake 3.16+ |

The result is a native Apple Silicon app that builds with Homebrew
dependencies and packages into a distributable DMG.

## Building

```bash
# Install dependencies
brew install cmake qt@6 xapian zlib

# Build (creates openBibleViewer.app)
bash package.sh

# Or build manually:
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j$(sysctl -n hw.ncpu)
open openBibleViewer.app
```

The `package.sh` script handles the full pipeline: CMake configure, build,
`macdeployqt` (bundles Qt frameworks), Xapian library bundling,
QtWebEngineProcess helper setup, ad-hoc codesigning, and DMG creation.

Output: `dist/openBibleViewer-0.9.1-macOS-arm64.dmg`

## Requirements

- macOS 13+ (Apple Silicon / arm64)
- Qt 6.6+: Core, Gui, Widgets, Xml, Network, Sql, PrintSupport,
  WebEngineWidgets, Qml, Core5Compat
- Xapian 1.4+
- zlib
- CMake 3.16+

## What changed from the original

The port touched every layer of the application:

### Build system

- Migrated CMake build from Qt 5 / QtWebKit to Qt 6 / QtWebEngineWidgets
- Added `Core5Compat` bridge for QTextCodec and other removed Qt 5 APIs
- Replaced CLucene full-text search engine with Xapian 1.4
- Added `FindXapian.cmake`, `xapian_wrapper.h` (isolates Xapian's
  `signals`/`slots` keyword conflicts with Qt)
- Disabled `CMAKE_AUTOUIC` to coexist with explicit `qt6_wrap_ui()`
- Added `package.sh` for one-command macOS packaging

### Qt 5 to Qt 6 API migration

- QScriptEngine → QJSEngine (web dictionary/commentary modules)
- QTextCodec removed from Qt6Core; using Qt6::Core5Compat
- QWebEnginePage: `NavigateBack`/`NavigateForward` → `Back`/`Forward`
- `setUserStyleSheetUrl()` removed; replaced with QWebEngineScript
  JavaScript injection for custom CSS
- QRegExp → QRegularExpression
- QStringRef → QStringView
- qSort → std::sort
- QModelIndex::child() removed; using sourceModel()->index()
- QSortFilterProxyModel: filterRegExp → filterRegularExpression
- QHash::unite() → insert()
- Qt::CTRL+Qt::SHIFT → Qt::CTRL|Qt::SHIFT (keyboard shortcuts)
- QMapIterator: use constBegin/constEnd
- QWebEngineView forward declaration fix
- QSet construction from QList: use iterator constructor
- QSet::toList() → .values()
- QLayout::setMargin → setContentsMargins
- qOverload needed for overloaded signal connections

### Bug fixes

- **Book selector**: `QHash::keys()`/`values()` don't guarantee matching
  order in Qt6; now sorted by book ID
- **Verse highlighting**: `m_lastTextRanges` set before `showText()` so
  scroll-to-anchor works; deferred to `loadFinished` signal (Qt6
  `setHtml()` is async)
- **QtWebEngineProcess helper**: relative symlinks to main bundle
  Frameworks for proper library resolution on Apple Silicon
- **Ad-hoc codesigning**: required on Apple Silicon for all executables

### Packaging

- Hardcoded `openBibleViewer.icns` icon in Info.plist.in
- Icon upscaled from 124x124 source to all macOS icon sizes
- Ad-hoc codesigning for all binaries
- DMG created via `hdiutil` with UDZO compression

## Platform notes

- Xapian's `xapian.h` defines `signals`/`slots` macros that conflict
  with Qt; resolved by `#undef` before including Qt headers
- QtWebEngineProcess helper needs 7 levels of relative symlinks to
  find Qt frameworks at runtime
- Apple Silicon requires ad-hoc code signing on all executables and
  frameworks after any modification

## License

GPLv3 — see [LICENSE](LICENSE) file.

## Credits

Original project by Paul Walger (metaxy).
macOS/Qt6 port by Matt Briggs.
