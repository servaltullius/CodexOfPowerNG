# Codex of Power NG — Changelog

This changelog tracks **Codex of Power NG (CodexOfPowerNG)** only.  
It is **not** a continuation of Codex of Power / SVCollection.

## Unreleased
- **UI (Prisma UI):** Web UI menu (`PrismaUI/views/codexofpowerng/index.html`) with English/Korean UI text + language override.
- **Performance:** Paginated inventory list + virtualized tables (renders only visible rows) to reduce cursor/hover stutter while the menu is open.
- **High DPI:** Input hitbox correction via `Input scale` (175% → 1.75, 200% → 2.0) + emergency key controls (`[`/`]`, `0`, `ESC`/`F4`).
- **UX:** UI auto scaling for 4K, manual UI scale override, smoother wheel scrolling, larger language dropdown text.
- **Stability:** Settings save + localization reload are queued to reduce frame hitches.

