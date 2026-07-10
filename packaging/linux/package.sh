#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${1:-build-linux}"
OUTPUT_DIR="${2:-dist/CVVerify}"
QT_ROOT="${CVVERIFY_QT_ROOT:-}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
BINARY_PATH="${REPO_ROOT}/${BUILD_DIR}/CVVerify"

if [[ ! -x "${BINARY_PATH}" ]]; then
  echo "Executable not found: ${BINARY_PATH}" >&2
  echo "Build the project first (Release recommended)." >&2
  exit 1
fi

TARGET_DIR="${REPO_ROOT}/${OUTPUT_DIR}"
rm -rf "${TARGET_DIR}"
mkdir -p "${TARGET_DIR}"

install -m 755 "${BINARY_PATH}" "${TARGET_DIR}/CVVerify"
cp -a "${REPO_ROOT}/config" "${TARGET_DIR}/config"
cp -a "${REPO_ROOT}/samples" "${TARGET_DIR}/samples"

if [[ -n "${QT_ROOT}" && -d "${QT_ROOT}/lib" ]]; then
  echo "Collecting Qt libraries from ${QT_ROOT}/lib"
  for lib in Qt5Core Qt5Gui Qt5Widgets; do
    so_path="$(find "${QT_ROOT}/lib" -maxdepth 1 -name "lib${lib}.so*" -print -quit || true)"
    if [[ -n "${so_path}" ]]; then
      cp -L "${so_path}" "${TARGET_DIR}/" || true
    fi
  done

  if [[ -d "${QT_ROOT}/plugins/platforms" ]]; then
    mkdir -p "${TARGET_DIR}/platforms"
    cp -a "${QT_ROOT}/plugins/platforms/." "${TARGET_DIR}/platforms/"
  fi
fi

if [[ -n "${CVVERIFY_OPENCV_LIB_DIR:-}" && -d "${CVVERIFY_OPENCV_LIB_DIR}" ]]; then
  echo "Copying OpenCV shared libraries from ${CVVERIFY_OPENCV_LIB_DIR}"
  cp -a "${CVVERIFY_OPENCV_LIB_DIR}"/libopencv_*.so* "${TARGET_DIR}/" 2>/dev/null || true
fi

cat > "${TARGET_DIR}/run-cvverify.sh" <<'EOF'
#!/usr/bin/env bash
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export LD_LIBRARY_PATH="${DIR}:${LD_LIBRARY_PATH:-}"
exec "${DIR}/CVVerify" "$@"
EOF
chmod +x "${TARGET_DIR}/run-cvverify.sh"

echo "Packaged CVVerify to ${TARGET_DIR}"
echo "Launch with: ${TARGET_DIR}/run-cvverify.sh"
