const { test } = require("node:test");
const assert = require("node:assert/strict");

let kc = null;
try {
  // This module is used by the in-game PrismaUI view.
  kc = require("../PrismaUI/views/codexofpowerng/keycodes.js");
} catch {
  kc = null;
}

test("keycodes module exists", () => {
  assert.ok(kc, "Expected PrismaUI/views/codexofpowerng/keycodes.js to exist and export functions");
});

test("parseKeybindInput supports hex DIK values", () => {
  assert.equal(kc.parseKeybindInput("0x3E"), 0x3e);
  assert.equal(kc.parseKeybindInput("0X3e"), 0x3e);
});

test("parseKeybindInput supports decimal DIK values", () => {
  assert.equal(kc.parseKeybindInput("62"), 0x3e);
});

test("parseKeybindInput supports key names (F4/E/1)", () => {
  assert.equal(kc.parseKeybindInput("F4"), 0x3e);
  assert.equal(kc.parseKeybindInput("f4"), 0x3e);
  assert.equal(kc.parseKeybindInput("E"), 0x12);
  assert.equal(kc.parseKeybindInput("e"), 0x12);
  assert.equal(kc.parseKeybindInput("1"), 0x02);
});

test("dikToKeyName formats known keys", () => {
  assert.equal(kc.dikToKeyName(0x3e), "F4");
  assert.equal(kc.dikToKeyName(0x12), "E");
  assert.equal(kc.dikToKeyName(0x02), "1");
});

test("jsCodeToDik maps common KeyboardEvent.code values", () => {
  assert.equal(kc.jsCodeToDik("F4"), 0x3e);
  assert.equal(kc.jsCodeToDik("KeyE"), 0x12);
  assert.equal(kc.jsCodeToDik("Digit1"), 0x02);
});

test("dikToHex returns compact DIK hex", () => {
  assert.equal(kc.dikToHex(0x3e), "0x3E");
  assert.equal(kc.dikToHex(0x12), "0x12");
  assert.equal(kc.dikToHex(0x02), "0x02");
});

test("formatDikDisplay prefers key name when known", () => {
  assert.equal(kc.formatDikDisplay(0x3e), "F4 (0x3E)");
  assert.equal(kc.formatDikDisplay(0x12), "E (0x12)");
});
