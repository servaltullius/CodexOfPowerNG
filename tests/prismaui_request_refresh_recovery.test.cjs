const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const requestOpsPath = path.join(__dirname, "..", "src", "PrismaUIRequestOps.cpp");
const requestsPath = path.join(__dirname, "..", "src", "PrismaUIRequests.cpp");

function read(p) {
  return fs.readFileSync(p, "utf8");
}

test("dropped UI refresh requests are marked pending and replayed", () => {
  const requestOpsSrc = read(requestOpsPath);
  const requestsSrc = read(requestsPath);

  assert.match(
    requestOpsSrc,
    /std::atomic_bool\s+g_refreshPending\{\s*false\s*\};/,
    "Request ops should keep a pending refresh flag for dropped queue sends",
  );

  assert.match(
    requestOpsSrc,
    /void FlushPendingUIRefresh\(\) noexcept[\s\S]*if \(!g_refreshPending\.exchange\(false, std::memory_order_acq_rel\)\) \{[\s\S]*return;[\s\S]*\}/,
    "FlushPendingUIRefresh should atomically consume the pending refresh flag",
  );

  assert.match(
    requestOpsSrc,
    /void FlushPendingUIRefresh\(\) noexcept[\s\S]*QueueSendInventory\(SnapshotLastInventoryRequest\(\)\);[\s\S]*QueueSendRegistered\(\);[\s\S]*QueueSendBuild\(\);[\s\S]*QueueSendRewards\(\);[\s\S]*QueueSendUndoList\(\);/,
    "Pending refresh flush should replay all core UI payload sends",
  );

  assert.match(
    requestOpsSrc,
    /g_refreshPending\.store\(true, std::memory_order_release\);/,
    "Queue-drop path should mark pending refresh",
  );

  assert.match(
    requestOpsSrc,
    /main task queue unavailable; request dropped \(pending refresh marked\)/,
    "Queue-drop log should make pending refresh behavior explicit",
  );

  assert.match(
    requestsSrc,
    /void OnJsRequestState\(const char\* \/\*argument\*\/\) noexcept[\s\S]*FlushPendingUIRefresh\(\);/,
    "State request should opportunistically flush pending refresh",
  );

  assert.match(
    requestsSrc,
    /void OnJsRequestInventory\(const char\* argument\) noexcept[\s\S]*FlushPendingUIRefresh\(\);/,
    "Inventory request should opportunistically flush pending refresh",
  );
});
