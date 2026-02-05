(function (global) {
  "use strict";

  /**
   * Minimal DIK (DirectInput keyboard scan codes) mapper used by the PrismaUI view.
   *
   * - Skyrim's RE::ButtonEvent::GetIDCode() uses DIK values for keyboard events.
   * - This file provides:
   *   - parseKeybindInput("F4" | "E" | "1" | "0x3E" | "62") -> DIK code
   *   - dikToKeyName(0x3E) -> "F4"
   *   - jsCodeToDik("KeyE" | "Digit1" | "F4") -> DIK code
   *
   * Keep the surface small and dependency-free so it runs in Ultralight and in Node tests.
   */

  /** @type {Record<string, number>} */
  const NAME_TO_DIK = Object.create(null);
  /** @type {Record<number, string>} */
  const DIK_TO_NAME = Object.create(null);

  function add(name, dik) {
    const upper = String(name || "").trim().toUpperCase();
    const code = Number(dik) >>> 0;
    if (!upper || !isFinite(code)) return;
    NAME_TO_DIK[upper] = code;
    // Prefer first canonical name for each code.
    if (DIK_TO_NAME[code] == null) DIK_TO_NAME[code] = upper;
  }

  // Digits (main keyboard)
  add("1", 0x02);
  add("2", 0x03);
  add("3", 0x04);
  add("4", 0x05);
  add("5", 0x06);
  add("6", 0x07);
  add("7", 0x08);
  add("8", 0x09);
  add("9", 0x0a);
  add("0", 0x0b);

  // Letters
  add("Q", 0x10);
  add("W", 0x11);
  add("E", 0x12);
  add("R", 0x13);
  add("T", 0x14);
  add("Y", 0x15);
  add("U", 0x16);
  add("I", 0x17);
  add("O", 0x18);
  add("P", 0x19);

  add("A", 0x1e);
  add("S", 0x1f);
  add("D", 0x20);
  add("F", 0x21);
  add("G", 0x22);
  add("H", 0x23);
  add("J", 0x24);
  add("K", 0x25);
  add("L", 0x26);

  add("Z", 0x2c);
  add("X", 0x2d);
  add("C", 0x2e);
  add("V", 0x2f);
  add("B", 0x30);
  add("N", 0x31);
  add("M", 0x32);

  // Function keys
  add("F1", 0x3b);
  add("F2", 0x3c);
  add("F3", 0x3d);
  add("F4", 0x3e);
  add("F5", 0x3f);
  add("F6", 0x40);
  add("F7", 0x41);
  add("F8", 0x42);
  add("F9", 0x43);
  add("F10", 0x44);
  add("F11", 0x57);
  add("F12", 0x58);

  // Common named keys (canonical UI names)
  add("ESC", 0x01);
  add("ESCAPE", 0x01);
  add("TAB", 0x0f);
  add("ENTER", 0x1c);
  add("RETURN", 0x1c);
  add("SPACE", 0x39);
  add("BACKSPACE", 0x0e);
  add("CAPSLOCK", 0x3a);

  add("LSHIFT", 0x2a);
  add("RSHIFT", 0x36);
  add("LCTRL", 0x1d);
  add("RCTRL", 0x9d);
  add("LALT", 0x38);
  add("RALT", 0xb8);

  add("UP", 0xc8);
  add("DOWN", 0xd0);
  add("LEFT", 0xcb);
  add("RIGHT", 0xcd);

  add("HOME", 0xc7);
  add("END", 0xcf);
  add("PGUP", 0xc9);
  add("PAGEUP", 0xc9);
  add("PGDN", 0xd1);
  add("PAGEDOWN", 0xd1);
  add("INS", 0xd2);
  add("INSERT", 0xd2);
  add("DEL", 0xd3);
  add("DELETE", 0xd3);

  // Punctuation (main keyboard)
  add("-", 0x0c);
  add("MINUS", 0x0c);
  add("=", 0x0d);
  add("EQUALS", 0x0d);
  add("[", 0x1a);
  add("LBRACKET", 0x1a);
  add("]", 0x1b);
  add("RBRACKET", 0x1b);
  add("\\", 0x2b);
  add("BACKSLASH", 0x2b);
  add(";", 0x27);
  add("SEMICOLON", 0x27);
  add("'", 0x28);
  add("APOSTROPHE", 0x28);
  add("`", 0x29);
  add("GRAVE", 0x29);
  add(",", 0x33);
  add("COMMA", 0x33);
  add(".", 0x34);
  add("PERIOD", 0x34);
  add("/", 0x35);
  add("SLASH", 0x35);

  // Numpad
  add("NUMPAD0", 0x52);
  add("NUMPAD1", 0x4f);
  add("NUMPAD2", 0x50);
  add("NUMPAD3", 0x51);
  add("NUMPAD4", 0x4b);
  add("NUMPAD5", 0x4c);
  add("NUMPAD6", 0x4d);
  add("NUMPAD7", 0x47);
  add("NUMPAD8", 0x48);
  add("NUMPAD9", 0x49);
  add("NUMPADENTER", 0x9c);
  add("NUMPAD+", 0x4e);
  add("NUMPADADD", 0x4e);
  add("NUMPAD-", 0x4a);
  add("NUMPADSUBTRACT", 0x4a);
  add("NUMPAD*", 0x37);
  add("NUMPADMULTIPLY", 0x37);
  add("NUMPAD/", 0xb5);
  add("NUMPADDIVIDE", 0xb5);
  add("NUMPAD.", 0x53);
  add("NUMPADDECIMAL", 0x53);

  function normalizeName(raw) {
    return String(raw || "")
      .trim()
      .replace(/\s+/g, "")
      .toUpperCase();
  }

  function clampDik(n) {
    const v = Number(n);
    if (!isFinite(v)) return null;
    const code = (v >>> 0) & 0xffffffff;
    // DIK is 0..255-ish but Skyrim uses a 32-bit field; keep within 0..0xFF for safety.
    const masked = code & 0xff;
    return masked;
  }

  function dikToKeyName(dik) {
    const code = clampDik(dik);
    if (code == null) return null;
    return DIK_TO_NAME[code] || null;
  }

  function dikToHex(dik) {
    const code = clampDik(dik);
    if (code == null) return null;
    let hex = code.toString(16).toUpperCase();
    if (hex.length < 2) hex = "0" + hex;
    return "0x" + hex;
  }

  function formatDikDisplay(dik) {
    const code = clampDik(dik);
    if (code == null) return null;
    const name = dikToKeyName(code);
    const hex = dikToHex(code);
    return name ? `${name} (${hex})` : hex;
  }

  /**
   * Parses a user-entered keybind string into a DIK code.
   *
   * Supported:
   * - "F4", "E", "1"
   * - "KeyE", "Digit1" (KeyboardEvent.code style)
   * - "0x3E" (hex) or "62" (decimal DIK)
   *
   * Note:
   * - Single-character digits/letters are treated as key names, not DIK numeric codes.
   *   To enter DIK code 1 (Escape), use "0x01" or "ESC".
   */
  function parseKeybindInput(raw) {
    const s0 = String(raw || "").trim();
    if (!s0) return null;

    // Hex DIK.
    if (s0.startsWith("0x") || s0.startsWith("0X")) {
      const n = parseInt(s0, 16);
      return clampDik(n);
    }

    const s = normalizeName(s0);
    if (!s) return null;

    // Single-char A-Z / 0-9 -> key name.
    if (s.length === 1 && /[A-Z0-9]/.test(s)) {
      return NAME_TO_DIK[s] != null ? NAME_TO_DIK[s] : null;
    }

    // Allow "DIK_F4" etc.
    const withoutPrefix = s.startsWith("DIK_") ? s.slice(4) : s;

    // KeyboardEvent.code forms.
    if (withoutPrefix.startsWith("KEY") && withoutPrefix.length === 4) {
      const ch = withoutPrefix.slice(3);
      return NAME_TO_DIK[ch] != null ? NAME_TO_DIK[ch] : null;
    }
    if (withoutPrefix.startsWith("DIGIT") && withoutPrefix.length === 6) {
      const d = withoutPrefix.slice(5);
      return NAME_TO_DIK[d] != null ? NAME_TO_DIK[d] : null;
    }

    // Key names like F4, ESC, SPACE, etc.
    if (NAME_TO_DIK[withoutPrefix] != null) return NAME_TO_DIK[withoutPrefix];

    // Decimal DIK (multi-digit only).
    if (/^\d+$/.test(withoutPrefix)) {
      const n = parseInt(withoutPrefix, 10);
      return clampDik(n);
    }

    return null;
  }

  function jsCodeToDik(codeRaw) {
    const code = String(codeRaw || "").trim();
    if (!code) return null;

    // Most stable: use KeyboardEvent.code ("KeyE", "Digit1", "F4", ...).
    if (code.startsWith("Key") && code.length === 4) {
      return parseKeybindInput(code);
    }
    if (code.startsWith("Digit") && code.length === 6) {
      return parseKeybindInput(code);
    }
    if (/^F\d{1,2}$/.test(code)) {
      return parseKeybindInput(code);
    }

    // Map common non-alphanumeric codes.
    const map = {
      Escape: "ESC",
      Tab: "TAB",
      Enter: "ENTER",
      Space: "SPACE",
      Backspace: "BACKSPACE",
      ArrowUp: "UP",
      ArrowDown: "DOWN",
      ArrowLeft: "LEFT",
      ArrowRight: "RIGHT",
      PageUp: "PGUP",
      PageDown: "PGDN",
      Home: "HOME",
      End: "END",
      Insert: "INS",
      Delete: "DEL",
      Minus: "MINUS",
      Equal: "EQUALS",
      BracketLeft: "LBRACKET",
      BracketRight: "RBRACKET",
      Backslash: "BACKSLASH",
      Semicolon: "SEMICOLON",
      Quote: "APOSTROPHE",
      Backquote: "GRAVE",
      Comma: "COMMA",
      Period: "PERIOD",
      Slash: "SLASH",
      ShiftLeft: "LSHIFT",
      ShiftRight: "RSHIFT",
      ControlLeft: "LCTRL",
      ControlRight: "RCTRL",
      AltLeft: "LALT",
      AltRight: "RALT",
      NumpadEnter: "NUMPADENTER",
      NumpadAdd: "NUMPADADD",
      NumpadSubtract: "NUMPADSUBTRACT",
      NumpadMultiply: "NUMPADMULTIPLY",
      NumpadDivide: "NUMPADDIVIDE",
      NumpadDecimal: "NUMPADDECIMAL",
    };

    if (code.startsWith("Numpad") && code.length === 7) {
      // Numpad0..Numpad9
      const digit = code.slice(6);
      if (/^[0-9]$/.test(digit)) {
        return parseKeybindInput("NUMPAD" + digit);
      }
    }

    const mapped = map[code];
    if (mapped) return parseKeybindInput(mapped);

    return null;
  }

  const api = Object.freeze({
    parseKeybindInput,
    dikToKeyName,
    dikToHex,
    formatDikDisplay,
    jsCodeToDik,
  });

  // Node/CommonJS
  if (typeof module !== "undefined" && module && module.exports) {
    module.exports = api;
  }

  // Browser/Ultralight
  global.COPNG_KEYCODES = api;
})(typeof window !== "undefined" ? window : globalThis);
