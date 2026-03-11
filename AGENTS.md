# BUILD AND TEST GUIDE

## OVERVIEW
CodexOfPowerNG is a Skyrim SKSE plugin with a Prisma UI web overlay.
Use this file as the quick reference for local setup, build, test, and packaging commands.

## COMMANDS
```bash
python3 scripts/vibe.py doctor --full

export VCPKG_ROOT="/mnt/c/Users/<user>/vcpkg"
cmake --preset wsl-release
cmake --build --preset wsl-release
cmake --install build/wsl-release

scripts/test.sh
node --test tests/*.test.cjs

cd dist/CodexOfPowerNG
zip -r -FS "../../releases/Codex of Power NG.zip" .
```

## NOTES
- WSL build uses the `wsl-release` preset.
- If `VCPKG_ROOT` mismatch errors appear, clear `build/wsl-release/` and reconfigure.
- Runtime settings are layered as `settings.json` plus `settings.user.json`.
- Release notes live under `docs/releases/` and should remain Korean, while `CHANGELOG.md` stays English.
- TypeScript language server is not installed here; JS verification relies on Node tests.
- `scripts/test.sh` is the fast host-safe regression gate; use `ctest --test-dir build/wsl-release --output-on-failure` for the full WSL/native test sweep.
