# TEST SUITE KNOWLEDGE BASE

## OVERVIEW
`tests/` combines host-run C++ executable tests and Node.js source-assert tests for runtime/UI contracts.

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Unified local runner | `scripts/test.sh` | runs Node tests, compiles/runs host-safe C++ tests |
| JS module contract tests | `tests/*_module.test.cjs` | validates UI JS module exports/behavior |
| Source guard tests | `tests/*.test.cjs` | regex/static checks for C++ policy regressions |
| Rewards sync math tests | `tests/rewards_*.test.cpp`, `tests/reward_cap_normalization.test.cpp` | deterministic reward/sync calculations |
| Registration/rules tests | `tests/registration_*.test.cpp`, `tests/quick_register_rules.test.cjs` | form-id parsing and grouping constraints |
| Serialization/event tests | `tests/serialization_*.test.cpp`, `tests/events_notify_gate.test.cpp` | write/load behavior and notify gating |

## CONVENTIONS
- C++ tests are standalone executables with `main()` and `assert` checks.
- JS tests use Node built-ins (`node:test`, `node:assert/strict`) and avoid framework dependencies.
- Source-assert tests intentionally read repository files to lock critical logic patterns.
- `scripts/test.sh` skips C++ tests requiring direct `RE/` or `SKSE/` headers for host compatibility.

## ANTI-PATTERNS
- Do not claim in-game integration coverage from host-only/unit source-assert tests.
- Do not silently remove source-assert checks when production code changes; update intentfully.
- Do not add C++ tests with Skyrim header/runtime requirements without corresponding runner strategy.
- Do not hardcode platform-specific paths in tests; keep repo-relative path joins.

## NOTES
- Preferred JS entrypoint: `node --test tests/*.test.cjs`.
- Preferred full local check: `scripts/test.sh`.
- Keep assertions explicit; avoid fragile timing-based expectations.
- For policy regressions, add source-assert tests before modifying runtime behavior.
