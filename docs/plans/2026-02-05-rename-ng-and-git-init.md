# CodexOfPowerNG Naming + Git Init Implementation Plan

> **For Codex:** REQUIRED SUB-SKILL: Use `superpowers:executing-plans` to implement this plan task-by-task.

**Goal:** Remove legacy `1.1` naming remnants, standardize on “NG” naming, and initialize a local git repository with clean ignore rules.

**Architecture:** Rename legacy “Codex of Power” artifacts/docs to `Codex of Power NG` naming, update all references, and regenerate the MO2-ready archive under the new name. Initialize git with a `.gitignore` that excludes generated outputs (`build/`, `dist/`, `vcpkg_installed/`, release zips) and IDE metadata.

**Tech Stack:** git, bash, text edits.

---

### Task 1: Inventory of legacy naming references

**Files:**
- Modify: `AGENTS.md`
- Modify: `README.md` (if it references old release names)
- Modify: `NEXUS_DESCRIPTION.md` / `CHANGELOG.md` (rename + update as needed)

**Step 1: Search for references**

Run:
- `rg -n \"Codex of Power\\s+\\d+\\.\\d+|\\d+\\.\\d+\\.zip|\\bv\\d+\\.\\d+\\b\" .`

**Expected:** Identify all places that still use `1.1` as a “name” (not as an internal version).

---

### Task 2: Rename release archive name to NG

**Files:**
- Move: `releases/Codex of Power NG.zip` (ensure this is the only release zip name)

**Step 1: Rename**

Run:
- `mv \"releases/Codex of Power NG.zip\" \"releases/Codex of Power NG.zip\"` (no-op if already correct)

**Step 2: Update references**

Edit docs to point to:
- `releases/Codex of Power NG.zip`

**Step 3: Recreate archive using the new name**

Run:
- `cmake --install build/wsl-release`
- `cd dist/CodexOfPowerNG && zip -r -FS \"../../releases/Codex of Power NG.zip\" .`

**Expected:** MO2-ready zip exists at `releases/Codex of Power NG.zip`.

---

### Task 3: Git initialization + ignore hygiene

**Files:**
- Modify: `.gitignore`
- Create: `.git/` (git metadata)

**Step 1: Update ignores**
- Add IDE + release zips:
  - `.idea/`
  - `.idea.backup*/`
  - `releases/*.zip`

**Step 2: Initialize git and create first commit**

Run:
- `git init -b main`
- `git add -A`
- `git commit -m \"chore: initialize CodexOfPowerNG repo\"`

**Expected:** Clean working tree; source+docs tracked; generated outputs ignored.
