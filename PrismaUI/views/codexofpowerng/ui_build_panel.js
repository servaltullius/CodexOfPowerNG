(function (root, factory) {
  const api = factory();
  if (typeof module === "object" && module.exports) {
    module.exports = api;
  }
  if (root) {
    root.COPNGBuildPanel = api;
  }
})(typeof globalThis !== "undefined" ? globalThis : this, function () {
  "use strict";

  function asFn(maybeFn, fallback) {
    return typeof maybeFn === "function" ? maybeFn : fallback;
  }

  function defaultT(_key, fallback) {
    return fallback;
  }

  function defaultTFmt(_key, fallback, vars) {
    return String(fallback).replace(/\{([a-zA-Z0-9_]+)\}/g, (match, name) =>
      vars && Object.prototype.hasOwnProperty.call(vars, name) ? String(vars[name]) : match,
    );
  }

  function defaultEscapeHtml(value) {
    return String(value == null ? "" : value)
      .replace(/&/g, "&amp;")
      .replace(/</g, "&lt;")
      .replace(/>/g, "&gt;")
      .replace(/"/g, "&quot;")
      .replace(/'/g, "&#39;");
  }

  function createEmptyBuild() {
    return {
      disciplines: {
        attack: { score: 0, recordCount: 0, buildPoints: 0, currentTier: 0, nextTierPoints: 8, pointsToNextTier: 8, nextTierScore: 8, scoreToNextTier: 8 },
        defense: { score: 0, recordCount: 0, buildPoints: 0, currentTier: 0, nextTierPoints: 8, pointsToNextTier: 8, nextTierScore: 8, scoreToNextTier: 8 },
        utility: { score: 0, recordCount: 0, buildPoints: 0, currentTier: 0, nextTierPoints: 8, pointsToNextTier: 8, nextTierScore: 8, scoreToNextTier: 8 },
      },
      selectedDiscipline: "attack",
      selectedTheme: "",
      selectedOptionId: "",
      themeMap: {
        attack: [],
        defense: [],
        utility: [],
      },
      groupedCatalog: {
        attack: { discipline: "attack", themes: [] },
        defense: { discipline: "defense", themes: [] },
        utility: { discipline: "utility", themes: [] },
      },
      options: [],
      selectedThemeRows: [],
      selectedOptionDetail: null,
      activeSlots: [],
      migrationNotice: {
        needsNotice: false,
        legacyRewardsMigrated: false,
        unresolvedHistoricalRegistrations: 0,
      },
    };
  }

  function humanizeDiscipline(id) {
    const value = String(id || "").toLowerCase();
    if (value === "attack") return "Attack";
    if (value === "defense") return "Defense";
    if (value === "utility") return "Utility";
    if (value === "wildcard") return "Wildcard";
    return "Unknown";
  }

  function humanizeOptionId(id) {
    return String(id || "")
      .split(".")
      .filter(Boolean)
      .slice(-1)[0]
      .replace(/[_-]+/g, " ")
      .replace(/\b\w/g, (ch) => ch.toUpperCase());
  }

  function humanizeThemeId(id) {
    return String(id || "")
      .replace(/[_-]+/g, " ")
      .replace(/\b\w/g, (ch) => ch.toUpperCase());
  }

  function slotSupportsOption(slot, option) {
    const slotKind = String((slot && slot.slotKind) || "").toLowerCase();
    const discipline = String((option && option.discipline) || "").toLowerCase();
    const rule = String((option && option.slotCompatibility) || "same_or_wildcard").toLowerCase();
    if (!slotKind || !discipline) return false;
    if (rule === "wildcard_only") return slotKind === "wildcard";
    if (rule === "same_discipline_only") return slotKind === discipline;
    return slotKind === discipline || slotKind === "wildcard";
  }

  function disciplineClassName(id) {
    const value = String(id || "").toLowerCase();
    if (value === "attack") return "disc-attack";
    if (value === "defense") return "disc-defense";
    if (value === "utility") return "disc-utility";
    if (value === "wildcard") return "disc-wildcard";
    return "disc-unknown";
  }

  function hierarchyClassName(id) {
    const value = String(id || "").toLowerCase();
    if (value === "signpost") return "isSignpost";
    if (value === "special") return "isSpecial";
    return "isStandard";
  }

  function slotTitle(slot, t) {
    const slotId = String((slot && slot.slotId) || "");
    const slotKind = String((slot && slot.slotKind) || "");
    return t("build.slot." + slotId, humanizeDiscipline(slotKind));
  }

  function disciplineLabel(id, t) {
    return t("build." + String(id || "").toLowerCase(), humanizeDiscipline(id));
  }

  function themeTitleKey(discipline, theme) {
    return "build.theme." + String(discipline || "").toLowerCase() + "." + String(theme || "").toLowerCase();
  }

  function themeLabel(discipline, theme, providedTitleKey, t) {
    const titleKey = providedTitleKey || themeTitleKey(discipline, theme);
    return t(titleKey, humanizeThemeId(theme));
  }

  function formatMagnitude(value) {
    const numeric = Number(value || 0);
    if (!Number.isFinite(numeric)) return "0";
    const rounded = Math.round(numeric);
    if (Math.abs(numeric - rounded) < 0.001) return String(rounded);
    if (Math.abs(numeric) < 1) return String(Number(numeric.toFixed(2)));
    return String(Number(numeric.toFixed(1)));
  }

  function normalizeNumber(value, fallback) {
    const numeric = Number(value);
    return Number.isFinite(numeric) ? numeric : fallback;
  }

  function formatBuildPoints(value) {
    return formatMagnitude(normalizeNumber(value, 0));
  }

  const FRACTIONAL_PERCENT_EFFECT_KEYS = new Set([
    "stamina_rate",
    "heal_rate",
    "magicka_rate",
    "destruction_modifier",
    "restoration_modifier",
    "alteration_modifier",
    "conjuration_modifier",
    "illusion_modifier",
    "smithing_modifier",
    "alchemy_modifier",
    "enchanting_modifier",
    "sneaking_modifier",
    "lockpicking_modifier",
    "pickpocket_modifier",
  ]);
  const ABSOLUTE_PERCENT_EFFECT_KEYS = new Set([
    "shout_recovery_mult",
  ]);

  function formatEffectDisplayMagnitude(effectKey, magnitude) {
    const numeric = Number(magnitude || 0);
    if (!Number.isFinite(numeric)) return "0";
    const normalizedKey = String(effectKey || "").toLowerCase();
    if (FRACTIONAL_PERCENT_EFFECT_KEYS.has(normalizedKey)) {
      return formatMagnitude(numeric * 100);
    }
    if (ABSOLUTE_PERCENT_EFFECT_KEYS.has(normalizedKey)) {
      return formatMagnitude(Math.abs(numeric) * 100);
    }
    return formatMagnitude(numeric);
  }

  function magnitudeValue(option, key) {
    if (!option || typeof option !== "object") return 0;
    const raw = option[key];
    if (typeof raw === "number") return raw;
    return Number(raw || 0);
  }

  function summarizeScaledEffect(effectKey, magnitude, tFmt, t) {
    const numericValue = formatEffectDisplayMagnitude(effectKey, magnitude);
    if (!effectKey) return t("build.none", "No options");
    return tFmt("build.effectMagnitude." + effectKey, effectKey + " {value}", { value: numericValue });
  }

  function summarizeDiscipline(discipline, disciplines, options, activeSlots) {
    const info = disciplines[discipline] || { score: 0, recordCount: 0, buildPoints: 0, currentTier: 0, nextTierPoints: 8, pointsToNextTier: 8, nextTierScore: 8, scoreToNextTier: 8 };
    const optionRows = options.filter((option) => String((option && option.discipline) || "").toLowerCase() === discipline);
    const unlockedCount = optionRows.filter((option) => !!option.unlocked).length;
    const activeCount = optionRows.filter((option) =>
      activeSlots.some((slot) => slot && slot.optionId === option.id),
    ).length;
    return {
      score: Number(info.score || 0) >>> 0,
      recordCount: Number((info.recordCount != null ? info.recordCount : info.score) || 0) >>> 0,
      buildPoints: normalizeNumber(info.buildPoints != null ? info.buildPoints : info.score, 0),
      currentTier: Number(info.currentTier || 0) >>> 0,
      nextTierPoints: normalizeNumber(info.nextTierPoints != null ? info.nextTierPoints : info.nextTierScore, 0),
      pointsToNextTier: normalizeNumber(info.pointsToNextTier != null ? info.pointsToNextTier : info.scoreToNextTier, 0),
      nextTierScore: normalizeNumber(info.nextTierScore != null ? info.nextTierScore : info.nextTierPoints, 0),
      scoreToNextTier: normalizeNumber(info.scoreToNextTier != null ? info.scoreToNextTier : info.pointsToNextTier, 0),
      unlockedCount,
      activeCount,
    };
  }

  function normalizeThemeMap(sourceBuild, options, t) {
    const result = { attack: [], defense: [], utility: [] };
    const sourceThemeMap = sourceBuild && sourceBuild.themeMap && typeof sourceBuild.themeMap === "object" ? sourceBuild.themeMap : {};
    const disciplines = ["attack", "defense", "utility"];

    for (const discipline of disciplines) {
      const sourceList = Array.isArray(sourceThemeMap[discipline]) ? sourceThemeMap[discipline] : [];
      if (sourceList.length > 0) {
        result[discipline] = sourceList.map((theme) => ({
          id: String((theme && theme.id) || ""),
          titleKey: String((theme && theme.titleKey) || themeTitleKey(discipline, theme && theme.id)),
          label: themeLabel(discipline, theme && theme.id, theme && theme.titleKey, t),
          optionCount: Number((theme && theme.optionCount) || 0) >>> 0,
        }));
        continue;
      }

      const seen = new Set();
      const derived = [];
      for (const option of options) {
        const optionDiscipline = String((option && option.discipline) || "").toLowerCase();
        if (optionDiscipline !== discipline) continue;
        const themeId = String((option && option.themeId) || "").toLowerCase();
        if (!themeId || seen.has(themeId)) continue;
        seen.add(themeId);
        derived.push({
          id: themeId,
          titleKey: String((option && option.themeTitleKey) || themeTitleKey(discipline, themeId)),
          label: themeLabel(discipline, themeId, option && option.themeTitleKey, t),
          optionCount: options.filter((row) => String((row && row.discipline) || "").toLowerCase() === discipline && String((row && row.themeId) || "").toLowerCase() === themeId).length,
        });
      }
      result[discipline] = derived;
    }

    return result;
  }

  function normalizeGroupedCatalog(sourceBuild, themeMap, options) {
    const sourceCatalog =
      sourceBuild && sourceBuild.groupedCatalog && typeof sourceBuild.groupedCatalog === "object"
        ? sourceBuild.groupedCatalog
        : null;
    const result = {
      attack: { discipline: "attack", themes: [] },
      defense: { discipline: "defense", themes: [] },
      utility: { discipline: "utility", themes: [] },
    };

    for (const discipline of ["attack", "defense", "utility"]) {
      const explicitThemes =
        sourceCatalog &&
        sourceCatalog[discipline] &&
        Array.isArray(sourceCatalog[discipline].themes)
          ? sourceCatalog[discipline].themes
          : null;
      if (explicitThemes) {
        result[discipline].themes = explicitThemes.map((theme) => ({
          id: String((theme && theme.id) || "").toLowerCase(),
          titleKey: String((theme && theme.titleKey) || ""),
          optionCount: Number((theme && theme.optionCount) || 0) >>> 0,
          rows: Array.isArray(theme && theme.rows) ? theme.rows.slice() : [],
        }));
        continue;
      }

      const fallbackThemes = Array.isArray(themeMap[discipline]) ? themeMap[discipline] : [];
      result[discipline].themes = fallbackThemes.map((theme) => {
        const themeId = String((theme && theme.id) || "").toLowerCase();
        const rows = options.filter((option) => {
          if (String((option && option.discipline) || "").toLowerCase() !== discipline) return false;
          return !themeId || String((option && option.themeId) || "").toLowerCase() === themeId;
        });
        return {
          id: themeId,
          titleKey: String((theme && theme.titleKey) || ""),
          optionCount: Number((theme && theme.optionCount) || rows.length || 0) >>> 0,
          rows,
        };
      });
    }

    return result;
  }

  function resolveSelectedDiscipline(sourceBuild, themeMap) {
    const requested = String((sourceBuild && sourceBuild.selectedDiscipline) || "").toLowerCase();
    if (requested && Object.prototype.hasOwnProperty.call(themeMap, requested)) return requested;
    if (themeMap.attack.length) return "attack";
    if (themeMap.defense.length) return "defense";
    if (themeMap.utility.length) return "utility";
    return "attack";
  }

  function resolveSelectedTheme(sourceBuild, selectedDiscipline, themeMap) {
    const requested = String((sourceBuild && sourceBuild.selectedTheme) || "").toLowerCase();
    const themes = Array.isArray(themeMap[selectedDiscipline]) ? themeMap[selectedDiscipline] : [];
    if (requested && themes.some((theme) => theme.id === requested)) return requested;
    return themes.length ? String(themes[0].id || "") : "";
  }

  function resolveRequestedSelection(sourceBuild, themeMap) {
    const selection =
      sourceBuild && sourceBuild.buildSelection && typeof sourceBuild.buildSelection === "object"
        ? sourceBuild.buildSelection
        : sourceBuild || {};
    const selectedDiscipline = resolveSelectedDiscipline({ selectedDiscipline: selection.discipline || selection.selectedDiscipline }, themeMap);
    const selectedTheme = resolveSelectedTheme(
      { selectedTheme: selection.theme || selection.selectedTheme },
      selectedDiscipline,
      themeMap,
    );
    return {
      discipline: selectedDiscipline,
      theme: selectedTheme,
      optionId: String(selection.optionId || selection.selectedOptionId || ""),
    };
  }

  function buildOptionView(option, activeSlots, t, tFmt) {
    const optionId = String((option && option.id) || "");
    const unlockPoints = normalizeNumber(option && (option.unlockPoints != null ? option.unlockPoints : option.unlockScore), 0);
    const compatibleSlots = activeSlots.filter((slot) => slotSupportsOption(slot, option));
    const activeSlot = compatibleSlots.find((slot) => slot && slot.optionId === optionId) || null;
    const emptySlot = compatibleSlots.find((slot) => slot && !slot.optionId) || null;
    const unlocked = !!(option && option.unlocked);
    const discipline = String((option && option.discipline) || "").toLowerCase();
    const themeId = String((option && option.themeId) || "").toLowerCase();
    const hierarchy = String((option && option.hierarchy) || "standard").toLowerCase();
    const title = t(option.titleKey, humanizeOptionId(optionId));
    const description = t(option.descriptionKey, "");
    const stateKey = activeSlot ? "build.active" : unlocked ? "build.unlocked" : "build.locked";
    const stateText = t(stateKey, activeSlot ? "Active" : unlocked ? "Unlocked" : "Locked");
    const stateClass = activeSlot ? "isActive" : unlocked ? "isUnlocked" : "isLocked";
    const unlockText = tFmt("build.requiresScore", "Need {score} pt", { score: formatBuildPoints(unlockPoints) });
    const compatibleText = compatibleSlots.length
      ? compatibleSlots.map((slot) => slotTitle(slot, t)).join(" / ")
      : t("build.noCompatibleSlots", "No compatible slots");
    const currentTier = Number((option && option.currentTier) || 0) >>> 0;
    const nextTierPoints = normalizeNumber(option && (option.nextTierPoints != null ? option.nextTierPoints : option.nextTierScore), 0);
    const pointsToNextTier = normalizeNumber(option && (option.pointsToNextTier != null ? option.pointsToNextTier : option.scoreToNextTier), 0);
    const hasCurrentMagnitude =
      option && Object.prototype.hasOwnProperty.call(option, "currentMagnitude")
        ? option.currentMagnitude != null
        : option && Object.prototype.hasOwnProperty.call(option, "magnitude");
    const hasNextMagnitude = option && Object.prototype.hasOwnProperty.call(option, "nextMagnitude");
    const currentEffectText = hasCurrentMagnitude
      ? summarizeScaledEffect(
          String((option && option.effectKey) || ""),
          magnitudeValue(option, "currentMagnitude") || magnitudeValue(option, "magnitude"),
          tFmt,
          t,
        )
      : description;
    const nextEffectText = hasNextMagnitude
      ? summarizeScaledEffect(String((option && option.effectKey) || ""), magnitudeValue(option, "nextMagnitude"), tFmt, t)
      : t("build.nextTierFallback", "Effect improves at the next tier.");
    let actionHtml = `<span class="small buildHint">${String(unlockText)}</span>`;

    if (unlocked) {
      if (activeSlot) {
        actionHtml = `<button type="button" class="buildActionButton isPrimary" data-action="build-deactivate" data-slot-id="${defaultEscapeHtml(
          String(activeSlot.slotId || ""),
        )}">${defaultEscapeHtml(t("build.deactivate", "Deactivate"))}</button>`;
      } else if (emptySlot) {
        actionHtml = `<button type="button" class="buildActionButton isPrimary" data-action="build-activate" data-option-id="${defaultEscapeHtml(
          optionId,
        )}" data-slot-id="${defaultEscapeHtml(String(emptySlot.slotId || ""))}">${defaultEscapeHtml(
          t("build.activate", "Activate"),
        )}</button>`;
      } else {
        actionHtml = `<span class="small buildHint">${defaultEscapeHtml(
          t("build.slotOccupiedHint", "Deactivate a compatible slot first."),
        )}</span>`;
      }
    }

    return {
      optionId,
      discipline,
      disciplineClass: disciplineClassName(discipline),
      themeId,
      hierarchy,
      hierarchyClass: hierarchyClassName(hierarchy),
      title,
      description,
      unlockText,
      unlockPoints,
      stateText,
      stateClass,
      unlocked,
      activeSlot,
      emptySlot,
      compatibleSlots,
      compatibleText,
      currentTier,
      nextTierPoints,
      pointsToNextTier,
      currentEffectText,
      nextEffectText,
      actionHtml,
    };
  }

  function renderBuildPanelHtml(build, helpers) {
    const source = build && typeof build === "object" ? build : createEmptyBuild();
    const t = asFn(helpers && helpers.t, defaultT);
    const tFmt = asFn(helpers && helpers.tFmt, defaultTFmt);
    const escapeHtml = asFn(helpers && helpers.escapeHtml, defaultEscapeHtml);
    const disciplines = source.disciplines && typeof source.disciplines === "object" ? source.disciplines : {};
    const activeSlots = Array.isArray(source.activeSlots) ? source.activeSlots : [];
    const options = Array.isArray(source.options) ? source.options : [];
    const themeMap = normalizeThemeMap(source, options, t);
    const groupedCatalog = normalizeGroupedCatalog(source, themeMap, options);
    const currentSelection = resolveRequestedSelection(source, themeMap);
    const selectedDiscipline = currentSelection.discipline;
    const selectedTheme = currentSelection.theme;
    const activeOptionId = currentSelection.optionId;
    const payloadSelection = {
      discipline: resolveSelectedDiscipline(source, themeMap),
      theme: resolveSelectedTheme(source, resolveSelectedDiscipline(source, themeMap), themeMap),
      optionId: String((source && source.selectedOptionId) || ""),
    };
    const optionViews = options.map((option) => buildOptionView(option, activeSlots, t, tFmt));
    const groupedThemes =
      groupedCatalog &&
      groupedCatalog[selectedDiscipline] &&
      Array.isArray(groupedCatalog[selectedDiscipline].themes)
        ? groupedCatalog[selectedDiscipline].themes
        : [];
    const groupedTheme = groupedThemes.find((theme) => String((theme && theme.id) || "").toLowerCase() === selectedTheme) || null;
    const selectedThemeRows =
      selectedDiscipline === payloadSelection.discipline &&
      selectedTheme === payloadSelection.theme &&
      Array.isArray(source.selectedThemeRows) &&
      source.selectedThemeRows.length
        ? source.selectedThemeRows.slice()
        : groupedTheme && Array.isArray(groupedTheme.rows) && groupedTheme.rows.length
          ? groupedTheme.rows.slice()
          : options.filter(
              (option) =>
                String((option && option.discipline) || "").toLowerCase() === selectedDiscipline &&
                (!selectedTheme || String((option && option.themeId) || "").toLowerCase() === selectedTheme),
            );
    const themedViews = selectedThemeRows.map((option) => buildOptionView(option, activeSlots, t, tFmt));
    const selectedOptionDetail =
      source.selectedOptionDetail &&
      typeof source.selectedOptionDetail === "object" &&
      selectedDiscipline === payloadSelection.discipline &&
      selectedTheme === payloadSelection.theme
        ? source.selectedOptionDetail
        : null;
    const focusedSourceOption =
      (selectedOptionDetail &&
        String((selectedOptionDetail && selectedOptionDetail.id) || "") === activeOptionId &&
        selectedOptionDetail) ||
      selectedThemeRows.find((row) => String((row && row.id) || "") === activeOptionId) ||
      (selectedOptionDetail && !activeOptionId ? selectedOptionDetail : null) ||
      selectedThemeRows.find((row) =>
        activeSlots.some((slot) => String((slot && slot.optionId) || "") === String((row && row.id) || "")),
      ) ||
      selectedThemeRows.find((row) => !!(row && row.unlocked)) ||
      selectedThemeRows[0] ||
      options.find((option) => activeSlots.some((slot) => String((slot && slot.optionId) || "") === String((option && option.id) || ""))) ||
      options.find((option) => !!(option && option.unlocked)) ||
      options[0] ||
      null;
    const focusedView = focusedSourceOption ? buildOptionView(focusedSourceOption, activeSlots, t, tFmt) : null;

    const disciplineOrder = ["attack", "defense", "utility"];
    const summaryHtml = disciplineOrder
      .map((discipline) => {
        const info = summarizeDiscipline(discipline, disciplines, options, activeSlots);
        const label = disciplineLabel(discipline, t);
        const sectionClass = disciplineClassName(discipline);
        return `
          <article class="buildSummaryCard ${sectionClass}">
            <div class="small buildPanelEyebrow">${escapeHtml(t("build.scoreSummaryLabel", "Build Score"))}</div>
            <strong>${escapeHtml(label)}</strong>
            <div class="buildSummaryValue">${escapeHtml(String(info.recordCount))}</div>
            <div class="small">${escapeHtml(
              tFmt("build.pointsSummaryLabel", "{points} pt", { points: formatBuildPoints(info.buildPoints) }),
            )}</div>
            <div class="small">${escapeHtml(
              tFmt("build.summaryTier", "Tier {tier}", { tier: info.currentTier }),
            )}</div>
            <div class="small">${escapeHtml(
              tFmt("build.summaryNextTier", "{score} pt to next tier", { score: formatBuildPoints(info.pointsToNextTier) }),
            )}</div>
          </article>`;
      })
      .join("");

    const migration = source.migrationNotice && typeof source.migrationNotice === "object" ? source.migrationNotice : {};
    const migrationParts = [];
    if (migration.needsNotice && migration.legacyRewardsMigrated) {
      migrationParts.push(
        `<div class="buildMigrationLine">${escapeHtml(
          t("build.migrationLegacy", "Legacy rewards migrated into the new build progression."),
        )}</div>`,
      );
    }
    if (migration.needsNotice && Number(migration.unresolvedHistoricalRegistrations || 0) > 0) {
      migrationParts.push(
        `<div class="buildMigrationLine">${escapeHtml(
          tFmt("build.migrationSkipped", "{count} historical registrations could not be converted.", {
            count: Number(migration.unresolvedHistoricalRegistrations || 0) >>> 0,
          }),
        )}</div>`,
      );
    }

    const disciplineRailHtml = disciplineOrder
      .map((discipline) => {
        const info = summarizeDiscipline(discipline, disciplines, options, activeSlots);
        const sectionClass = disciplineClassName(discipline);
        const activeClass = discipline === selectedDiscipline ? " isActive" : "";
        return `
          <button
            type="button"
            class="buildDisciplineButton ${sectionClass}${activeClass}"
            data-action="build-select-discipline"
            data-discipline="${escapeHtml(discipline)}"
          >
            <span class="buildDisciplineButtonLabel">${escapeHtml(disciplineLabel(discipline, t))}</span>
            <span class="small">${escapeHtml(tFmt("build.scorePill", "Record {score}", { score: info.recordCount }))}</span>
          </button>`;
      })
      .join("");

    const activeThemes = groupedThemes.length
      ? groupedThemes.map((theme) => ({
          id: String((theme && theme.id) || "").toLowerCase(),
          titleKey: String((theme && theme.titleKey) || ""),
          label: themeLabel(selectedDiscipline, theme && theme.id, theme && theme.titleKey, t),
          optionCount: Number((theme && theme.optionCount) || (Array.isArray(theme && theme.rows) ? theme.rows.length : 0) || 0) >>> 0,
        }))
      : Array.isArray(themeMap[selectedDiscipline])
        ? themeMap[selectedDiscipline]
        : [];
    const themeTabsHtml = activeThemes
      .map((theme) => {
        const activeClass = theme.id === selectedTheme ? " isActive" : "";
        return `
          <button
            type="button"
            class="buildThemeTab${activeClass}"
            data-action="build-select-theme"
            data-discipline="${escapeHtml(selectedDiscipline)}"
            data-theme-id="${escapeHtml(theme.id)}"
          >
            <span>${escapeHtml(theme.label)}</span>
            <span class="small">${escapeHtml(String(Number(theme.optionCount || 0) >>> 0))}</span>
          </button>`;
      })
      .join("");

    const catalogRowsHtml = themedViews.length
      ? themedViews
          .map((view) => {
            const focusClass = focusedView && focusedView.optionId === view.optionId ? " isFocused" : "";
            return `
              <article
                class="buildCatalogRow ${view.disciplineClass} ${view.stateClass}${focusClass}"
                data-option-id="${escapeHtml(view.optionId)}"
              >
                <button
                  type="button"
                  class="buildCatalogRowMain"
                  data-action="build-select-option"
                  data-option-id="${escapeHtml(view.optionId)}"
                >
                  <div class="buildCatalogLead">
                        <div class="buildCatalogTitleRow">
                          <strong>${escapeHtml(view.title)}</strong>
                          <span class="buildCatalogState ${view.stateClass}">${escapeHtml(view.stateText)}</span>
                        </div>
                        <div class="small buildCatalogEffect">${escapeHtml(view.currentEffectText)}</div>
                      </div>
                      <div class="buildCatalogMeta">
                        <span class="small mono">${escapeHtml(view.unlockText)}</span>
                        <span class="small">${escapeHtml(
                          tFmt("build.nextTierInline", "Next tier in {score} pt", { score: formatBuildPoints(view.pointsToNextTier) }),
                        )}</span>
                      </div>
                    </button>
                  </article>`;
          })
          .join("")
      : `<div class="small">${escapeHtml(t("build.none", "No options"))}</div>`;

    const slotMatrixHtml = activeSlots
      .map((slot) => {
        const slotId = String((slot && slot.slotId) || "");
        const slotKind = String((slot && slot.slotKind) || "");
        const slotClass = disciplineClassName(slotKind);
        const option = optionViews.find((view) => view.optionId === String(slot && slot.optionId)) || null;
        const optionTitle = option ? option.title : t("build.emptySlot", "Empty slot");
        const actionHtml = option
          ? `<button type="button" class="buildActionButton" data-action="build-deactivate" data-slot-id="${escapeHtml(
              slotId,
            )}">${escapeHtml(t("build.deactivate", "Deactivate"))}</button>`
          : `<span class="small buildHint">${escapeHtml(t("build.slotWaiting", "Awaiting option"))}</span>`;
        return `
          <article class="buildSlotMatrixCard ${slotClass} ${option ? "isOccupied" : "isEmpty"}" data-slot-id="${escapeHtml(
            slotId,
          )}">
            <div class="buildSlotMatrixHeader">
              <span class="disciplineMark ${slotClass}">${escapeHtml(slotTitle(slot, t))}</span>
            </div>
            <strong class="buildSlotMatrixName">${escapeHtml(optionTitle)}</strong>
            <div class="buildSlotMatrixActions">${actionHtml}</div>
          </article>`;
      })
      .join("");

    let detailHtml = `
      <section class="card buildSelectedOptionPanel buildStaticPanel">
        <div class="small buildPanelEyebrow">${escapeHtml(t("build.focusTitle", "Focused Option"))}</div>
        <h2>${escapeHtml(t("build.availableOptions", "Available Options"))}</h2>
        <div class="small">${escapeHtml(t("build.focusEmpty", "Unlock or activate an option to inspect it here."))}</div>
      </section>`;

    if (focusedView) {
      const slotLabel = focusedView.activeSlot ? slotTitle(focusedView.activeSlot, t) : t("build.none", "No options");
      const themeName = themeLabel(selectedDiscipline, focusedView.themeId, null, t);
      detailHtml = `
        <section class="card buildSelectedOptionPanel buildStaticPanel ${focusedView.disciplineClass}" data-wheel-surface="build-detail">
          <div class="small buildPanelEyebrow">${escapeHtml(t("build.focusTitle", "Focused Option"))}</div>
          <h2>${escapeHtml(focusedView.title)}</h2>
          <div class="buildFocusStateRow">
            <span class="disciplineMark ${focusedView.disciplineClass}">${escapeHtml(
              disciplineLabel(focusedView.discipline, t),
            )}</span>
            <span class="buildCatalogState ${focusedView.stateClass}">${escapeHtml(focusedView.stateText)}</span>
          </div>
          <div class="small buildFocusDescription">${escapeHtml(focusedView.description)}</div>
          <div class="small buildFocusStat">${escapeHtml(
            tFmt("build.currentEffectLabel", "Current Effect", {}),
          )}: ${escapeHtml(focusedView.currentEffectText)}</div>
          <div class="small buildFocusStat">${escapeHtml(
            tFmt("build.currentTierLabel", "Current Tier", {}),
          )}: ${escapeHtml(String(focusedView.currentTier))}</div>
          <div class="small buildFocusStat">${escapeHtml(
            tFmt("build.nextEffectLabel", "Next Tier Effect", {}),
          )}: ${escapeHtml(focusedView.nextEffectText)}</div>
          <div class="small buildFocusStat">${escapeHtml(
            tFmt("build.nextTierLabel", "Next Tier In", {}),
          )}: ${escapeHtml(
            tFmt("build.nextTierInline", "Next tier in {score} pt", { score: formatBuildPoints(focusedView.pointsToNextTier) }),
          )}</div>
          <div class="buildFocusActions">${focusedView.actionHtml}</div>
          <div class="buildFocusMeta">
            <div class="buildFocusMetaItem">
              <span class="small">${escapeHtml(t("build.themeLabel", "Theme"))}</span>
              <strong>${escapeHtml(themeName)}</strong>
            </div>
            <div class="buildFocusMetaItem">
              <span class="small">${escapeHtml(t("build.requiresScoreLabel", "Unlock"))}</span>
              <strong>${escapeHtml(focusedView.unlockText)}</strong>
            </div>
            <div class="buildFocusMetaItem">
              <span class="small">${escapeHtml(t("build.compatibleSlotsLabel", "Compatible Slots"))}</span>
              <strong>${escapeHtml(focusedView.compatibleText)}</strong>
            </div>
            <div class="buildFocusMetaItem">
              <span class="small">${escapeHtml(t("build.activeSlotLabel", "Current Slot"))}</span>
              <strong>${escapeHtml(slotLabel)}</strong>
            </div>
          </div>
        </section>`;
    }

    return `
      <div class="buildBoardInner buildFixedBoard">
        <div class="buildSummaryBar">
          ${summaryHtml}
        </div>
        <div id="buildMigrationNotice" class="buildMigrationNotice${migrationParts.length ? "" : " isEmpty"}">
          ${migrationParts.join("")}
        </div>
        <div class="buildCatalogShell buildFixedSurface">
          <div class="buildCatalogLayout">
            <aside class="card buildDisciplineRail buildStaticPanel">
              <div class="small buildPanelEyebrow">${escapeHtml(t("build.disciplineLedger", "Discipline"))}</div>
              <div class="buildDisciplineRailBody">
                ${disciplineRailHtml}
              </div>
            </aside>
            <section class="card buildCatalogPanel buildStaticPanel">
              <div class="buildCatalogHeader">
                <div class="small buildPanelEyebrow">${escapeHtml(t("build.availableOptions", "Available Options"))}</div>
                <h2>${escapeHtml(themeLabel(selectedDiscipline, selectedTheme, null, t))}</h2>
                <div class="small buildCatalogLead">${escapeHtml(
                  t(
                    "build.help",
                    "Record count stays visible for collection motivation, while weighted build points unlock options and linearly scale slotted effects.",
                  ),
                )}</div>
              </div>
              <div class="buildThemeTabs">
                ${themeTabsHtml}
              </div>
              <div id="buildCatalogScroller" class="buildCatalogScroller" data-wheel-surface="build-options">
                <div class="buildCatalogRows">${catalogRowsHtml}</div>
              </div>
            </section>
            <aside class="buildDetailRail">
              ${detailHtml}
              <section class="card buildSlotSummaryPanel buildStaticPanel">
                <div class="small buildPanelEyebrow">${escapeHtml(t("build.activeSlots", "Active Slots"))}</div>
                <div class="buildSlotMatrix">
                  ${slotMatrixHtml}
                </div>
              </section>
            </aside>
          </div>
        </div>
      </div>`;
  }

  return Object.freeze({
    renderBuildPanelHtml,
  });
});
