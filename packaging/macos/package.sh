#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${1:-build-macos}"
OUTPUT_DIR="${2:-dist/CVVerify.app}"
QT_ROOT="${CVVERIFY_QT_ROOT:-}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
BINARY_PATH="${REPO_ROOT}/${BUILD_DIR}/CVVerify.app/Contents/MacOS/CVVerify"

if [[ ! -x "${BINARY_PATH}" ]]; then
  ALT_BINARY="${REPO_ROOT}/${BUILD_DIR}/CVVerify"
  if [[ -x "${ALT_BINARY}" ]]; then
    echo "Flat binary found; creating .app bundle skeleton"
    APP_DIR="${REPO_ROOT}/${OUTPUT_DIR}"
    rm -rf "${APP_DIR}"
    mkdir -p "${APP_DIR}/Contents/MacOS" "${APP_DIR}/Contents/Resources"
    install -m 755 "${ALT_BINARY}" "${APP_DIR}/Contents/MacOS/CVVerify"
    cat > "${APP_DIR}/Contents/Info.plist" <<'PLIST'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleExecutable</key><string>CVVerify</string>
  <key>CFBundleIdentifier</key><string>com.cvverify.desktop</string>
  <key>CFBundleName</key><string>CVVerify</string>
  <key>CFBundlePackageType</key><string>APPL</string>
  <key>CFBundleShortVersionString</key><string>0.1.0</string>
  <key>CFBundleVersion</key><string>0.1.0</string>
  <key>NSHighResolutionCapable</key><true/>
</dict>
</plist>
PLIST
    BINARY_PATH="${APP_DIR}/Contents/MacOS/CVVerify"
    OUTPUT_DIR="${OUTPUT_DIR}"
  else
    echo "Executable not found: ${BINARY_PATH}" >&2
    exit 1
  fi
fi

APP_PATH="${REPO_ROOT}/${OUTPUT_DIR}"
if [[ ! -d "${APP_PATH}" ]]; then
  APP_PATH="$(dirname "$(dirname "$(dirname "${BINARY_PATH}")")")"
fi

RESOURCES_DIR="${APP_PATH}/Contents/Resources"
mkdir -p "${RESOURCES_DIR}"
rm -rf "${RESOURCES_DIR}/config" "${RESOURCES_DIR}/samples"
cp -a "${REPO_ROOT}/config" "${RESOURCES_DIR}/config"
cp -a "${REPO_ROOT}/samples" "${RESOURCES_DIR}/samples"

if [[ -z "${QT_ROOT}" ]]; then
  if command -v qmake >/dev/null 2>&1; then
    QT_ROOT="$(qmake -query QT_INSTALL_PREFIX 2>/dev/null || true)"
  fi
fi

if [[ -n "${QT_ROOT}" ]]; then
  MACDEPLOYQT="${QT_ROOT}/bin/macdeployqt"
  if [[ -x "${MACDEPLOYQT}" ]]; then
    "${MACDEPLOYQT}" "${APP_PATH}" -always-overwrite
  else
    echo "macdeployqt not found under ${QT_ROOT}/bin; bundle Qt manually if needed." >&2
  fi
else
  echo "Set CVVERIFY_QT_ROOT to run macdeployqt for Qt dependencies." >&2
fi

echo "Packaged CVVerify to ${APP_PATH}"
