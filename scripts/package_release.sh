#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DEFAULT_OUTPUT="$ROOT_DIR/releases/Codex of Power NG.zip"

if ! command -v zip >/dev/null 2>&1; then
  echo "[copng] zip not found (required for release packaging)" >&2
  exit 1
fi

STAGE_ARG="${1:-}"
OUTPUT_ZIP="${2:-$DEFAULT_OUTPUT}"
STAGE_DIR=""
TMP_STAGE=""

if [[ -n "$STAGE_ARG" ]]; then
  STAGE_DIR="$STAGE_ARG"
  if [[ ! -d "$STAGE_DIR" ]]; then
    echo "[copng] stage directory not found: $STAGE_DIR" >&2
    exit 1
  fi
else
  DLL_PATH="$ROOT_DIR/dist/CodexOfPowerNG/SKSE/Plugins/CodexOfPowerNG.dll"
  if [[ ! -f "$DLL_PATH" ]]; then
    echo "[copng] built DLL not found: $DLL_PATH" >&2
    echo "[copng] pass a prepared stage directory as the first argument or build/install first." >&2
    exit 1
  fi

  TMP_STAGE="$(mktemp -d)"
  cleanup() {
    [[ -n "$TMP_STAGE" ]] && rm -rf "$TMP_STAGE"
  }
  trap cleanup EXIT

  STAGE_DIR="$TMP_STAGE"
  mkdir -p "$STAGE_DIR/SKSE/Plugins" "$STAGE_DIR/PrismaUI/views"
  cp "$DLL_PATH" "$STAGE_DIR/SKSE/Plugins/CodexOfPowerNG.dll"
  cp -R "$ROOT_DIR/SKSE/Plugins/CodexOfPowerNG" "$STAGE_DIR/SKSE/Plugins/"
  cp -R "$ROOT_DIR/PrismaUI/views/codexofpowerng" "$STAGE_DIR/PrismaUI/views/"
fi

if [[ ! -f "$STAGE_DIR/SKSE/Plugins/CodexOfPowerNG.dll" ]]; then
  echo "[copng] stage is missing SKSE/Plugins/CodexOfPowerNG.dll" >&2
  exit 1
fi
if [[ ! -f "$STAGE_DIR/PrismaUI/views/codexofpowerng/index.html" ]]; then
  echo "[copng] stage is missing PrismaUI/views/codexofpowerng/index.html" >&2
  exit 1
fi

mkdir -p "$(dirname "$OUTPUT_ZIP")"
rm -f "$OUTPUT_ZIP"

(
  cd "$STAGE_DIR"
  zip -r -FS "$OUTPUT_ZIP" .
)

echo "[copng] packaged release: $OUTPUT_ZIP"
