#!/usr/bin/env bash
set -euo pipefail

DEFAULT_LOG="${HOME}/Documents/My Games/Skyrim Special Edition/SKSE/CodexOfPowerNG.log"
LOG_PATH="${1:-$DEFAULT_LOG}"

if [[ ! -f "$LOG_PATH" ]]; then
  echo "[copng] log file not found: $LOG_PATH" >&2
  echo "[copng] usage: $0 [/path/to/CodexOfPowerNG.log]" >&2
  exit 1
fi

echo "[copng] scanning load-boundary cleanup markers in: $LOG_PATH"
PATTERN='PreLoadGame: forced PrismaView destroy|PreLoadGame: failed to queue UI cleanup task|Close: queued force-hide PrismaUI_FocusMenu|Close: queued MenuCursor hide'

if ! rg -n "$PATTERN" "$LOG_PATH"; then
  echo "[copng] no preload-cleanup markers found"
fi

if rg -q "PreLoadGame: failed to queue UI cleanup task" "$LOG_PATH"; then
  echo "[copng][warn] detected UI cleanup queue failure during preload; verify task interface timing."
  exit 2
fi

echo "[copng] preload cleanup marker scan complete"

