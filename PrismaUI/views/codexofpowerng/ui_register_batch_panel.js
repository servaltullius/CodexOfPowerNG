(function (root, factory) {
  const api = factory();
  if (typeof module === "object" && module.exports) {
    module.exports = api;
  }
  if (root) {
    root.COPNGRegisterBatchPanel = api;
  }
})(typeof globalThis !== "undefined" ? globalThis : this, function () {
  "use strict";

  function asFn(maybeFn, fallback) {
    return typeof maybeFn === "function" ? maybeFn : fallback;
  }

  function defaultEscapeHtml(value) {
    return String(value == null ? "" : value)
      .replace(/&/g, "&amp;")
      .replace(/</g, "&lt;")
      .replace(/>/g, "&gt;")
      .replace(/\"/g, "&quot;")
      .replace(/'/g, "&#39;");
  }

  function defaultT(_key, fallback) {
    return fallback;
  }

  function defaultTFmt(_key, fallback, vars) {
    return String(fallback).replace(/\{([a-zA-Z0-9_]+)\}/g, (match, name) =>
      vars && Object.prototype.hasOwnProperty.call(vars, name) ? String(vars[name]) : match,
    );
  }

  function defaultToHex32(value) {
    const n = Number(value >>> 0);
    let hex = n.toString(16).toUpperCase();
    while (hex.length < 8) hex = "0" + hex;
    return "0x" + hex;
  }

  function humanizeDiscipline(id) {
    const value = String(id || "").toLowerCase();
    if (value === "attack") return "Attack";
    if (value === "defense") return "Defense";
    if (value === "utility") return "Utility";
    return "Unknown";
  }

  function buildStateTags(row) {
    const tags = [];
    if (!row) return tags;
    if (!row.actionable && row.disabledReason) tags.push(String(row.disabledReason));
    if (Number(row.safeCount || 0) <= 0 && tags.indexOf("not_actionable") === -1) tags.push("not_actionable");
    return tags;
  }

  function buildRegisterBatchViewModel(inventoryPage, selectedIds, filters) {
    const source = inventoryPage && typeof inventoryPage === "object" ? inventoryPage : {};
    const sections = Array.isArray(source.sections) ? source.sections : [];
    const selectedSet = new Set((Array.isArray(selectedIds) ? selectedIds : []).map((id) => Number(id) >>> 0));
    const options = filters && typeof filters === "object" ? filters : {};
    const actionableOnly = !!options.actionableOnly;
    const query = String(options.query || "").trim().toLowerCase();
    const disciplineGain = { attack: 0, defense: 0, utility: 0 };
    const rows = [];
    const disabledRows = [];
    const viewSections = [];
    const viewSectionByDiscipline = new Map();

    for (const section of sections) {
      const discipline = String((section && section.discipline) || "utility").toLowerCase();
      const sectionRows = Array.isArray(section && section.rows) ? section.rows : [];
      const nextSectionRows = [];

      for (const rawRow of sectionRows) {
        const row = rawRow && typeof rawRow === "object" ? rawRow : {};
        const name = String(row.name || "");
        if (query && name.toLowerCase().indexOf(query) === -1) continue;

        const actionable = !!row.actionable;
        if (actionableOnly && !actionable) continue;

        const formId = Number(row.formId || 0) >>> 0;
        const reasonTag = row.disabledReason ? String(row.disabledReason) : null;
        const canBatchSelect = actionable;
        const isSelected = canBatchSelect && selectedSet.has(formId);
        const vmRow = {
          formId,
          regKey: Number(row.regKey || 0) >>> 0,
          name: name || "(unnamed)",
          discipline,
          groupName: section.groupName || row.groupName || discipline,
          totalCount: Number(row.totalCount || 0) >>> 0,
          safeCount: Number(row.safeCount || 0) >>> 0,
          actionable,
          canBatchSelect,
          singleRegisterAction: actionable ? "enabled" : "disabled",
          reasonTag,
          stateTags: buildStateTags({
            discipline,
            actionable,
            disabledReason: reasonTag,
            safeCount: Number(row.safeCount || 0) >>> 0,
          }),
          isSelected,
        };

        rows.push(vmRow);
        nextSectionRows.push(vmRow);
        if (reasonTag) disabledRows.push(vmRow);
      }

      if (nextSectionRows.length > 0) {
        let viewSection = viewSectionByDiscipline.get(discipline);
        if (!viewSection) {
          viewSection = {
            discipline,
            label: humanizeDiscipline(discipline),
            rows: [],
          };
          viewSectionByDiscipline.set(discipline, viewSection);
          viewSections.push(viewSection);
        }
        viewSection.rows.push(...nextSectionRows);
      }
    }

    const formIds = rows.filter((row) => row.isSelected).map((row) => row.formId);
    for (const row of rows) {
      if (!row.isSelected) continue;
      if (Object.prototype.hasOwnProperty.call(disciplineGain, row.discipline)) {
        disciplineGain[row.discipline] += 1;
      }
    }

    return {
      listScope: "all_register_relevant",
      sections: viewSections,
      rows,
      disabledRows,
      summary: {
        selectedRows: formIds.length,
        disciplineGain,
        formIds,
      },
    };
  }

  function reasonText(reasonTag, t) {
    if (reasonTag === "quest_protected") return t("quick.reason.quest_protected", "Quest protected");
    if (reasonTag === "favorite_protected") return t("quick.reason.favorite_protected", "Favorite protected");
    return t("quick.reason.not_actionable", "Unavailable");
  }

  function renderRegisterBatchRowsHtml(viewModel, helpers) {
    const model = viewModel && typeof viewModel === "object" ? viewModel : buildRegisterBatchViewModel({}, [], {});
    const escapeHtml = asFn(helpers && helpers.escapeHtml, defaultEscapeHtml);
    const t = asFn(helpers && helpers.t, defaultT);
    const toHex32 = asFn(helpers && helpers.toHex32, defaultToHex32);
    const parts = [];

    for (const section of model.sections || []) {
      if (!section || !Array.isArray(section.rows) || section.rows.length === 0) continue;
      parts.push(`
          <tr class="sectionRow">
            <td colspan="5"><strong>${escapeHtml(t("build." + section.discipline, section.label || section.discipline))}</strong></td>
          </tr>`);

      for (const row of section.rows) {
        const tags = row.stateTags
          .map((tag) => `<span class="pill small">${escapeHtml(t("quick.reason." + tag, tag))}</span>`)
          .join(" ");
        const reason = row.reasonTag ? `<div class="small">${escapeHtml(reasonText(row.reasonTag, t))}</div>` : "";
        const checkbox = row.canBatchSelect
          ? `<input type="checkbox" data-action="batch-toggle" data-id="${row.formId}"${row.isSelected ? " checked" : ""} />`
          : `<input type="checkbox" disabled />`;
        const buttonDisabled = row.singleRegisterAction === "disabled" ? " disabled" : "";
        const rowIdAttr = row.singleRegisterAction === "enabled" ? String(row.formId) : "";

        parts.push(`
        <tr class="dataRow ${row.isSelected ? "selected" : ""}" data-row-id="${rowIdAttr}">
          <td class="colSelect">${checkbox}</td>
          <td class="colGroup"><span class="pill">${escapeHtml(t("build." + row.discipline, row.discipline))}</span></td>
          <td>
            <div class="itemName">${escapeHtml(row.name)}</div>
            <div class="small mono">${escapeHtml(toHex32(row.formId))}</div>
            <div class="small">${tags}</div>
            ${reason}
          </td>
          <td class="mono colCount"><span class="good">${row.safeCount}</span>/${row.totalCount}</td>
          <td class="colAction"><button class="primary" data-action="reg" data-id="${row.formId}"${buttonDisabled}><span class="btnLabel">${escapeHtml(
            t("btn.register", "Register"),
          )}</span></button></td>
        </tr>`);
      }
    }

    return parts.join("");
  }

  function renderRegisterBatchSummaryHtml(viewModel, helpers) {
    const model = viewModel && typeof viewModel === "object" ? viewModel : buildRegisterBatchViewModel({}, [], {});
    const escapeHtml = asFn(helpers && helpers.escapeHtml, defaultEscapeHtml);
    const t = asFn(helpers && helpers.t, defaultT);
    const tFmt = asFn(helpers && helpers.tFmt, defaultTFmt);
    return `
      <div class="quickBatchSummaryInner">
        <div class="small">${escapeHtml(tFmt("quick.batch.selected", "Selected rows: {count}", { count: model.summary.selectedRows }))}</div>
        <div class="small">${escapeHtml(
          tFmt("quick.batch.disciplineGain", "Attack +{attack} / Defense +{defense} / Utility +{utility}", model.summary.disciplineGain),
        )}</div>
      </div>`;
  }

  return Object.freeze({
    buildRegisterBatchViewModel,
    renderRegisterBatchRowsHtml,
    renderRegisterBatchSummaryHtml,
    renderRegisterBatchTbody: renderRegisterBatchRowsHtml,
  });
});
