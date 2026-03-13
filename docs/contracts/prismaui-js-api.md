# Prisma UI JS API Contract (CodexOfPowerNG)

## Naming
- All JS → C++ bindings are exposed by Prisma UI as global functions: `window.<functionName>`
- CodexOfPowerNG uses the prefix: `copng_`
- Payloads are **UTF-8 JSON strings** unless stated otherwise.

## View path
- Prisma UI base directory: `Data/PrismaUI/views/`
- CodexOfPowerNG view: `Data/PrismaUI/views/codexofpowerng/index.html`
- C++ uses: `CreateView("codexofpowerng/index.html")`

## JS → C++ (RegisterJSListener)

### `window.copng_log(payloadJson)`
Used for JS-side diagnostics.

Payload:
```json
{ "level": "info|warn|error|debug", "message": "string" }
```

### `window.copng_requestState(payloadJson)`
Requests the plugin to send current state back to the UI via `window.copng_setState(...)`.

Payload:
```json
{}
```

### `window.copng_requestToggle(payloadJson)`
Requests native plugin to toggle UI focus/visibility (same as hotkey).

Payload:
```json
{}
```

### `window.copng_requestInventory(payloadJson)`
Requests the current **quick-register** inventory list via `window.copng_setInventory(...)`.

Payload:
```json
{}
```

### `window.copng_requestRegistered(payloadJson)`
Requests the **registered** list via `window.copng_setRegistered(...)`.

Payload:
```json
{}
```

### `window.copng_requestRewards(payloadJson)`
Requests reward totals via `window.copng_setRewards(...)`.

Payload:
```json
{}
```

### `window.copng_requestBuild(payloadJson)`
Requests build summary, option catalog state, and active slots via `window.copng_setBuild(...)`.

Payload:
```json
{}
```

### `window.copng_requestUndoList(payloadJson)`
Requests recent register-undo candidates via `window.copng_setUndoList(...)`.

Payload:
```json
{}
```

### `window.copng_getSettings(payloadJson)`
Requests current settings via `window.copng_setSettings(...)`.

Payload:
```json
{}
```

### `window.copng_saveSettings(payloadJson)`
Saves settings to `Data/SKSE/Plugins/CodexOfPowerNG/settings.user.json` and reloads localization.
On load, NG applies settings in this order:
1. `settings.json` (distributed defaults/template)
2. `settings.user.json` (user override)

Payload (all fields optional; omitted fields keep current value):
```json
{
  "toggleKeyCode": 62,
  "languageOverride": "auto|en|ko",
  "uiPauseGame": true,
  "uiDisableFocusMenu": false,
  "uiDestroyOnClose": true,
  "uiInputScale": 1.0,
  "normalizeRegistration": false,
  "requireTccDisplayed": false,
  "protectFavorites": true,
  "enableLootNotify": true
}
```

### `window.copng_registerItem(payloadJson)`
Attempts to register an inventory item and consumes **1** of it safely.

Payload:
```json
{ "formId": 46775 }
```

Notes:
- `formId` may be a number or a string (`"0x00012EB7"`).
- On failure, the plugin returns an error toast and the item is not consumed.

### `window.copng_requestRegisterBatch(payloadJson)`
Attempts to register multiple inventory rows in order.

Payload:
```json
{ "formIds": [46775, 51234] }
```

Notes:
- Each array entry represents exactly one requested registration row.
- Native code currently preserves array order and does not collapse rows before processing.

### `window.copng_activateBuildOption(payloadJson)`
Attempts to activate a build option into a slot.

Payload:
```json
{ "optionId": "build.attack.ferocity", "slotId": "attack_1" }
```

### `window.copng_deactivateBuildOption(payloadJson)`
Attempts to clear one active build slot.

Payload:
```json
{ "slotId": "attack_1" }
```

### `window.copng_swapBuildOption(payloadJson)`
Attempts to move an already active option between slots.

Payload:
```json
{
  "optionId": "build.attack.ferocity",
  "fromSlotId": "attack_1",
  "toSlotId": "wildcard_1"
}
```

Notes:
- Build activation/deactivation/swap requests are combat-locked.

### `window.copng_undoRegisterItem(payloadJson)`
Attempts to undo the latest registration action.

Payload:
```json
{ "actionId": 42 }
```

Notes:
- Safety policy: only the latest action can be undone (LIFO).
- Undo restores 1 consumed item and rolls back build-progression contribution recorded for that action.
- If item restore succeeds but no remaining progression delta can be applied, the success message may include a warning suffix (`[warning: reward rollback not applied]` / `[경고: 보상 롤백이 적용되지 않음]`).

### `window.copng_refundRewards(payloadJson)`
Refunds recorded rewards (does not restore consumed items; registrations remain).

Payload:
```json
{}
```

## C++ → JS

## Refresh Contract
- `copng_registerItem` success/failure presentation invalidates:
  - `copng_setState`
  - `copng_setInventory`
  - `copng_setRegistered`
  - `copng_setBuild`
  - `copng_setRewards`
  - `copng_setUndoList`
