#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ZIP_PATH="${1:-$ROOT_DIR/releases/Codex of Power NG.zip}"

if ! command -v unzip >/dev/null 2>&1; then
  echo "[copng] unzip not found (required for release zip validation)" >&2
  exit 1
fi

if [[ ! -f "$ZIP_PATH" ]]; then
  echo "[copng] release zip not found: $ZIP_PATH" >&2
  exit 1
fi

mapfile -t archive_entries < <(unzip -Z1 "$ZIP_PATH")

required_entries=(
  "SKSE/Plugins/CodexOfPowerNG.dll"
  "SKSE/Plugins/CodexOfPowerNG/settings.json"
  "SKSE/Plugins/CodexOfPowerNG/exclude_map.json"
  "SKSE/Plugins/CodexOfPowerNG/exclude_user.json"
  "SKSE/Plugins/CodexOfPowerNG/variant_map.json"
  "SKSE/Plugins/CodexOfPowerNG/lang/en.json"
  "SKSE/Plugins/CodexOfPowerNG/lang/ko.json"
  "PrismaUI/views/codexofpowerng/index.html"
  "PrismaUI/views/codexofpowerng/keycodes.js"
  "PrismaUI/views/codexofpowerng/lang_ui.js"
  "PrismaUI/views/codexofpowerng/ui_i18n.js"
  "PrismaUI/views/codexofpowerng/ui_state.js"
  "PrismaUI/views/codexofpowerng/ui_interactions.js"
  "PrismaUI/views/codexofpowerng/ui_bootstrap.js"
  "PrismaUI/views/codexofpowerng/ui_rendering.js"
  "PrismaUI/views/codexofpowerng/ui_wiring.js"
  "PrismaUI/views/codexofpowerng/native_state_bridge.js"
  "PrismaUI/views/codexofpowerng/native_bridge_bootstrap.js"
  "PrismaUI/views/codexofpowerng/interop_bridge.js"
  "PrismaUI/views/codexofpowerng/input_shortcuts.js"
  "PrismaUI/views/codexofpowerng/input_correction.js"
  "PrismaUI/views/codexofpowerng/reward_orbit.js"
  "PrismaUI/views/codexofpowerng/virtual_tables.js"
  "PrismaUI/views/codexofpowerng/assets/character.png"
)

missing=0
for entry in "${required_entries[@]}"; do
  if ! printf '%s\n' "${archive_entries[@]}" | grep -Fxq "$entry"; then
    echo "[copng] missing release entry: $entry" >&2
    missing=1
  fi
done

if (( missing != 0 )); then
  exit 1
fi

echo "[copng] release zip OK: $ZIP_PATH"
