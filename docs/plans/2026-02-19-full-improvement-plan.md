# 2026-02-19 Full Improvement Plan

## Scope
- Address three review findings only:
- C++ tests not wired to CTest/CI.
- Potential indefinite wait in close worker (`future.get()` without timeout).
- Overuse of silent `catch(...)` in request/settings payload parsing.

## Tasks

1. CMake test wiring for C++ tests
- Change:
- Add `enable_testing()` in root `CMakeLists.txt`.
- Discover `tests/*.cpp`, create one executable per test file, and register each with `add_test(...)`.
- Ensure cross-compiled tests are runnable from CTest in WSL by invoking via `wine` when cross-compiling to Windows.
- Acceptance criteria:
- `ctest --test-dir build/wsl-release` lists all C++ tests instead of `No tests were found`.
- Each test target builds successfully.
- CTest execution succeeds in this environment.

2. Close worker bounded wait and fallback safety
- Change:
- In `src/PrismaUIViewWorkers.cpp`, replace unbounded `future.get()` in `QueueCloseRetry` with `future.wait_for(...)` timeout.
- On timeout or future exception, log diagnostic context and trigger safe fallback (`QueueForceHideFocusMenu`, `QueueHideSkyrimCursor`) without risking indefinite block.
- Keep retry behavior compatible and avoid UI-thread deadlock amplification.
- Acceptance criteria:
- No code path in close retry thread blocks forever waiting on future completion.
- Timeout/exception paths are logged and recover by forcing focus/cursor cleanup.
- Normal close path behavior remains unchanged when UI task completes on time.

3. Replace silent `catch(...)` with typed, diagnostic handling
- Change:
- In `src/PrismaUISettingsPayload.cpp` and `src/PrismaUIRequestOps.cpp`, replace broad silent catches with typed exceptions (`nlohmann::json` exceptions and `std::exception` where useful).
- Add concise logs that identify operation and error.
- Preserve existing user behavior (same toasts/fallback outputs) and avoid repeated noisy toasts.
- Acceptance criteria:
- Targeted files no longer contain silent `catch(...)` swallowing parse/type errors.
- Parse failures produce actionable log messages.
- Existing API behavior remains safe (invalid payloads return defaults/nullopt and existing toasts still fire where already defined).

4. Test updates for touched logic
- Change:
- Adjust existing C++ test coverage by wiring all `tests/*.cpp` into CTest so they execute in CI/validation.
- Keep parser/runtime changes behavior-compatible; rely on existing unit coverage plus runtime-safe fallback semantics for this surgical pass.
- Acceptance criteria:
- Existing C++ tests pass under CTest after wiring.
- Required validations run successfully with no regressions.

5. Validation and reporting
- Change:
- Run required validations:
- `node --test tests/*.cjs`
- `ctest` from configured build directory after wiring/build.
- Produce final report at `docs/plans/2026-02-19-full-improvement-report.md` summarizing:
- files changed
- rationale per change
- commands executed + pass/fail results
- Acceptance criteria:
- Both required validation commands are executed and results recorded.
- Report file exists with complete summary.
