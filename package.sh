#!/bin/bash
set -e

APP_NAME="openBibleViewer"
VERSION="0.9.1"
BUILD_DIR="build"
APP_BUNDLE="${BUILD_DIR}/${APP_NAME}.app"
CONTENTS="${APP_BUNDLE}/Contents"
DMG_NAME="${APP_NAME}-${VERSION}-macOS-arm64"
DIST_DIR="dist"

echo "=== openBibleViewer macOS Packager ==="
echo "Target: Apple Silicon (arm64)"
echo ""

# Clean previous builds
echo "[1/8] Cleaning previous builds..."
rm -rf "${BUILD_DIR}"
rm -rf "${DIST_DIR}"
mkdir -p "${DIST_DIR}"

# Configure
echo "[2/8] Configuring CMake (Release)..."
cmake -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release

# Build
echo "[3/8] Building..."
cmake --build "${BUILD_DIR}" -j$(sysctl -n hw.ncpu)

# Copy icon into app bundle
echo "[4/8] Installing icon..."
mkdir -p "${CONTENTS}/Resources"
mkdir -p "${CONTENTS}/Frameworks"
cp src/icons/openBibleViewer.icns "${CONTENTS}/Resources/"

# Run macdeployqt to bundle Qt frameworks (creates Frameworks/ dir)
echo "[5/8] Running macdeployqt..."
macdeployqt "${APP_BUNDLE}" -verbose=1

# Copy Xapian library into bundle after macdeployqt has created Frameworks/
echo "[6/8] Bundling Xapian..."
XAPIAN_LIB=$(find /opt/homebrew/lib -name "libxapian.dylib" -not -path "*/Cellar/*" 2>/dev/null | head -1)
if [ -z "${XAPIAN_LIB}" ]; then
    XAPIAN_LIB=$(find /opt/homebrew/Cellar/xapian -name "libxapian.dylib" 2>/dev/null | head -1)
fi
if [ -n "${XAPIAN_LIB}" ]; then
    XAPIAN_REAL=$(readlink -f "${XAPIAN_LIB}" 2>/dev/null || stat -f "%Y" "${XAPIAN_LIB}" 2>/dev/null)
    if [ -z "${XAPIAN_REAL}" ]; then
        XAPIAN_REAL="${XAPIAN_LIB}"
    fi
    # Make absolute if relative
    case "${XAPIAN_REAL}" in
        /*) ;;
        *) XAPIAN_REAL=$(cd "$(dirname "${XAPIAN_LIB}")" && pwd)/${XAPIAN_REAL} ;;
    esac
    echo "  Xapian lib: ${XAPIAN_REAL}"
    cp "${XAPIAN_REAL}" "${CONTENTS}/Frameworks/"
    BINARY="${CONTENTS}/MacOS/${APP_NAME}"
    chmod u+w "${BINARY}"
    install_name_tool -change "${XAPIAN_REAL}" "@executable_path/../Frameworks/libxapian.dylib" "${BINARY}" 2>/dev/null || \
    install_name_tool -change "${XAPIAN_LIB}" "@executable_path/../Frameworks/libxapian.dylib" "${BINARY}" 2>/dev/null || true
    chmod u-w "${BINARY}"
fi

# Fix library paths
echo "[7/8] Fixing library paths..."
# Rewrite all @rpath and /opt/homebrew references to use @executable_path/../Frameworks
for dylib in "${CONTENTS}/Frameworks/"*.dylib; do
    [ -f "${dylib}" ] || continue
    chmod u+w "${dylib}"
    # Fix self-referencing install names
    DNAME=$(basename "${dylib}")
    install_name_tool -id "@executable_path/../Frameworks/${DNAME}" "${dylib}" 2>/dev/null || true
done

# QtWebEngineProcess helper needs libraries too
WEBENGINE_HELPER="${CONTENTS}/Frameworks/QtWebEngineCore.framework/Versions/A/Helpers/QtWebEngineProcess.app"
if [ -d "${WEBENGINE_HELPER}" ]; then
    echo "  Creating symlinks for QtWebEngineProcess..."
    mkdir -p "${WEBENGINE_HELPER}/Contents/Frameworks"
    # Use relative symlinks so they work after install
    for item in "${CONTENTS}/Frameworks/"*; do
        DNAME=$(basename "${item}")
        # Create relative symlink: helper/Frameworks/X -> ../../../../../../Frameworks/X
        # From helper's Frameworks/ up to Contents/Frameworks/ = 7 levels
        ln -sf "../../../../../../../${DNAME}" "${WEBENGINE_HELPER}/Contents/Frameworks/${DNAME}" 2>/dev/null || true
    done
fi

# Re-link the main binary's Xapian reference (macdeployqt may have re-written it)
BINARY="${CONTENTS}/MacOS/${APP_NAME}"
chmod u+w "${BINARY}"
install_name_tool -change "${XAPIAN_REAL}" "@executable_path/../Frameworks/libxapian.dylib" "${BINARY}" 2>/dev/null || \
install_name_tool -change "${XAPIAN_LIB}" "@executable_path/../Frameworks/libxapian.dylib" "${BINARY}" 2>/dev/null || true

# Ensure rpath exists
install_name_tool -add_rpath "@executable_path/../Frameworks" "${BINARY}" 2>/dev/null || true
chmod u-w "${BINARY}"

# Re-sign everything (ad-hoc) — required on Apple Silicon
echo "[8/8] Code signing (ad-hoc)..."
# Sign frameworks first (they are dependencies)
for dylib in "${CONTENTS}/Frameworks/"*.dylib; do
    [ -f "${dylib}" ] || continue
    codesign --force --sign - "${dylib}" 2>/dev/null || true
done
for fw in "${CONTENTS}/Frameworks/"*.framework; do
    [ -d "${fw}" ] || continue
    codesign --force --deep --sign - "${fw}" 2>/dev/null || true
done
# Sign the app bundle
codesign --force --deep --sign - "${APP_BUNDLE}"

echo ""
echo "Verifying signature..."
codesign --verify --verbose "${APP_BUNDLE}" 2>&1 || true

# Create DMG
echo ""
echo "Creating DMG..."
hdiutil create -volname "${APP_NAME}" \
    -srcfolder "${APP_BUNDLE}" \
    -ov -format UDZO \
    "${DIST_DIR}/${DMG_NAME}.dmg"

echo ""
echo "=== Done! ==="
echo "App bundle: ${APP_BUNDLE}"
echo "DMG:        ${DIST_DIR}/${DMG_NAME}.dmg"
echo ""
echo "To run: open ${APP_BUNDLE}"
echo "To install: open ${DIST_DIR}/${DMG_NAME}.dmg"
