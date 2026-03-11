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
    const slotHtml = activeSlots
      .map((slot) => {
        const slotId = String((slot && slot.slotId) || "");
        const slotKind = String((slot && slot.slotKind) || "");
        const option = optionById.get(slot && slot.optionId) || null;
        const slotTitle = t("build.slot." + slotId, humanizeDiscipline(slotKind));
        const stateLabel = option ? t("build.active", "Active") : t("build.emptySlot", "Empty slot");
        const optionTitle = option
          ? t(option.titleKey, humanizeOptionId(option.id))
          : t("build.emptySlot", "Empty slot");
        const actionButton = option
          ? `<button type="button" data-action="build-deactivate" data-slot-id="${escapeHtml(slotId)}">${escapeHtml(
              t("build.deactivate", "Deactivate"),
            )}</button>`
          : "";
        return `
          <article class="buildSlotCard" data-slot-id="${escapeHtml(slotId)}">
            <div class="buildSlotCardHeader">
              <span class="pill">${escapeHtml(slotTitle)}</span>
              <span class="small">${escapeHtml(stateLabel)}</span>
            </div>
            <strong>${escapeHtml(optionTitle)}</strong>
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
        const info = disciplines[discipline] || { score: 0, unlockedBaselineCount: 0 };
        const label = t("build." + discipline, humanizeDiscipline(discipline));
        const rows = options
          .filter((option) => String((option && option.discipline) || "").toLowerCase() === discipline)
          .map((option) => {
            const optionId = String(option.id || "");
            const unlockScore = Number(option.unlockScore || 0) >>> 0;
            const compatibleSlots = activeSlots.filter((slot) => slotSupportsOption(slot, option));
            const activeSlot = compatibleSlots.find((slot) => slot && slot.optionId === optionId) || null;
            const emptySlot = compatibleSlots.find((slot) => !slot || !slot.optionId) || null;
            const unlocked = !!option.unlocked;
            const stateKey = activeSlot ? "build.active" : unlocked ? "build.unlocked" : "build.locked";
            const stateText = t(stateKey, activeSlot ? "Active" : unlocked ? "Unlocked" : "Locked");
            let actionHtml = `<span class="small">${escapeHtml(
              tFmt("build.requiresScore", "Need {score} Score", { score: unlockScore }),
            )}</span>`;

            if (unlocked) {
              if (activeSlot) {
                actionHtml = `<button type="button" data-action="build-deactivate" data-slot-id="${escapeHtml(
                  String(activeSlot.slotId || ""),
                )}">${escapeHtml(t("build.deactivate", "Deactivate"))}</button>`;
              } else if (emptySlot) {
                actionHtml = `<button type="button" data-action="build-activate" data-option-id="${escapeHtml(
                  optionId,
                )}" data-slot-id="${escapeHtml(String(emptySlot.slotId || ""))}">${escapeHtml(
                  t("build.activate", "Activate"),
                )}</button>`;
              } else {
                actionHtml = `<span class="small">${escapeHtml(
                  t("build.slotOccupiedHint", "Deactivate a compatible slot first."),
                )}</span>`;
              }
            }

            return `
              <article class="buildOptionCard ${activeSlot ? "isActive" : unlocked ? "isUnlocked" : "isLocked"}" data-option-id="${escapeHtml(
                optionId,
              )}">
                <div class="buildOptionHeader">
                  <strong>${escapeHtml(t(option.titleKey, humanizeOptionId(optionId)))}</strong>
                  <span class="pill">${escapeHtml(stateText)}</span>
                </div>
                <div class="small">${escapeHtml(t(option.descriptionKey, ""))}</div>
                <div class="small mono">${escapeHtml(
                  tFmt("build.requiresScore", "Need {score} Score", { score: unlockScore }),
                )}</div>
                <div class="buildOptionActions">${actionHtml}</div>
              </article>`;
          })
          .join("");

        return `
          <section class="buildDisciplineSection" data-discipline="${escapeHtml(discipline)}">
            <div class="buildDisciplineHeader">
              <h3>${escapeHtml(label)}</h3>
              <div class="buildDisciplineStats">
                <span class="pill">${escapeHtml(tFmt("build.scorePill", "Score {score}", { score: Number(info.score || 0) >>> 0 }))}</span>
                <span class="small">${escapeHtml(
                  tFmt("build.baselinePill", "Baseline {count}", {
                    count: Number(info.unlockedBaselineCount || 0) >>> 0,
                  }),
                )}</span>
              </div>
            </div>
            <div class="buildOptionList">${rows || `<div class="small">${escapeHtml(t("build.none", "No options"))}</div>`}</div>
          </section>`;
      })
      .join("");

    return `
      <div class="buildBoardInner">
        <div class="buildMetaStrip">
          <span class="pill">${escapeHtml(
            tFmt("build.scoreSummary", "Attack {attack} / Defense {defense} / Utility {utility}", {
              attack: Number((disciplines.attack && disciplines.attack.score) || 0) >>> 0,
              defense: Number((disciplines.defense && disciplines.defense.score) || 0) >>> 0,
              utility: Number((disciplines.utility && disciplines.utility.score) || 0) >>> 0,
            }),
          )}</span>
        </div>
        <div id="buildMigrationNotice" class="buildMigrationNotice${migrationParts.length ? "" : " isEmpty"}">
          ${migrationParts.join("")}
        </div>
        <div class="buildPanels">
          <section id="buildSlotsPanel" class="card buildPanelSection">
            <h2>${escapeHtml(t("build.activeSlots", "Active Slots"))}</h2>
            <div class="small buildActionLegend">${escapeHtml(
              `${t("build.activate", "Activate")} / ${t("build.deactivate", "Deactivate")} / ${t("build.swap", "Swap")}`,
            )}</div>
            <div class="buildSlotsGrid">${slotHtml || ""}</div>
          </section>
          <section id="buildCardsPanel" class="buildPanelSection">
            <h2>${escapeHtml(t("build.availableOptions", "Available Options"))}</h2>
            <div class="buildCardsGrid">${cardsHtml}</div>
          </section>
        </div>
      </div>`;
  }

  return Object.freeze({
    renderBuildPanelHtml,
  });
});
