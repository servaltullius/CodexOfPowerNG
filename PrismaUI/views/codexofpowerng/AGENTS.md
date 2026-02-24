# PRISMA UI MODULE KNOWLEDGE BASE

## OVERVIEW
`PrismaUI/views/codexofpowerng/` is a single-page UI with modular JS files for interop, input handling, and virtualized lists.

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Layout/theme/tab shell | `index.html` | CSS variables, sections, controls, root wiring |
| DOM event wiring | `ui_wiring.js` | button handlers, tab switches, settings control hooks |
| Native callback bridge | `interop_bridge.js` | `copng_set*` callbacks and payload normalization |
| Keyboard shortcuts | `input_shortcuts.js` | ESC/toggle hotkey, tab keybinds, quick-register key nav |
| High-DPI correction | `input_correction.js` | coordinate correction, wheel/mouse event adjustment |
| Virtualized tables | `virtual_tables.js` | visible-slice rendering and spacer-row strategy |
| Reward visual layout | `reward_orbit.js` | reward node orbit positioning and fallback display |
| Key map + language helpers | `keycodes.js`, `lang_ui.js` | DIK mapping and language dropdown data |

## CONVENTIONS
- Keep files in IIFE module style and export both `module.exports` and `global.COPNG*` APIs.
- Preserve native callback names (`copng_setState`, `copng_setInventory`, etc.) used by C++.
- Keep table virtualization assumptions stable (fixed row heights + spacer rows).
- Use `safeCall("copng_*")` for outbound native commands from UI actions.
- Maintain keyboard recovery paths (`ESC`, toggle key, scale reset keys) for misaligned UI scenarios.

## ANTI-PATTERNS
- Do not inline all JS logic back into one giant script block; keep module boundaries intact.
- Do not break `global.COPNG*` exports; node tests rely on module-level APIs.
- Do not remove payload normalization guards for malformed/partial native JSON.
- Do not remove perf-mode/high-DPI toggles from settings flow (`ui.inputScale`, perf mode, auto/manual scale).

## NOTES
- UI module files are exercised by `tests/*_module.test.cjs`; keep public function names stable.
