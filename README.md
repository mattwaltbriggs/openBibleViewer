# openBibleViewer

openBibleViewer is a Bible study application for reading, comparing, and
searching Bible translations. It supports Zefania XML, BibleQuote,
TheWord, ESword, MySword, RTF, PDF, and plain text formats.

This fork ports the application to **macOS on Apple Silicon** (arm64)
using **Qt 6** and **Xapian** for full-text search.

## Features

- Simple and advanced (MDI) interfaces
- Compare multiple Bible translations side by side
- Full-text search via Xapian
- Verse notes, bookmarks, and highlighting
- Strong's numbers and Robinson's morphological codes
- Print and export to plain text / HTML
- 180+ Zefania XML modules, 200+ BibleQuote modules

## Requirements

- macOS 13+ (Apple Silicon)
- Qt 6.6+ (Core, Gui, Widgets, Xml, Network, Sql, PrintSupport,
  WebEngineWidgets, Qml, Core5Compat)
- Xapian 1.4+
- zlib
- CMake 3.16+

## Building

```bash
# Install dependencies
brew install cmake qt@6 xapian zlib

# Build
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j$(sysctl -n hw.ncpu)

# Run
open openBibleViewer.app
```

## Creating a DMG

```bash
bash package.sh
# Output: dist/openBibleViewer-0.9.1-macOS-arm64.dmg
```

## Changes from upstream

This fork makes the following changes to the original
[openBibleViewer](https://github.com/metaxy/openBibleViewer) project:

- **Qt 5 → Qt 6**: Full migration including WebKit → WebEngine,
  QScriptEngine → QJSEngine, and dozens of deprecated API replacements
- **CLucene → Xapian**: Full-text search engine replaced for Apple
  Silicon compatibility
- **macOS packaging**: App bundle creation, QtWebEngineProcess helper
  setup, ad-hoc codesigning, and DMG generation
- **Bug fixes**: Book selector ordering, async scroll-to-anchor,
  verse highlighting in search results

See [CHANGELOG.md](CHANGELOG.md) for full details.

## License

GPLv3 — see [LICENSE](LICENSE) file.

## Credits

Original project by Paul Walger (metaxy).
macOS port by Matt Briggs.
