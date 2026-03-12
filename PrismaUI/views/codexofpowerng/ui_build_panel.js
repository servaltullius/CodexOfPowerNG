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
        attack: { score: 0, unlockedBaselineCount: 0 },
        defense: { score: 0, unlockedBaselineCount: 0 },
        utility: { score: 0, unlockedBaselineCount: 0 },
      },
      options: [],
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

  function slotTitle(slot, t) {
    const slotId = String((slot && slot.slotId) || "");
    const slotKind = String((slot && slot.slotKind) || "");
    return t("build.slot." + slotId, humanizeDiscipline(slotKind));
  }

  function summarizeDiscipline(discipline, disciplines, options, activeSlots) {
    const info = disciplines[discipline] || { score: 0, unlockedBaselineCount: 0 };
    const optionRows = options.filter((option) => String((option && option.discipline) || "").toLowerCase() === discipline);
    const unlockedCount = optionRows.filter((option) => !!option.unlocked).length;
    const activeCount = optionRows.filter((option) =>
      activeSlots.some((slot) => slot && slot.optionId === option.id),
    ).length;
    return {
      score: Number(info.score || 0) >>> 0,
      unlockedBaselineCount: Number(info.unlockedBaselineCount || 0) >>> 0,
      unlockedCount,
      activeCount,
    };
  }

  function buildOptionView(option, activeSlots, t, tFmt) {
    const optionId = String((option && option.id) || "");
    const unlockScore = Number((option && option.unlockScore) || 0) >>> 0;
    const compatibleSlots = activeSlots.filter((slot) => slotSupportsOption(slot, option));
    const activeSlot = compatibleSlots.find((slot) => slot && slot.optionId === optionId) || null;
    const emptySlot = compatibleSlots.find((slot) => slot && !slot.optionId) || null;
    const unlocked = !!(option && option.unlocked);
    const discipline = String((option && option.discipline) || "").toLowerCase();
    const title = t(option.titleKey, humanizeOptionId(optionId));
    const description = t(option.descriptionKey, "");
    const stateKey = activeSlot ? "build.active" : unlocked ? "build.unlocked" : "build.locked";
    const stateText = t(stateKey, activeSlot ? "Active" : unlocked ? "Unlocked" : "Locked");
    const stateClass = activeSlot ? "isActive" : unlocked ? "isUnlocked" : "isLocked";
    const unlockText = tFmt("build.requiresScore", "Need {score} Score", { score: unlockScore });
    const compatibleText = compatibleSlots.length
      ? compatibleSlots.map((slot) => slotTitle(slot, t)).join(" / ")
      : t("build.noCompatibleSlots", "No compatible slots");
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
      title,
      description,
      unlockText,
      stateText,
      stateClass,
      unlocked,
      activeSlot,
      emptySlot,
      compatibleSlots,
      compatibleText,
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
    const optionById = new Map();

    for (const option of options) {
      if (option && typeof option.id === "string") optionById.set(option.id, option);
    }

    const disciplineOrder = ["attack", "defense", "utility"];
    const optionViews = options.map((option) => buildOptionView(option, activeSlots, t, tFmt));
    const focusOptionView =
      optionViews.find((view) => !!view.activeSlot) ||
      optionViews.find((view) => view.unlocked) ||
      optionViews[0] ||
      null;
    const slotHtml = activeSlots
      .map((slot) => {
        const slotId = String((slot && slot.slotId) || "");
        const slotKind = String((slot && slot.slotKind) || "");
        const option = optionById.get(slot && slot.optionId) || null;
        const slotLabel = slotTitle(slot, t);
        const stateLabel = option ? t("build.active", "Active") : t("build.emptySlot", "Empty slot");
        const optionTitle = option
          ? t(option.titleKey, humanizeOptionId(option.id))
          : t("build.emptySlot", "Empty slot");
        const actionButton = option
          ? `<button type="button" class="buildActionButton" data-action="build-deactivate" data-slot-id="${escapeHtml(
              slotId,
            )}">${escapeHtml(
              t("build.deactivate", "Deactivate"),
            )}</button>`
          : `<span class="small buildHint">${escapeHtml(t("build.slotWaiting", "Awaiting option"))}</span>`;
        const slotClass = disciplineClassName(slotKind);
        return `
          <article class="buildSlotCard ${slotClass} ${option ? "isOccupied" : "isEmpty"}" data-slot-id="${escapeHtml(
            slotId,
          )}" data-slot-kind="${escapeHtml(slotKind)}">
            <div class="buildSlotCardHeader">
              <span class="disciplineMark ${slotClass}">${escapeHtml(slotLabel)}</span>
              <span class="small">${escapeHtml(stateLabel)}</span>
            </div>
            <strong class="buildSlotName">${escapeHtml(optionTitle)}</strong>
            <div class="small mono">${escapeHtml(slotId)}</div>
            <div class="buildSlotCardActions">${actionButton}</div>
          </article>`;
      })
      .join("");

    const migrationParts = [];
    const migration = source.migrationNotice && typeof source.migrationNotice === "object" ? source.migrationNotice : {};
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
          tFmt(
            "build.migrationSkipped",
            "{count} historical registrations could not be converted.",
            { count: Number(migration.unresolvedHistoricalRegistrations || 0) >>> 0 },
          ),
        )}</div>`,
      );
    }

    const cardsHtml = disciplineOrder
      .map((discipline) => {
        const info = summarizeDiscipline(discipline, disciplines, options, activeSlots);
        const label = t("build." + discipline, humanizeDiscipline(discipline));
        const sectionClass = disciplineClassName(discipline);
        const rows = optionViews
          .filter((view) => view.discipline === discipline)
          .map((view) => {
            const isFocus = focusOptionView && focusOptionView.optionId === view.optionId;
            return `
              <article class="buildOptionCard ${view.stateClass} ${view.disciplineClass} ${isFocus ? "isFocus" : ""}" data-option-id="${escapeHtml(
                view.optionId,
              )}">
                <div class="buildOptionHeader">
                  <strong>${escapeHtml(view.title)}</strong>
                  <span class="pill">${escapeHtml(view.stateText)}</span>
                </div>
                <div class="small">${escapeHtml(view.description)}</div>
                <div class="small mono">${escapeHtml(view.unlockText)}</div>
                <div class="small buildOptionCompat">${escapeHtml(
                  tFmt("build.compatibleSlots", "Compatible Slots: {slots}", { slots: view.compatibleText }),
                )}</div>
                <div class="buildOptionActions">${view.actionHtml}</div>
              </article>`;
          })
          .join("");

        return `
          <section class="buildDisciplineSection ${sectionClass}" data-discipline="${escapeHtml(discipline)}">
            <div class="buildDisciplineHeader">
              <div>
                <div class="small buildPanelEyebrow">${escapeHtml(t("build.disciplineLedger", "Discipline"))}</div>
                <h3>${escapeHtml(label)}</h3>
              </div>
              <div class="buildDisciplineStats">
                <span class="pill">${escapeHtml(tFmt("build.scorePill", "Score {score}", { score: info.score }))}</span>
                <span class="small">${escapeHtml(
                  tFmt("build.baselinePill", "Baseline {count}", {
                    count: info.unlockedBaselineCount,
                  }),
                )}</span>
                <span class="small">${escapeHtml(
                  tFmt("build.unlockedCount", "Unlocked {count}", { count: info.unlockedCount }),
                )}</span>
                <span class="small">${escapeHtml(
                  tFmt("build.activeCount", "Active {count}", { count: info.activeCount }),
                )}</span>
              </div>
            </div>
            <div class="buildOptionList">${rows || `<div class="small">${escapeHtml(t("build.none", "No options"))}</div>`}</div>
          </section>`;
      })
      .join("");

    const summaryHtml = disciplineOrder
      .map((discipline) => {
        const info = summarizeDiscipline(discipline, disciplines, options, activeSlots);
        const label = t("build." + discipline, humanizeDiscipline(discipline));
        const sectionClass = disciplineClassName(discipline);
        return `
          <article class="buildSummaryCard ${sectionClass}">
            <div class="small buildPanelEyebrow">${escapeHtml(t("build.scoreSummaryLabel", "Build Score"))}</div>
            <strong>${escapeHtml(label)}</strong>
            <div class="buildSummaryValue">${escapeHtml(String(info.score))}</div>
            <div class="small">${escapeHtml(
              tFmt("build.summaryCounts", "Baseline {baseline} / Unlocked {unlocked} / Active {active}", {
                baseline: info.unlockedBaselineCount,
                unlocked: info.unlockedCount,
                active: info.activeCount,
              }),
            )}</div>
          </article>`;
      })
      .join("");

    let focusHtml = `
      <section id="buildDetailPanel" class="card buildPanelSection buildFocusPanel buildStaticPanel">
        <div class="small buildPanelEyebrow">${escapeHtml(t("build.focusTitle", "Focused Option"))}</div>
        <h2>${escapeHtml(t("build.availableOptions", "Available Options"))}</h2>
        <div class="small">${escapeHtml(t("build.focusEmpty", "Unlock or activate an option to inspect it here."))}</div>
      </section>`;

    if (focusOptionView) {
      const focusDisciplineLabel = t("build." + focusOptionView.discipline, humanizeDiscipline(focusOptionView.discipline));
      const focusSlotLabel = focusOptionView.activeSlot ? slotTitle(focusOptionView.activeSlot, t) : t("build.none", "No options");
      focusHtml = `
        <section id="buildDetailPanel" class="card buildPanelSection buildFocusPanel buildStaticPanel ${focusOptionView.disciplineClass}">
          <div class="small buildPanelEyebrow">${escapeHtml(t("build.focusTitle", "Focused Option"))}</div>
          <h2>${escapeHtml(focusOptionView.title)}</h2>
          <div class="buildFocusStateRow">
            <span class="disciplineMark ${focusOptionView.disciplineClass}">${escapeHtml(focusDisciplineLabel)}</span>
            <span class="pill">${escapeHtml(focusOptionView.stateText)}</span>
          </div>
          <div class="small buildFocusDescription">${escapeHtml(focusOptionView.description)}</div>
          <div class="buildFocusMeta">
            <div class="buildFocusMetaItem">
              <span class="small">${escapeHtml(t("build.requiresScoreLabel", "Unlock"))}</span>
              <strong>${escapeHtml(focusOptionView.unlockText)}</strong>
            </div>
            <div class="buildFocusMetaItem">
              <span class="small">${escapeHtml(t("build.compatibleSlotsLabel", "Compatible Slots"))}</span>
              <strong>${escapeHtml(focusOptionView.compatibleText)}</strong>
            </div>
            <div class="buildFocusMetaItem">
              <span class="small">${escapeHtml(t("build.activeSlotLabel", "Current Slot"))}</span>
              <strong>${escapeHtml(focusSlotLabel)}</strong>
            </div>
          </div>
          <div class="buildFocusActions">${focusOptionView.actionHtml}</div>
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
        <div class="buildPanels buildShrineGrid buildFixedSurface">
          <section id="buildCardsPanel" class="card buildPanelSection buildOptionRail">
            <div class="buildOptionRailHeader">
              <div class="small buildPanelEyebrow">${escapeHtml(t("build.disciplineLedger", "Discipline"))}</div>
              <h2>${escapeHtml(t("build.availableOptions", "Available Options"))}</h2>
              <div class="small buildOptionRailLead">${escapeHtml(
                t("build.help", "Build score opens options permanently, but only your active slots apply to the current build."),
              )}</div>
            </div>
            <div id="buildCardsScroller" class="buildOptionRailBody" data-wheel-surface="build-options">
              <div class="buildCardsGrid">
                ${cardsHtml}
              </div>
            </div>
          </section>
          <section id="buildSlotsPanel" class="card buildPanelSection buildAltarPanel buildStaticPanel">
            <div class="small buildPanelEyebrow">${escapeHtml(t("build.altarTitle", "Build Shrine"))}</div>
            <h2>${escapeHtml(t("build.activeSlots", "Active Slots"))}</h2>
            <div class="small buildActionLegend">${escapeHtml(
              `${t("build.activate", "Activate")} / ${t("build.deactivate", "Deactivate")} / ${t("build.swap", "Swap")}`,
            )}</div>
            <div class="buildSlotStage">
              <div class="buildSlotCluster">${slotHtml || ""}</div>
            </div>
          </section>
          ${focusHtml}
        </div>
      </div>`;
  }

  return Object.freeze({
    renderBuildPanelHtml,
  });
});
