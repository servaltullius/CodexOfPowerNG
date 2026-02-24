# RELEASE NOTES KNOWLEDGE BASE

## OVERVIEW
`docs/releases/` stores versioned release notes and strict Korean release-note policy for CodexOfPowerNG.

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Policy source of truth | `docs/releases/PATCH_NOTE_RULES.ko.md` | required sections, language, forbidden claims |
| Stable release notes | `docs/releases/vX.Y.Z.md` | final tagged release notes |
| Pre-release notes | `docs/releases/vX.Y.Z-rc.N.md` | RC notes tied to pre-release tags |
| Nexus upload logs | `docs/releases/*-nexus-*.md` | upload/accounting history |

## CONVENTIONS
- Default release-note language is Korean.
- Title format: `# Codex of Power NG vX.Y.Z (YYYY-MM-DD)`; pre-release titles include `(Pre-release)`.
- Required sections stay in fixed order: summary, major changes, user impact, technical detail, verification, compatibility/cautions, package.
- Filename/version/tag must match exactly (`vX.Y.Z.md` or `vX.Y.Z-rc.N.md`).
- Verification section must list actually executed commands and outcomes.

## ANTI-PATTERNS
- Do not report tests you did not run.
- Do not publish unverified performance numbers or bug-fix rates.
- Do not describe untouched areas as changed.
- Do not ship release-note files with version/title/tag mismatches.

## NOTES
- Keep `CHANGELOG.md` language policy separate (English changelog, Korean release notes).
- Before publish, cross-check release asset path: `releases/Codex of Power NG.zip`.
- Include compatibility notices for legacy residue cleanup when relevant.
- Keep pre-release notes explicit about RC status in title and summary.
- Treat `PATCH_NOTE_RULES.ko.md` as the canonical checklist, not a suggestion.
