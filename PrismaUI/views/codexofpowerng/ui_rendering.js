(function (root, factory) {
  const api = factory();
  if (typeof module === "object" && module.exports) {
    module.exports = api;
  }
  if (root) {
    root.COPNGUIRendering = api;
  }
})(typeof globalThis !== "undefined" ? globalThis : this, function () {
  "use strict";

  function noop() {}

  function asFn(maybeFn, fallback) {
    return typeof maybeFn === "function" ? maybeFn : fallback;
  }

  function defaultT(_key, fallback) {
    return fallback;
  }

  function defaultTFmt(_key, fallback, vars) {
    return String(fallback).replace(/\{([a-zA-Z0-9_]+)\}/g, (m, k) =>
      vars && Object.prototype.hasOwnProperty.call(vars, k) ? String(vars[k]) : m,
    );
  }

  function defaultCoalesce(value, fallback) {
    return value == null ? fallback : value;
  }

  function defaultClamp(value, lo, hi) {
    return Math.max(lo, Math.min(hi, value));
  }

  function defaultToHex32(value) {
    const n = Number(value >>> 0);
    let hex = n.toString(16).toUpperCase();
    while (hex.length < 8) hex = "0" + hex;
    return "0x" + hex;
  }

  function escapeHtml(value) {
    return String(value == null ? "" : value)
      .replace(/&/g, "&amp;")
      .replace(/</g, "&lt;")
      .replace(/>/g, "&gt;")
      .replace(/\"/g, "&quot;")
      .replace(/'/g, "&#39;");
  }

  function sanitizeI18nHtml(raw) {
    return String(raw == null ? "" : raw)
      .replace(/&/g, "&amp;")
      .replace(/</g, "&lt;")
      .replace(/>/g, "&gt;")
      .replace(/&lt;(\/?(span|br)\b[^&]*?)&gt;/gi, "<$1>");
  }

  function createUIRendering(opts) {
    const options = opts || {};
    const documentObj = options.documentObj || (typeof document !== "undefined" ? document : null);
    const refs = options.refs || {};
    const t = asFn(options.t, defaultT);
    const tFmt = asFn(options.tFmt, defaultTFmt);
    const coalesce = asFn(options.coalesce, defaultCoalesce);
    const clamp = asFn(options.clamp, defaultClamp);
    const toHex32 = asFn(options.toHex32, defaultToHex32);
    const showToast = asFn(options.showToast, noop);
    const updateToggleKeyResolved = asFn(options.updateToggleKeyResolved, noop);
    const setToggleKeyInputFromDik = asFn(options.setToggleKeyInputFromDik, noop);
    const setInputScale = asFn(options.setInputScale, noop);
    const applyUiScaleFromPrefs = asFn(options.applyUiScaleFromPrefs, noop);
    const applyPerfModeFromPrefs = asFn(options.applyPerfModeFromPrefs, noop);
    const scheduleVirtualRender = asFn(options.scheduleVirtualRender, noop);
    const getCurrentUiScale = asFn(options.getCurrentUiScale, () => 1);
    const getVirtualTableManager = asFn(options.getVirtualTableManager, () => null);
    const getUiLang = asFn(options.getUiLang, () => "en");
    const getState = asFn(options.getState, () => ({}));
    const getInventoryPage = asFn(options.getInventoryPage, () => ({ page: 0, pageSize: 200, total: 0, hasMore: false, items: [] }));
    const getQuickSelectedId = asFn(options.getQuickSelectedId, () => 0);
    const setQuickSelectedId = asFn(options.setQuickSelectedId, noop);
    const setQuickVisibleIds = asFn(options.setQuickVisibleIds, noop);
    const getQuickBatchSelectedIds = asFn(options.getQuickBatchSelectedIds, () => []);
    const setQuickBatchSelectedIds = asFn(options.setQuickBatchSelectedIds, noop);
    const getQuickActionableOnly = asFn(options.getQuickActionableOnly, () => false);
    const getRegistered = asFn(options.getRegistered, () => []);
    const getUndoItems = asFn(options.getUndoItems, () => []);
    const getBuild = asFn(options.getBuild, () => ({
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
    }));
    const getRewards = asFn(options.getRewards, () => ({ totals: [] }));
    const getSettings = asFn(options.getSettings, () => null);
    const getInputScale = asFn(options.getInputScale, () => 1);
    const getLangMenuOpen = asFn(options.getLangMenuOpen, () => false);
    const setLangMenuOpen = asFn(options.setLangMenuOpen, noop);
    const getLotdGateBlockingToastShown = asFn(options.getLotdGateBlockingToastShown, () => false);
    const setLotdGateBlockingToastShown = asFn(options.setLotdGateBlockingToastShown, noop);
    const rewardOrbitApi = options.rewardOrbitApi || null;
    const buildPanelApi = options.buildPanelApi || null;
    const registerBatchPanelApi = options.registerBatchPanelApi || null;
    const langUiApi = options.langUiApi || null;

    function syncRewardCharacterImageState() {
      const characterImgEl = refs.rewardCharacterImgEl;
      const fallbackEl = refs.rewardImageFallbackEl;
      if (rewardOrbitApi && typeof rewardOrbitApi.syncRewardCharacterImageState === "function") {
        rewardOrbitApi.syncRewardCharacterImageState({
          characterImgEl,
          fallbackEl,
        });
        return;
      }
      if (!characterImgEl || !fallbackEl) return;
      const loaded = !!(characterImgEl.complete && characterImgEl.naturalWidth > 0);
      characterImgEl.style.display = loaded ? "" : "none";
      fallbackEl.classList.toggle("show", !loaded);
    }

    function renderRewardOrbit(rows) {
      const rewardOrbitEl = refs.rewardOrbitEl;
      if (!rewardOrbitEl) return;
      if (rewardOrbitApi && typeof rewardOrbitApi.renderRewardOrbit === "function") {
        rewardOrbitApi.renderRewardOrbit({
          orbitEl: rewardOrbitEl,
          rows,
          scale: getCurrentUiScale(),
          viewportW: typeof window !== "undefined" ? Number(window.innerWidth || 0) : 0,
          escapeHtml,
          t,
          tFmt,
          coalesce,
          onAfterRender: syncRewardCharacterImageState,
        });
        return;
      }

      const srcRows = Array.isArray(rows) ? rows : [];
      rewardOrbitEl.style.setProperty("--rewardNodeWidth", "188px");
      if (srcRows.length === 0) {
        rewardOrbitEl.innerHTML = `<div class="rewardOrbitEmpty small">${escapeHtml(t("rewards.none", "(No rewards yet)"))}</div>`;
        syncRewardCharacterImageState();
        return;
      }
      rewardOrbitEl.innerHTML = "";
      syncRewardCharacterImageState();
    }

    function closeLangMenu() {
      setLangMenuOpen(false);
      if (refs.langMenuEl) refs.langMenuEl.classList.remove("open");
    }

    function syncLangDropdown() {
      const langSelectEl = refs.langSelectEl;
      const langBtnLabelEl = refs.langBtnLabelEl;
      if (!langSelectEl || !langBtnLabelEl) return;
      const nextValue =
        langUiApi && typeof langUiApi.normalizeLanguageValue === "function"
          ? langUiApi.normalizeLanguageValue(langSelectEl.value)
          : langSelectEl.value || "auto";
      if (langSelectEl.value !== nextValue) langSelectEl.value = nextValue;
      langBtnLabelEl.textContent = t("lang." + nextValue, nextValue);
    }

    function openLangMenu() {
      const langMenuEl = refs.langMenuEl;
      const langSelectEl = refs.langSelectEl;
      if (!langMenuEl) return;
      const items =
        langUiApi && typeof langUiApi.getLanguageItems === "function"
          ? langUiApi.getLanguageItems(t)
          : [
              { value: "auto", label: t("lang.auto", "auto") },
              { value: "en", label: t("lang.en", "en") },
              { value: "ko", label: t("lang.ko", "ko") },
            ];
      const current = langSelectEl ? String(langSelectEl.value || "auto") : "auto";

      langMenuEl.innerHTML = items
        .map((item) => {
          const active = item.value === current;
          const cls = "dropdownItem" + (active ? " active" : "");
          return `<button type="button" class="${cls}" data-value="${escapeHtml(item.value)}">${escapeHtml(item.label)}</button>`;
        })
        .join("");

      langMenuEl.querySelectorAll("button[data-value]").forEach((btn) => {
        btn.addEventListener("click", () => {
          const value = btn.getAttribute("data-value") || "auto";
          if (langSelectEl) langSelectEl.value = value;
          syncLangDropdown();
          closeLangMenu();
        });
      });

      setLangMenuOpen(true);
      langMenuEl.classList.add("open");
    }

    function getActiveTabId() {
      if (!documentObj || typeof documentObj.querySelector !== "function") return "tabQuick";
      const activeSection = documentObj.querySelector(".section.active");
      const activeTabId = activeSection && typeof activeSection.id === "string" ? activeSection.id : "";
      return activeTabId || "tabQuick";
    }

    function isTabActive(tabId) {
      return getActiveTabId() === String(tabId || "");
    }

    function setTab(tabId) {
      if (!documentObj) return;
      documentObj.querySelectorAll(".tabs button").forEach((btn) =>
        btn.classList.toggle("active", btn.dataset.tab === tabId),
      );
      documentObj.querySelectorAll(".section").forEach((sec) =>
        sec.classList.toggle("active", sec.id === tabId),
      );
      renderTab(tabId);
      scheduleVirtualRender({ force: true });
    }

    function renderStatus() {
      const state = getState() || {};
      const ui = state.ui || {};
      if (refs.statusEl) {
        refs.statusEl.textContent = `${t("status.ui", "UI")}: ${ui.ready ? t("status.ready", "ready") : t("status.loading", "loading")} | ${
          ui.hidden ? t("status.hidden", "hidden") : t("status.shown", "shown")
        } | ${ui.focused ? t("status.focus", "focus") : t("status.noFocus", "no-focus")}`;
      }
      if (refs.countsEl) refs.countsEl.textContent = `${t("status.registered", "Registered")}: ${coalesce(state.registeredCount, 0)}`;
      if (refs.langEl) refs.langEl.textContent = `${t("status.lang", "Lang")}: ${coalesce(state.language, "?")}`;
      if (refs.hotkeyEl) {
        refs.hotkeyEl.textContent = `${t("status.hotkey", "Hotkey")}: ${asFn(options.formatToggleKeyDisplay, defaultToHex32)(
          coalesce(state.toggleKeyCode, 0) >>> 0,
        )}`;
      }
      if (refs.inputScalePillEl) refs.inputScalePillEl.textContent = `Input: ${Number(getInputScale() || 0).toFixed(2)}`;

      if (refs.lotdGatePillEl) {
        const gate = (state && state.lotdGate) || {};
        const enabled = !!gate.requireTccDisplayed;
        const blocking = !!gate.blocking;

        if (!enabled) {
          refs.lotdGatePillEl.style.display = "none";
          refs.lotdGatePillEl.classList.remove("warn", "ok");
          setLotdGateBlockingToastShown(false);
        } else {
          refs.lotdGatePillEl.style.display = "";
          refs.lotdGatePillEl.classList.toggle("warn", blocking);
          refs.lotdGatePillEl.classList.toggle("ok", !blocking);
          refs.lotdGatePillEl.textContent = `${t("status.lotdGate", "LOTD Gate")}: ${
            blocking ? t("status.lotdBlocked", "TCC missing") : t("status.lotdOk", "TCC ready")
          }`;

          if (blocking && !getLotdGateBlockingToastShown()) {
            showToast(
              "warn",
              t("toast.lotdGateBlocked", "LOTD Display gate is enabled, but TCC lists are unavailable. Registration is blocked."),
            );
            setLotdGateBlockingToastShown(true);
          } else if (!blocking) {
            setLotdGateBlockingToastShown(false);
          }
        }
      }

      if (refs.lotdGateWarnBannerEl) {
        const gate = (state && state.lotdGate) || {};
        const enabled = !!gate.requireTccDisplayed;
        const blocking = !!gate.blocking;
        refs.lotdGateWarnBannerEl.style.display = enabled && blocking ? "" : "none";
      }
    }

    function renderQuickVirtual(opts) {
      const mgr = getVirtualTableManager();
      if (!mgr) return;
      mgr.renderQuickVirtual(opts || { force: false });
    }

    function renderRegisteredVirtual(opts) {
      const mgr = getVirtualTableManager();
      if (!mgr) return;
      mgr.renderRegisteredVirtual(opts || { force: false });
    }

    function renderQuick() {
      const inventoryPage = getInventoryPage() || {};
      const quickFilterEl = documentObj ? documentObj.getElementById("quickFilter") : null;
      const quickActionableOnlyEl = documentObj ? documentObj.getElementById("quickActionableOnly") : null;
      const query = String((quickFilterEl && quickFilterEl.value) || "").toLowerCase();
      if (quickActionableOnlyEl) quickActionableOnlyEl.checked = !!getQuickActionableOnly();
      if (registerBatchPanelApi && typeof registerBatchPanelApi.buildRegisterBatchViewModel === "function") {
        const viewModel = registerBatchPanelApi.buildRegisterBatchViewModel(inventoryPage, getQuickBatchSelectedIds(), {
          actionableOnly: getQuickActionableOnly(),
          query,
        });

        if (refs.quickBody && refs.quickBody.dataset) refs.quickBody.dataset.virtualMode = "grouped";
        if (refs.quickVirtual) refs.quickVirtual.rows = [];
        const actionableIds = (Array.isArray(viewModel.rows) ? viewModel.rows : [])
          .filter((row) => row && row.canBatchSelect)
          .map((row) => Number(row.formId) >>> 0);
        setQuickVisibleIds(actionableIds);
        if (getQuickSelectedId() && actionableIds.indexOf(getQuickSelectedId()) === -1) {
          setQuickSelectedId(0);
        }
        const validSelected = Array.isArray(viewModel.summary && viewModel.summary.formIds)
          ? viewModel.summary.formIds
          : [];
        setQuickBatchSelectedIds(validSelected);

        if (refs.quickBody && typeof registerBatchPanelApi.renderRegisterBatchTbody === "function") {
          refs.quickBody.innerHTML = registerBatchPanelApi.renderRegisterBatchTbody(viewModel, {
            t,
            escapeHtml,
            toHex32,
          });
        }

        if (refs.quickBatchMetaEl) {
          refs.quickBatchMetaEl.textContent = tFmt("quick.selectedRows", "Selected {count}", {
            count: Number((viewModel.summary && viewModel.summary.selectedRows) || 0) >>> 0,
          });
        }
        if (refs.quickBatchGainEl) {
          const gain = (viewModel.summary && viewModel.summary.disciplineGain) || {};
          refs.quickBatchGainEl.textContent = tFmt(
            "quick.disciplineGain",
            "Attack +{attack} / Defense +{defense} / Utility +{utility}",
            {
              attack: Number(gain.attack || 0) >>> 0,
              defense: Number(gain.defense || 0) >>> 0,
              utility: Number(gain.utility || 0) >>> 0,
            },
          );
        }
        if (refs.btnRegisterBatchEl) {
          refs.btnRegisterBatchEl.disabled = !viewModel.summary || !Array.isArray(viewModel.summary.formIds) || viewModel.summary.formIds.length === 0;
        }

        const total = coalesce(inventoryPage.total, viewModel.rows.length);
        if (refs.invMetaEl) {
          refs.invMetaEl.textContent = `${t("inv.inventory", "Inventory")}: ${t("inv.showing", "showing")} ${viewModel.rows.length}/${total}`;
        }
        if (refs.invPageSizeEl) {
          const shownSize = String(coalesce(inventoryPage.pageSize, 200) || 200);
          if (refs.invPageSizeEl.value !== shownSize) refs.invPageSizeEl.value = shownSize;
        }
        if (refs.btnInvPrev) refs.btnInvPrev.disabled = true;
        if (refs.btnInvNext) refs.btnInvNext.disabled = true;
        return;
      }

      if (refs.quickBody && refs.quickBody.dataset) delete refs.quickBody.dataset.virtualMode;
      const items = Array.isArray(inventoryPage.items) ? inventoryPage.items : [];
      const rows = items.filter((item) => !query || String(item.name || "").toLowerCase().indexOf(query) !== -1);
      if (refs.quickVirtual) refs.quickVirtual.rows = rows;
      const visibleIds = rows.map((item) => Number(item.formId) >>> 0);
      setQuickVisibleIds(visibleIds);
      if (getQuickSelectedId() && visibleIds.indexOf(getQuickSelectedId()) === -1) {
        setQuickSelectedId(0);
      }
      scheduleVirtualRender({ force: true });

      const page = coalesce(inventoryPage.page, 0);
      const pageSize = coalesce(inventoryPage.pageSize, 0);
      const total = coalesce(inventoryPage.total, 0);
      const totalPages = total > 0 && pageSize > 0 ? Math.max(1, Math.ceil(total / pageSize)) : 0;

      if (refs.invMetaEl) {
        const pagesText = totalPages > 0 ? `${page + 1}/${totalPages}` : `${page + 1}/${t("inv.unknown", "unknown")}`;
        const totalText = total > 0 ? String(total) : t("inv.unknown", "unknown");
        refs.invMetaEl.textContent = `${t("inv.inventory", "Inventory")}: ${t("inv.page", "page")} ${pagesText} (${t(
          "inv.showing",
          "showing",
        )} ${rows.length}/${items.length}, ${t("inv.total", "total")} ${totalText})`;
      }
      if (refs.invPageSizeEl) {
        const shownSize = String(pageSize > 0 ? pageSize : 200);
        if (refs.invPageSizeEl.value !== shownSize) refs.invPageSizeEl.value = shownSize;
      }
      if (refs.btnInvPrev) refs.btnInvPrev.disabled = page <= 0;
      if (refs.btnInvNext) {
        refs.btnInvNext.disabled = totalPages > 0 ? page + 1 >= totalPages : !inventoryPage.hasMore;
      }
    }

    function renderRegistered() {
      const regFilterEl = documentObj ? documentObj.getElementById("regFilter") : null;
      const query = String((regFilterEl && regFilterEl.value) || "").toLowerCase();
      const registered = Array.isArray(getRegistered()) ? getRegistered() : [];
      const rows = registered.filter((item) => !query || String(item.name || "").toLowerCase().indexOf(query) !== -1);
      if (refs.regVirtual) refs.regVirtual.rows = rows;
      scheduleVirtualRender({ force: true });
    }

    function renderUndo() {
      if (!refs.undoBody) return;
      const rows = Array.isArray(getUndoItems()) ? getUndoItems() : [];

      if (refs.undoMetaEl) {
        refs.undoMetaEl.textContent = `${t("undo.meta", "Recent registrations (latest first)")} (${rows.length}/${10})`;
      }

      refs.undoBody.innerHTML =
        rows.length === 0
          ? `<tr><td colspan="4" class="small">${escapeHtml(t("undo.none", "(No recent registrations)"))}</td></tr>`
          : rows
              .map((item, index) => {
                const actionId = Number(item.actionId || 0);
                const canUndo = !!item.canUndo;
                const rewardText = item.hasRewardDelta ? t("undo.rewarded", "rewarded") : t("undo.noReward", "no reward");
                const buttonText = canUndo ? t("btn.undo", "Undo") : t("undo.onlyLatest", "Latest only");
                const disabled = canUndo ? "" : " disabled";
                const actionClass = canUndo ? "danger" : "";
                return `
          <tr class="dataRow ${index % 2 === 0 ? "rowOdd" : ""}">
            <td class="colGroup"><span class="pill">${escapeHtml(item.groupName || String(item.group || ""))}</span></td>
            <td>
              <span class="itemName">${escapeHtml(item.name || "(unnamed)")}</span>
              <span class="small mono">${escapeHtml(rewardText)}</span>
            </td>
            <td class="colFormId mono">${toHex32(Number(item.formId || 0) >>> 0)}</td>
            <td class="colAction"><button class="${actionClass}" data-action="undo" data-id="${actionId}"${disabled}><span class="btnLabel">${escapeHtml(
                  buttonText,
                )}</span></button></td>
          </tr>`;
              })
              .join("");
    }

    function renderRewards() {
      const rewards = getRewards() || {};
      if (refs.rewardMetaEl) {
        refs.rewardMetaEl.textContent = `${t("rewards.rolls", "Rolls")}: ${coalesce(rewards.rolls, 0)} | ${t(
          "rewards.every",
          "Every",
        )}: ${coalesce(rewards.rewardEvery, "?")} | ${t("rewards.mult", "Mult")}: ${coalesce(rewards.rewardMultiplier, "?")}`;
      }
      const rows = (Array.isArray(rewards.totals) ? rewards.totals : [])
        .slice()
        .sort((a, b) => String(a.label || "").localeCompare(String(b.label || "")));
      renderRewardOrbit(rows);

      if (!refs.rewardsBody) return;
      refs.rewardsBody.innerHTML =
        rows.length === 0
          ? `<tr><td colspan="2" class="small">${escapeHtml(t("rewards.none", "(No rewards yet)"))}</td></tr>`
          : rows
              .map(
                (item, index) => `
          <tr class="dataRow ${index % 2 === 0 ? "rowOdd" : ""}">
            <td>${escapeHtml(item.label || "Unknown")}</td>
            <td class="mono colCount">${escapeHtml(item.display || String(coalesce(item.total, 0)))}</td>
          </tr>`,
              )
              .join("");
    }

    function renderBuild() {
      const build = getBuild() || {};
      const disciplines = build.disciplines || {};
      if (refs.buildMetaEl) {
        refs.buildMetaEl.textContent = tFmt("build.scoreSummary", "Attack {attack} / Defense {defense} / Utility {utility}", {
          attack: Number((disciplines.attack && disciplines.attack.score) || 0) >>> 0,
          defense: Number((disciplines.defense && disciplines.defense.score) || 0) >>> 0,
          utility: Number((disciplines.utility && disciplines.utility.score) || 0) >>> 0,
        });
      }
      if (!refs.buildPanelEl) return;
      if (buildPanelApi && typeof buildPanelApi.renderBuildPanelHtml === "function") {
        refs.buildPanelEl.innerHTML = buildPanelApi.renderBuildPanelHtml(build, {
          t,
          tFmt,
          escapeHtml,
        });
        return;
      }

      refs.buildPanelEl.innerHTML = `
        <div id="buildMigrationNotice"></div>
        <section id="buildSlotsPanel"><h2>${escapeHtml(t("build.activeSlots", "Active Slots"))}</h2></section>
        <section id="buildCardsPanel"><h2>${escapeHtml(t("build.availableOptions", "Available Options"))}</h2></section>`;
    }

    function renderSettings() {
      const settings = getSettings();
      if (!settings || !documentObj) return;
      setToggleKeyInputFromDik(coalesce(settings.toggleKeyCode, 0) >>> 0);
      if (refs.langSelectEl) {
        refs.langSelectEl.value = coalesce(settings.languageOverride, "auto");
        syncLangDropdown();
      }

      const assignChecked = (id, value) => {
        const el = documentObj.getElementById(id);
        if (el) el.checked = !!value;
      };
      const assignValue = (id, value) => {
        const el = documentObj.getElementById(id);
        if (el) el.value = value;
      };

      assignChecked("setPauseGame", settings.uiPauseGame);
      assignChecked("setDisableFocusMenu", settings.uiDisableFocusMenu);
      assignChecked("setDestroyOnClose", settings.uiDestroyOnClose);
      assignChecked("setNormalize", settings.normalizeRegistration);
      assignChecked("setRequireTccDisplayed", settings.requireTccDisplayed);
      assignChecked("setProtectFav", settings.protectFavorites);
      assignChecked("setLootNotify", settings.enableLootNotify);

      const desiredInputScale = parseFloat(String(coalesce(settings.uiInputScale, 1.0)));
      setInputScale(Number.isFinite(desiredInputScale) && desiredInputScale > 0 ? desiredInputScale : 1.0, { persist: false });
      applyUiScaleFromPrefs();

      if (refs.perfModeEl) refs.perfModeEl.value = coalesce(options.getPerfMode ? options.getPerfMode() : "auto", "auto");
      applyPerfModeFromPrefs();
    }

    function renderTab(tabId) {
      const nextTabId = String(tabId || "");
      if (nextTabId === "tabRegistered") {
        renderRegistered();
        return;
      }
      if (nextTabId === "tabUndo") {
        renderUndo();
        return;
      }
      if (nextTabId === "tabBuild") {
        renderBuild();
        return;
      }
      if (nextTabId === "tabSettings") {
        renderSettings();
        return;
      }
      renderQuick();
    }

    function renderActiveTab() {
      renderTab(getActiveTabId());
    }

    function applyI18n() {
      if (!documentObj) return;
      documentObj.querySelectorAll("[data-i18n-html]").forEach((el) => {
        const key = el.getAttribute("data-i18n-html");
        if (!key) return;
        el.innerHTML = sanitizeI18nHtml(t(key, el.innerHTML));
      });
      documentObj.querySelectorAll("[data-i18n]").forEach((el) => {
        const key = el.getAttribute("data-i18n");
        if (!key) return;
        el.textContent = t(key, el.textContent);
      });
      documentObj.querySelectorAll("[data-i18n-placeholder]").forEach((el) => {
        const key = el.getAttribute("data-i18n-placeholder");
        if (!key) return;
        el.setAttribute("placeholder", t(key, el.getAttribute("placeholder") || ""));
      });

      syncLangDropdown();
      updateToggleKeyResolved();
      renderActiveTab();
    }

    return Object.freeze({
      escapeHtml,
      sanitizeI18nHtml,
      closeLangMenu,
      syncLangDropdown,
      openLangMenu,
      getActiveTabId,
      isTabActive,
      setTab,
      renderTab,
      renderActiveTab,
      renderStatus,
      renderQuickVirtual,
      renderRegisteredVirtual,
      renderQuick,
      renderRegistered,
      renderUndo,
      renderBuild,
      syncRewardCharacterImageState,
      renderRewardOrbit,
      renderRewards,
      renderSettings,
      applyI18n,
      isLangMenuOpen: getLangMenuOpen,
    });
  }

  return {
    escapeHtml,
    sanitizeI18nHtml,
    createUIRendering,
  };
});
