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
  "normalizeRegistration": false,
  "protectFavorites": true,
  "enableLootNotify": true,
  "enableRewards": true,
  "rewardEvery": 5,
  "rewardMultiplier": 1.0,
  "allowSkillRewards": false
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

### `window.copng_refundRewards(payloadJson)`
Refunds recorded rewards (does not restore consumed items; registrations remain).

Payload:
```json
{}
```

## C++ → JS

### `window.copng_setState(jsonOrString)`
Called by native code via `Invoke()` (or `InteropCall()` for hot paths).

Example payload:
```json
{
  "ui": { "ready": true, "focused": false, "hidden": true },
  "registeredCount": 3,
  "rewardCount": 12,
  "language": "en",
  "toggleKeyCode": 62
}
```

### `window.copng_setInventory(jsonOrString)`
Quick-register inventory list (safe-to-consume entries only).

Example:
```json
[
  {
    "formId": 46775,
    "regKey": 46775,
    "name": "Iron Sword",
    "group": 0,
    "groupName": "Weapons",
    "totalCount": 3,
    "safeCount": 2
  }
]
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

### `window.copng_setSettings(jsonOrString)`
Settings snapshot as JSON.

### `window.copng_toast(jsonOrString)`
Non-blocking UI notification.

Example:
```json
{ "level": "info|error|warn", "message": "..." }
```
