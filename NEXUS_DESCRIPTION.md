# Codex of Power NG (CodexOfPowerNG)

> **New mod. Not compatible with Codex of Power / SVCollection. No save migration.**

Codex of Power NG is an **ESP-less** SKSE plugin with a **Prisma UI** menu.  
Register items (consumes **1** of that item) and manage settings safely and quickly.

## Key points
- **No ESP/ESM** required (SKSE DLL + Prisma UI view only)
- UI is **Prisma UI (Web UI)** (no SkyUI/MCM/UIExtensions menu)
- Toggle menu with **F4** (default)
- Built for **Skyrim SE 1.5.97 + AE 1.6+** (CommonLibSSE-NG multi-target)

## Requirements
- SKSE for your game version
- Prisma UI (`PrismaUI.dll`)
- Address Library for SKSE Plugins
- Media Keys Fix SKSE

## Install (MO2)
Install the archive and ensure these paths exist:
- `SKSE/Plugins/CodexOfPowerNG.dll`
- `PrismaUI/views/codexofpowerng/index.html`

## Upgrade Note (from Codex of Power / SVCollection)
- NG is not compatible with legacy Codex/SVCollection runtime state.
- Remove stale legacy MCM files from your active Data path (MO2: usually `overwrite`):
  - `MCM/Settings/SVCollection.ini`
  - `MCM/Settings/keybinds.json` entry where `modName` is `SVCollection`
- NG logs a warning if legacy residue is detected at runtime.

## Notes
- This mod **consumes items** when registering. Use safety settings (favorites protection, exclude list).
- High DPI (Windows scaling 175–200%): use the UI **Input scale** setting if clicks don’t match the cursor.
- If cursor/hover feels choppy while the menu is open: enable **Performance mode (Smooth cursor)** in Settings.
