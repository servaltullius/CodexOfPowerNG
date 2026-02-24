# HEADER CONTRACT KNOWLEDGE BASE

## OVERVIEW
`include/CodexOfPowerNG/` defines public contracts shared across runtime modules and host tests.

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Runtime settings contract | `Config.h` | persisted settings schema and save/load API |
| Registration domain types | `Registration.h`, `RegistrationUndoTypes.h` | list/result/undo structures and API |
| Rewards sync math contracts | `Rewards.h`, `RewardsResync.h`, `RewardsSyncPolicy.h`, `RewardsSyncRuntime.h` | reward grant/refund/resync surface |
| Serialization contracts | `Serialization.h`, `SerializationWriteFlow.h` | plugin serialization entry + write policy |
| Inventory and rules contracts | `Inventory.h`, `RegistrationRules.h`, `RegistrationMaps.h` | filtering, grouping, map loading APIs |
| Shared runtime state | `State.h`, `Constants.h` | cross-module global data and constants |
| Task queue abstraction | `TaskScheduler.h` | queue interface used by runtime modules |

## CONVENTIONS
- Favor `[[nodiscard]]` on query/decision functions and keep `noexcept` where used by callers.
- Preserve type names used across runtime/tests (`QuickRegisterList`, `ListItem`, `UndoListItem`).
- Keep cross-module contracts minimal: headers should declare behavior, not runtime side effects.
- Maintain form-id normalization terminology (`formId`, `regKey`) consistently.

## ANTI-PATTERNS
- Do not rename established public types without coordinated updates across `src/` and `tests/`.
- Do not leak legacy module naming from Codex/SVCollection into new public contracts.
- Do not introduce heavy implementation-only dependencies into headers unless required by API.
- Do not change settings field semantics without updating disk IO and UI payload paths together.

## NOTES
- Keep contracts test-friendly: host C++ tests include these headers directly.
- If a header change affects payload/schema, update matching JSON builders in `src/PrismaUI*Payload*.cpp`.
- Reward sync helper headers (`RewardsResync.h`, `RewardsSyncPolicy.h`) are high-sensitivity regression targets.