- `copng_requestRegisterBatch` follows the same invalidation set as single register.
- `copng_undoRegisterItem` follows the same invalidation set as register.
- `copng_activateBuildOption`, `copng_deactivateBuildOption`, and `copng_swapBuildOption` invalidate:
  - `copng_setState`
  - `copng_setBuild`
- `copng_refundRewards` currently invalidates:
  - `copng_setState`
  - `copng_setRewards`
- If the main-thread queue is temporarily unavailable during a refresh-producing operation, native code marks a pending refresh and replays the full core refresh on the next state/inventory request.
- UI code should treat these refreshes as idempotent and should not assume only one panel updates at a time.

### `window.copng_setState(jsonOrString)`
Called by native code via `Invoke()` (or `InteropCall()` for hot paths).

Example payload:
```json
{
  "ui": { "ready": true, "focused": false, "hidden": true },
  "registeredCount": 3,
  "rewardCount": 12,
  "buildSummary": { "attackScore": 4, "defenseScore": 3, "utilityScore": 8, "totalScore": 15 },
  "buildMigration": { "state": 2, "version": 1, "needsNotice": false, "legacyRewardsMigrated": false, "unresolvedHistoricalRegistrations": 0 },
  "language": "en",
  "toggleKeyCode": 62
}
```

### `window.copng_setInventory(jsonOrString)`
Quick-register inventory list (register-relevant entries, including temporarily protected disabled rows).
Implementation note: native side may serve this from a short-lived internal cache.

Example:
```json
{
  "page": 0,
  "pageSize": 200,
  "total": 1,
  "hasMore": false,
  "items": [
    {
      "formId": 46775,
      "regKey": 46775,
      "name": "Iron Sword",
      "group": 0,
      "groupName": "Weapons",
      "totalCount": 3,
      "safeCount": 2
    }
  ],
  "sections": [
    {
      "group": 0,
      "groupName": "Weapons",
      "discipline": "attack",
      "rows": [
        {
          "formId": 46775,
          "regKey": 46775,
          "name": "Iron Sword",
          "discipline": "attack",
          "totalCount": 3,
          "safeCount": 2,
          "actionable": true,
          "disabledReason": null
        },
        {
          "formId": 51234,
          "regKey": 51234,
          "name": "Quest Blade",
          "discipline": "attack",
          "totalCount": 1,
          "safeCount": 0,
          "actionable": false,
          "disabledReason": "quest_protected"
        }
      ]
    }
  ]
}
```

### `window.copng_setRegistered(jsonOrString)`
Registered list.

Example:
```json
[
  { "formId": 46775, "name": "Iron Sword", "group": 0, "groupName": "Weapons" }
]
```

### `window.copng_setRewards(jsonOrString)`
Rewards summary + totals.

Example:
```json
{
  "registeredCount": 30,
  "rewardEvery": 5,
  "rewardMultiplier": 1.0,
  "rolls": 3,
  "totals": [
    { "label": "Health", "total": 4.0, "format": "raw", "display": "+4.00" }
  ]
}
```

### `window.copng_setBuild(jsonOrString)`
Build summary, unlock state, and active slot payload.

Example:
```json
{
  "disciplines": {
    "attack": { "score": 12, "currentTier": 1, "nextTierScore": 20, "scoreToNextTier": 8 },
    "defense": { "score": 4, "currentTier": 0, "nextTierScore": 10, "scoreToNextTier": 6 },
    "utility": { "score": 7, "currentTier": 0, "nextTierScore": 10, "scoreToNextTier": 3 }
  },
  "options": [
    {
      "id": "build.attack.ferocity",
      "discipline": "attack",
      "layer": "slotted",
      "unlockScore": 5,
      "unlocked": true,
      "slotCompatibility": "same_or_wildcard",
      "effectType": "actor_value",
      "effectKey": "attack_damage_mult",
      "magnitude": 6.0,
      "baseMagnitude": 5.0,
      "magnitudePerTier": 1.0,
      "currentMagnitude": 6.0,
      "nextMagnitude": 7.0,
      "currentTier": 1,
      "nextTierScore": 20,
      "scoreToNextTier": 8
    }
  ],
  "activeSlots": [
    { "slotId": "attack_1", "slotKind": "attack", "optionId": "build.attack.ferocity", "occupied": true }
  ],
  "migrationNotice": {
    "needsNotice": false,
    "legacyRewardsMigrated": false,
    "unresolvedHistoricalRegistrations": 0
  }
}
```

### `window.copng_setUndoList(jsonOrString)`
Recent undo list (newest first).

Example:
```json
[
  {
    "actionId": 42,
    "formId": 46775,
    "regKey": 46775,
    "name": "Iron Sword",
    "group": 0,
    "groupName": "Weapons",
    "canUndo": true,
    "hasRewardDelta": false
  }
]
```

### `window.copng_setSettings(jsonOrString)`
Settings snapshot as JSON.

### `window.copng_toast(jsonOrString)`
Non-blocking UI notification.

Example:
```json
{ "level": "info|error|warn", "message": "..." }
```
