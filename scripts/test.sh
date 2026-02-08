#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "[copng] JS tests"
if ! command -v node >/dev/null 2>&1; then
  echo "[copng] node not found (required for JS tests)" >&2
  exit 1
fi

shopt -s nullglob
JS_TESTS=( "$ROOT_DIR/tests/"*.test.cjs )
if (( ${#JS_TESTS[@]} == 0 )); then
  echo "[copng] No JS tests found (tests/*.test.cjs)"
else
  node --test "${JS_TESTS[@]}"
fi

echo "[copng] C++ host tests"
if ! command -v c++ >/dev/null 2>&1; then
  echo "[copng] c++ not found (required for C++ tests)" >&2
  exit 1
fi

CPP_TESTS=( "$ROOT_DIR/tests/"*.test.cpp )
if (( ${#CPP_TESTS[@]} == 0 )); then
  echo "[copng] No C++ tests found (tests/*.test.cpp)"
else
  TMP_DIR="$(mktemp -d)"
  cleanup() { rm -rf "$TMP_DIR"; }
  trap cleanup EXIT

  for test_src in "${CPP_TESTS[@]}"; do
    # Keep this runner host-friendly: skip any test that directly depends on SKSE/CommonLib headers.
    if grep -qE '^[[:space:]]*#include[[:space:]]*<(RE/|SKSE/)' "$test_src"; then
      echo "[copng] SKIP (requires Skyrim headers): $(basename "$test_src")"
      continue
    fi

    exe="$TMP_DIR/$(basename "${test_src%.cpp}")"
    echo "[copng] build: $(basename "$test_src")"
    c++ -std=c++20 -O0 -g -Wall -Wextra -I"$ROOT_DIR/include" "$test_src" -o "$exe"
    echo "[copng] run: $(basename "$test_src")"
    "$exe"
  done
fi

echo "[copng] OK"
