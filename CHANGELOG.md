# Changelog

## 0.9.4 (2026-08-27) — Fix Export Selection Reference

- **Fix**: Export Selection now correctly displays the book name, chapter, and
  verse reference in the header. The previous release showed wrong values
  because `verseSelection()` used multiple async JavaScript calls that hadn't
  completed by the time the function returned. Rewrote to use a single
  `runJavaScript` call returning a JSON object with a blocking `QEventLoop`.
- **Fix**: Export Selection calls `verseSelection()` directly instead of relying
  on a stale `m_lastSelection` that was only populated by the context menu.

## 0.9.3 (2026-08-27) — Bible Quote Export

### Export / Save

- **Export Passage** (`Ctrl+Shift+S`): Export the full displayed bible passage
  with a formatted reference header (e.g., "Gen 1:1-3 (KJV)").
- **Export Selection**: Export only the user's highlighted text with its verse
  reference header.
- **Five output formats**:
  - Plain Text (`.txt`)
  - HTML (`.html`) — styled with bold reference header
  - Rich Text / Word (`.rtf`) — hand-generated RTF with proper paragraph
    breaks (`\par`), line breaks (`\line`), bold (`\b`), italic (`\i`),
    and superscript (`\super`) support; opens natively in Microsoft Word
  - WordPerfect (`.wpd`) — RTF content saved as `.wpd`; WordPerfect reads
    RTF natively
  - Open Document Text (`.odt`) — via QTextDocumentWriter; opens in
    LibreOffice, Google Docs, and Microsoft Word
- HTML-to-RTF converter strips `<style>`, `<script>`, and all structural
  tags so only verse text content appears in the output.

## 0.9.2 (2026-08-27) — Module Download & Print Fixes

### Module download and installation (fixed)

- **QProcess::start(commandString) removed in Qt6**: The `unzip` command used
  to extract downloaded Bible modules silently failed because
  `QProcess::start(const QString&)` no longer parses command strings in Qt6.
  Replaced with `QProcess::startCommand()` so module extraction actually runs.
- **Preprocessor directives in unzip**: Invalid `#elseif` (should be `#elif`)
  with Qt5-only macros (`Q_WS_X11`, `Q_WS_MAC`) replaced with
  `#elif defined(Q_OS_MACOS)` / `#elif defined(Q_OS_LINUX)`.
- **Download URLs updated**: All SourceForge module catalog URLs changed from
  HTTP to HTTPS (SourceForge now requires HTTPS).
- **Download error handling**: `DownloadInFile` now emits `finished` signal on
  file-open failure (previously halted the entire download chain silently).
- **SSL error handling**: Qt6 HTTPS connections now handled via
  `QNetworkReply::sslErrors` → `ignoreSslErrors()`.
- **Network error checking**: `DownloadInFile::finish()` checks
  `QNetworkReply::error()` before writing to disk; network failures no longer
  saved as corrupt module files.
- **Redirect handling expanded**: HTTP 303/307/308 redirects now followed in
  addition to 301/302.
- **Signal connection race fixed**: `ModuleDownloader::download()` connected
  `finished` signal *after* starting the download — moved `connect()` before
  `download()` call.
- **Failed downloads skipped**: `ModuleDownloader::save()` now checks status
  code and skips failed downloads instead of treating them as success.
- **Directory creation**: `QDir::mkdir()` replaced with `mkpath()` to create
  full directory trees.
- **User-Agent updated**: Replaced obsolete `curl/7.21.2` with
  `openBibleViewer/0.9.1`.
- **Auto-save after download**: `SettingsDialog::addModules()` now emits
  `settingsChanged` immediately after registering downloaded modules, so they
  appear without requiring the user to click OK.

### Print preview

- **Printer page size**: Removed hardcoded A4 page size; printer now uses
  the system default page size from the connected printer.
- **Async print rendering**: `QWebEngineView::print()` is async in Qt6;
  print preview now blocks via `QEventLoop` + `printFinished` signal for
  synchronous rendering.

### Bug fixes

- Printer margins set to zero to avoid double-margining with web page CSS.
- Print preview menu action removed (was causing crashes with old code path).

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
