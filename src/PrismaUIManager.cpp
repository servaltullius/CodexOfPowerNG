#include "CodexOfPowerNG/PrismaUIManager.h"

#include "CodexOfPowerNG/Config.h"
#include "CodexOfPowerNG/Constants.h"
#include "CodexOfPowerNG/L10n.h"
#include "CodexOfPowerNG/Registration.h"
#include "CodexOfPowerNG/Rewards.h"
#include "CodexOfPowerNG/State.h"

#include <Windows.h>

#include <SKSE/SKSE.h>
#include <SKSE/Interfaces.h>
#include <SKSE/Logger.h>

#include "PrismaUI_API.h"

#include <RE/M/MenuCursor.h>
#include <RE/U/UIMessageQueue.h>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <atomic>
#include <charconv>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

namespace CodexOfPowerNG::PrismaUIManager
{
	namespace
		{
			std::atomic<PRISMA_UI_API::IVPrismaUI1*> g_prismaAPI{ nullptr };
			std::atomic<PrismaView> g_view{ 0 };
			std::atomic_bool g_domReady{ false };
			std::atomic_bool g_toggleAllowed{ false };
			std::atomic<std::uint64_t> g_toggleAllowedAtMs{ 0 };
			std::atomic_bool g_openRequested{ false };
			std::atomic<std::uint64_t> g_lastToggleMs{ 0 };
			constexpr std::uint64_t kToggleDebounceMs = 350;
			std::atomic_bool g_viewHidden{ true };
			std::atomic_bool g_viewFocused{ false };
			std::atomic_bool g_focusDelayArmed{ false };
			std::atomic_int g_focusAttemptCount{ 0 };

			struct SettingsSaveJob
			{
				Settings settings{};
				bool     reloadL10n{ false };
			};

			std::mutex                      g_settingsSaveMutex;
			std::optional<SettingsSaveJob>   g_pendingSettingsSave;
			std::atomic_bool                g_settingsSaveWorkerRunning{ false };

			using json = nlohmann::json;

			void CallJS(const char* fn, const json& payload) noexcept;
			void Toast(std::string_view level, std::string message) noexcept;

			struct InventoryRequest
			{
				std::uint32_t page{ 0 };
			std::uint32_t pageSize{ 200 };
		};

		[[nodiscard]] std::uint64_t NowMs() noexcept
		{
			using namespace std::chrono;
			return static_cast<std::uint64_t>(
				duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
		}

		[[nodiscard]] InventoryRequest ParseInventoryRequest(const char* argument) noexcept
		{
			InventoryRequest out{};
			json payload;
			try {
				payload = json::parse(argument ? argument : "{}");
			} catch (...) {
				return out;
			}

			try {
				if (auto it = payload.find("page"); it != payload.end() && it->is_number_integer()) {
					const auto v = it->get<std::int64_t>();
					out.page = v > 0 ? static_cast<std::uint32_t>(v) : 0;
				}
				if (auto it = payload.find("pageSize"); it != payload.end() && it->is_number_integer()) {
					const auto v = it->get<std::int64_t>();
					const auto clamped = std::clamp<std::int64_t>(v, 1, 500);
					out.pageSize = static_cast<std::uint32_t>(clamped);
				}
			} catch (...) {
			}

			return out;
		}

		[[nodiscard]] std::optional<RE::FormID> ParseFormIDFromJson(const json& j) noexcept
		{
			try {
				if (j.is_number_unsigned()) {
					return static_cast<RE::FormID>(j.get<std::uint32_t>());
				}
				if (j.is_number_integer()) {
					const auto v = j.get<std::int64_t>();
					if (v < 0 || v > 0xFFFFFFFFll) {
						return std::nullopt;
					}
					return static_cast<RE::FormID>(static_cast<std::uint32_t>(v));
				}
				if (j.is_string()) {
					const auto s = j.get<std::string>();
					std::size_t idx = 0;
					int base = 10;
					if (s.rfind("0x", 0) == 0 || s.rfind("0X", 0) == 0) {
						base = 16;
						idx = 2;
					}
					std::uint32_t value{};
					const auto* begin = s.data() + idx;
					const auto* end = s.data() + s.size();
					auto [ptr, ec] = std::from_chars(begin, end, value, base);
					if (ec != std::errc{} || ptr != end) {
						return std::nullopt;
					}
					return static_cast<RE::FormID>(value);
				}
			} catch (...) {
			}

			return std::nullopt;
		}

		[[nodiscard]] json SettingsToJson(const Settings& s)
		{
			return json{
				{ "toggleKeyCode", s.toggleKeyCode },
				{ "uiDisableFocusMenu", s.uiDisableFocusMenu },
				{ "uiPauseGame", s.uiPauseGame },
				{ "uiInputScale", s.uiInputScale },
				{ "uiDestroyOnClose", s.uiDestroyOnClose },
				{ "languageOverride", s.languageOverride },
				{ "normalizeRegistration", s.normalizeRegistration },
				{ "protectFavorites", s.protectFavorites },
				{ "enableLootNotify", s.enableLootNotify },
				{ "enableRewards", s.enableRewards },
				{ "rewardEvery", s.rewardEvery },
				{ "rewardMultiplier", s.rewardMultiplier },
				{ "allowSkillRewards", s.allowSkillRewards },
				{ "corpseExplosionEnabled", s.corpseExplosionEnabled },
				{ "corpseExplosionChancePct", s.corpseExplosionChancePct },
				{ "corpseExplosionCooldownMs", s.corpseExplosionCooldownMs },
				{ "corpseExplosionVfxMode", s.corpseExplosionVfxMode },
				{ "corpseExplosionVfxEditorID", s.corpseExplosionVfxEditorID },
				{ "corpseExplosionNotify", s.corpseExplosionNotify },
			};
		}

		[[nodiscard]] bool NearlyEqual(double a, double b) noexcept
		{
			return std::fabs(a - b) <= 1e-9;
		}

		[[nodiscard]] bool SettingsEquivalent(const Settings& a, const Settings& b) noexcept
		{
			return a.toggleKeyCode == b.toggleKeyCode &&
			       a.uiDisableFocusMenu == b.uiDisableFocusMenu &&
			       a.uiPauseGame == b.uiPauseGame &&
			       NearlyEqual(a.uiInputScale, b.uiInputScale) &&
			       a.uiDestroyOnClose == b.uiDestroyOnClose &&
			       a.languageOverride == b.languageOverride &&
			       a.normalizeRegistration == b.normalizeRegistration &&
			       a.protectFavorites == b.protectFavorites &&
			       a.enableLootNotify == b.enableLootNotify &&
			       a.enableRewards == b.enableRewards &&
			       a.rewardEvery == b.rewardEvery &&
			       NearlyEqual(a.rewardMultiplier, b.rewardMultiplier) &&
			       a.allowSkillRewards == b.allowSkillRewards &&
			       a.corpseExplosionEnabled == b.corpseExplosionEnabled &&
			       NearlyEqual(a.corpseExplosionChancePct, b.corpseExplosionChancePct) &&
			       a.corpseExplosionCooldownMs == b.corpseExplosionCooldownMs &&
			       a.corpseExplosionVfxMode == b.corpseExplosionVfxMode &&
			       a.corpseExplosionVfxEditorID == b.corpseExplosionVfxEditorID &&
			       a.corpseExplosionNotify == b.corpseExplosionNotify;
		}

		void QueueSaveSettingsToDisk(SettingsSaveJob job) noexcept
		{
			{
				std::scoped_lock lock(g_settingsSaveMutex);
				g_pendingSettingsSave = std::move(job);
			}

			bool expected = false;
			if (!g_settingsSaveWorkerRunning.compare_exchange_strong(expected, true, std::memory_order_relaxed)) {
				return;
			}

			std::thread([]() {
				for (;;) {
					SettingsSaveJob job{};
					{
						std::scoped_lock lock(g_settingsSaveMutex);
						if (!g_pendingSettingsSave) {
							g_settingsSaveWorkerRunning.store(false, std::memory_order_relaxed);
							return;
						}
						job = std::move(*g_pendingSettingsSave);
						g_pendingSettingsSave.reset();
					}

					const auto ok = SaveSettingsToDisk(job.settings);
					bool needsMainThreadL10n = false;
					if (ok && job.reloadL10n) {
						// Reload localization. Avoid calling RE::GetINISetting from a background thread when in auto mode.
						const auto o = job.settings.languageOverride;
						const auto explicitLang = (o == "en" || o == "ko");
						if (explicitLang) {
							L10n::Load();
						} else {
							needsMainThreadL10n = true;
						}
					}

					if (auto* tasks = SKSE::GetTaskInterface(); tasks) {
						tasks->AddUITask([ok, needsMainThreadL10n]() {
							if (ok) {
								if (needsMainThreadL10n) {
									L10n::Load();
								}
								Toast("info", "Settings saved");
							} else {
								Toast("error", "Failed to save settings");
							}
							CallJS("copng_setSettings", SettingsToJson(GetSettings()));
							SendStateToUI();
						});
					}
				}
			}).detach();
		}

		[[nodiscard]] Settings SettingsFromJson(const json& j, Settings base)
		{
			try {
				if (auto it = j.find("toggleKeyCode"); it != j.end() && it->is_number_unsigned()) {
					base.toggleKeyCode = it->get<std::uint32_t>();
				}
				if (auto it = j.find("uiDisableFocusMenu"); it != j.end() && it->is_boolean()) {
					base.uiDisableFocusMenu = it->get<bool>();
				}
				if (auto it = j.find("uiPauseGame"); it != j.end() && it->is_boolean()) {
					base.uiPauseGame = it->get<bool>();
				}
				if (auto it = j.find("uiInputScale"); it != j.end() && (it->is_number_float() || it->is_number_integer())) {
					base.uiInputScale = it->get<double>();
				}
				if (auto it = j.find("uiDestroyOnClose"); it != j.end() && it->is_boolean()) {
					base.uiDestroyOnClose = it->get<bool>();
				}
				if (auto it = j.find("languageOverride"); it != j.end() && it->is_string()) {
					base.languageOverride = it->get<std::string>();
				}
				if (auto it = j.find("normalizeRegistration"); it != j.end() && it->is_boolean()) {
					base.normalizeRegistration = it->get<bool>();
				}
				if (auto it = j.find("protectFavorites"); it != j.end() && it->is_boolean()) {
					base.protectFavorites = it->get<bool>();
				}
				if (auto it = j.find("enableLootNotify"); it != j.end() && it->is_boolean()) {
					base.enableLootNotify = it->get<bool>();
				}
				if (auto it = j.find("enableRewards"); it != j.end() && it->is_boolean()) {
					base.enableRewards = it->get<bool>();
				}
				if (auto it = j.find("rewardEvery"); it != j.end() && it->is_number_integer()) {
					base.rewardEvery = it->get<std::int32_t>();
				}
				if (auto it = j.find("rewardMultiplier"); it != j.end() && (it->is_number_float() || it->is_number_integer())) {
					base.rewardMultiplier = it->get<double>();
				}
				if (auto it = j.find("allowSkillRewards"); it != j.end() && it->is_boolean()) {
					base.allowSkillRewards = it->get<bool>();
				}
				if (auto it = j.find("corpseExplosionEnabled"); it != j.end() && it->is_boolean()) {
					base.corpseExplosionEnabled = it->get<bool>();
				}
				if (auto it = j.find("corpseExplosionChancePct"); it != j.end() && (it->is_number_float() || it->is_number_integer())) {
					base.corpseExplosionChancePct = it->get<double>();
				}
				if (auto it = j.find("corpseExplosionCooldownMs"); it != j.end() && it->is_number_integer()) {
					base.corpseExplosionCooldownMs = it->get<std::int32_t>();
				}
				if (auto it = j.find("corpseExplosionVfxMode"); it != j.end() && it->is_string()) {
					base.corpseExplosionVfxMode = it->get<std::string>();
				}
				if (auto it = j.find("corpseExplosionVfxEditorID"); it != j.end() && it->is_string()) {
					base.corpseExplosionVfxEditorID = it->get<std::string>();
				}
				if (auto it = j.find("corpseExplosionNotify"); it != j.end() && it->is_boolean()) {
					base.corpseExplosionNotify = it->get<bool>();
				}
			} catch (...) {
			}
			return base;
		}

		[[nodiscard]] bool IsReady() noexcept
		{
			auto* api = g_prismaAPI.load(std::memory_order_acquire);
			const auto view = g_view.load(std::memory_order_acquire);
			return api && view != 0 && api->IsValid(view);
		}

		void QueueForceHideFocusMenu() noexcept
		{
			// PrismaUI uses a native menu overlay ("PrismaUI_FocusMenu") for cursor/input capture.
			// In some cases it can remain open even after Hide/Unfocus/Destroy, leaving the cursor visible and the game paused.
			// Force-hide it on the main thread as a safety net.
			if (auto* tasks = SKSE::GetTaskInterface(); tasks) {
				tasks->AddTask([]() {
					if (auto* queue = RE::UIMessageQueue::GetSingleton(); queue) {
						queue->AddMessage(RE::BSFixedString("PrismaUI_FocusMenu"), RE::UI_MESSAGE_TYPE::kForceHide, nullptr);
					}
					SKSE::log::info("Close: queued force-hide PrismaUI_FocusMenu");
				});
			}
		}

		void QueueHideSkyrimCursor() noexcept
		{
			if (auto* tasks = SKSE::GetTaskInterface(); tasks) {
				tasks->AddTask([]() {
					if (auto* cursor = RE::MenuCursor::GetSingleton(); cursor) {
						cursor->SetCursorVisibility(false);
					}
					SKSE::log::info("Close: queued MenuCursor hide");
				});
			}
		}

		void ResetViewStateOnUIThread() noexcept
		{
			g_openRequested.store(false, std::memory_order_relaxed);
			g_viewHidden.store(true, std::memory_order_relaxed);
			g_viewFocused.store(false, std::memory_order_relaxed);
			g_focusDelayArmed.store(false, std::memory_order_relaxed);
			g_focusAttemptCount.store(0, std::memory_order_relaxed);

			g_domReady.store(false, std::memory_order_release);
			g_view.store(0, std::memory_order_release);
		}

		void QueueCloseRetry(PrismaView view, bool destroyOnClose, std::uint32_t attempt) noexcept;

		void BeginCloseOnUIThread(PrismaView view, bool destroyOnClose) noexcept
		{
			auto* api = g_prismaAPI.load(std::memory_order_acquire);
			if (!api || view == 0 || !api->IsValid(view)) {
				if (destroyOnClose) {
					ResetViewStateOnUIThread();
				}
				return;
			}

			// First: request focus release. (Hide should auto-unfocus, but it is not always immediate.)
			api->Unfocus(view);
			api->Hide(view);

			QueueForceHideFocusMenu();
			QueueHideSkyrimCursor();

			QueueCloseRetry(view, destroyOnClose, 1);
		}

		void QueueCloseRetry(PrismaView view, bool destroyOnClose, std::uint32_t attempt) noexcept
		{
			constexpr std::uint32_t kMaxAttempts = 20;

			if (attempt > kMaxAttempts) {
				SKSE::log::error("Close: focus did not clear after {} attempts; leaving view {}", kMaxAttempts, view);
				QueueForceHideFocusMenu();
				QueueHideSkyrimCursor();
				return;
			}

			std::thread([view, destroyOnClose, attempt]() {
				std::this_thread::sleep_for(std::chrono::milliseconds(50));

				if (auto* tasks = SKSE::GetTaskInterface(); tasks) {
					tasks->AddUITask([view, destroyOnClose, attempt]() {
						// If re-open was requested while we were closing, abort the close pipeline.
						if (g_openRequested.load(std::memory_order_relaxed)) {
							SKSE::log::info("Close: aborted (re-open requested)");
							return;
						}

						auto* api = g_prismaAPI.load(std::memory_order_acquire);
						const auto activeView = g_view.load(std::memory_order_acquire);
						if (!api || view == 0 || activeView == 0 || activeView != view || !api->IsValid(view)) {
							return;
						}

						if (api->HasFocus(view)) {
							SKSE::log::warn("Close: still focused (attempt {}); retrying Unfocus/Hide", attempt);
							api->Unfocus(view);
							api->Hide(view);
							QueueForceHideFocusMenu();
							QueueHideSkyrimCursor();
							QueueCloseRetry(view, destroyOnClose, attempt + 1);
							return;
						}

						// Focus cleared: now it is safe to destroy (if configured).
						if (destroyOnClose) {
							api->Destroy(view);
							SKSE::log::info("PrismaView destroyed: {}", view);
							ResetViewStateOnUIThread();
						}
					});
				}
			}).detach();
		}

		void CallJS(const char* fn, const json& payload) noexcept
		{
			if (!fn || fn[0] == '\0') {
				return;
			}
			if (!g_domReady.load(std::memory_order_acquire)) {
				return;
			}
			auto* api = g_prismaAPI.load(std::memory_order_acquire);
			const auto view = g_view.load(std::memory_order_acquire);
			if (!api || view == 0 || !api->IsValid(view)) {
				return;
			}
			const auto s = payload.dump();
			api->InteropCall(view, fn, s.c_str());
		}

		void QueueFocusAndState() noexcept;
		void QueueOpenIfRequested() noexcept;
		[[nodiscard]] bool FocusFromSettings() noexcept;

		void QueueDelayedFocusAndState(std::uint32_t delayMs) noexcept
		{
			bool expected = false;
			if (!g_focusDelayArmed.compare_exchange_strong(expected, true, std::memory_order_relaxed)) {
				return;
			}

			std::thread([delayMs]() {
				std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));

				if (auto* tasks = SKSE::GetTaskInterface(); tasks) {
					tasks->AddUITask([]() {
						g_focusDelayArmed.store(false, std::memory_order_relaxed);

						const auto attempted = FocusFromSettings();
						if (!attempted) {
							return;
						}
						SendStateToUI();
					});
				} else {
					g_focusDelayArmed.store(false, std::memory_order_relaxed);
				}
			}).detach();
		}

		[[nodiscard]] bool FocusFromSettings() noexcept
		{
			if (!IsReady() || !g_domReady.load(std::memory_order_acquire)) {
				return false;
			}

			// Only focus if we still consider the UI "open".
			if (!g_openRequested.load(std::memory_order_relaxed)) {
				return false;
			}

			auto* api = g_prismaAPI.load(std::memory_order_acquire);
			const auto view = g_view.load(std::memory_order_acquire);
			if (!api || view == 0 || !api->IsValid(view)) {
				return false;
			}

			// Ensure the view is visible (Show is idempotent).
			api->Show(view);
			g_viewHidden.store(false, std::memory_order_relaxed);

			const auto settings = GetSettings();
			SKSE::log::info(
				"Focusing PrismaView (pauseGame={}, disableFocusMenu={})",
				settings.uiPauseGame,
				settings.uiDisableFocusMenu);

			const auto ok = api->Focus(view, settings.uiPauseGame, settings.uiDisableFocusMenu);
			if (!ok) {
				const auto attempt = g_focusAttemptCount.fetch_add(1, std::memory_order_relaxed) + 1;
				SKSE::log::warn("Focus() failed (attempt {})", attempt);
				if (attempt < 5) {
					QueueDelayedFocusAndState(150);
				}
			} else {
				g_focusAttemptCount.store(0, std::memory_order_relaxed);
			}
			g_viewFocused.store(ok, std::memory_order_relaxed);
			SKSE::log::info("Focus() -> {}", ok);
			return true;
		}

		void QueueFocusAndState() noexcept
		{
			if (auto* tasks = SKSE::GetTaskInterface(); tasks) {
				// Give PrismaUI time to apply Show() before focusing (avoids races where IsHidden
				// stays true for a short window even though the view renders).
				QueueDelayedFocusAndState(50);
			}
		}

		void QueueOpenIfRequested() noexcept
		{
			if (auto* tasks = SKSE::GetTaskInterface(); tasks) {
				// Open on the next task tick to avoid re-entrancy around DOM ready / view creation.
				tasks->AddUITask([]() {
					if (!IsReady() || !g_domReady.load(std::memory_order_acquire)) {
						return;
					}
					if (!g_openRequested.load(std::memory_order_relaxed)) {
						return;
					}

					auto* api = g_prismaAPI.load(std::memory_order_acquire);
					const auto view = g_view.load(std::memory_order_acquire);
					if (!api || view == 0 || !api->IsValid(view)) {
						return;
					}

					SKSE::log::info("Open: begin");
					SKSE::log::info("Open: Show() begin");
					api->Show(view);
					g_viewHidden.store(false, std::memory_order_relaxed);
					SKSE::log::info("Open: Show() end");

					QueueFocusAndState();
				});
			}
		}

		void Toast(std::string_view level, std::string message) noexcept
		{
			CallJS("copng_toast", json{ { "level", level }, { "message", std::move(message) } });
		}

		void OnDomReady(PrismaView view) noexcept
		{
			auto* api = g_prismaAPI.load(std::memory_order_acquire);
			const auto activeView = g_view.load(std::memory_order_acquire);
			if (!api || view == 0 || activeView == 0 || view != activeView) {
				SKSE::log::info("DOM ready ignored for stale view {} (active {})", view, activeView);
				return;
			}
			if (!api->IsValid(view)) {
				SKSE::log::info("DOM ready ignored for invalid view {}", view);
				return;
			}

			g_domReady.store(true, std::memory_order_release);

			// Route initialization through the task queue to avoid any uncertainty
			// about which thread PrismaUI invokes the DOM-ready callback on.
			if (auto* tasks = SKSE::GetTaskInterface(); tasks) {
					tasks->AddUITask([view]() {
						auto* api = g_prismaAPI.load(std::memory_order_acquire);
						const auto activeView = g_view.load(std::memory_order_acquire);
						if (!api || activeView == 0 || activeView != view || !api->IsValid(activeView)) {
							return;
						}

						SendStateToUI();
						CallJS("copng_setSettings", SettingsToJson(GetSettings()));

						if (g_openRequested.load(std::memory_order_relaxed) && IsReady()) {
							SKSE::log::info("DOM ready: opening view (queued)");
							QueueOpenIfRequested();
						} else if (IsReady()) {
							if (api && activeView != 0 && api->IsValid(activeView)) {
								api->Hide(activeView);
							}
							g_viewHidden.store(true, std::memory_order_relaxed);
							g_viewFocused.store(false, std::memory_order_relaxed);
							SendStateToUI();
						}
				});
				return;
			}

			SendStateToUI();
			CallJS("copng_setSettings", SettingsToJson(GetSettings()));
		}

		void OnJsLog(const char* argument) noexcept
		{
			SKSE::log::info("[JS] {}", argument ? argument : "");
		}

		void OnJsRequestState(const char* /*argument*/) noexcept
		{
			SendStateToUI();
		}

		void OnJsRequestToggle(const char* /*argument*/) noexcept
		{
			SKSE::log::info("JS requested toggle UI");
			ToggleUI();
		}

			void QueueSendInventory(InventoryRequest req) noexcept
			{
				if (auto* tasks = SKSE::GetTaskInterface(); tasks) {
					// Inventory access must run on the main thread.
					tasks->AddTask([req]() {
						const auto tBuild0 = std::chrono::steady_clock::now();
						const auto offset = static_cast<std::size_t>(req.page) * static_cast<std::size_t>(req.pageSize);
						auto page = Registration::BuildQuickRegisterList(offset, req.pageSize);
						const auto tBuild1 = std::chrono::steady_clock::now();
						const auto buildMs =
							static_cast<std::uint32_t>(
								std::chrono::duration_cast<std::chrono::milliseconds>(tBuild1 - tBuild0).count());
						SKSE::log::info(
							"Inventory: built page {} ({} items, hasMore={}, total {}) in {}ms",
							req.page,
							page.items.size(),
							page.hasMore,
							page.total,
							buildMs);
						if (auto* uiTasks = SKSE::GetTaskInterface(); uiTasks) {
							// UI/JS interop must run on the UI thread.
							const auto itemCount = page.items.size();
							const auto totalCount = page.total;
							uiTasks->AddUITask([page = std::move(page), req, itemCount, totalCount, buildMs]() mutable {
								const auto tJson0 = std::chrono::steady_clock::now();
								json payload;
								payload["page"] = req.page;
								payload["pageSize"] = req.pageSize;
								payload["total"] = totalCount;
								payload["hasMore"] = page.hasMore;

								json arr = json::array();
								for (const auto& it : page.items) {
									arr.push_back({
										{ "formId", it.formId },
										{ "regKey", it.regKey },
										{ "name", it.name },
										{ "group", it.group },
										{ "groupName", Registration::GetDiscoveryGroupName(it.group) },
										{ "totalCount", it.totalCount },
										{ "safeCount", it.safeCount },
									});
								}
								payload["items"] = std::move(arr);
								const auto tJson1 = std::chrono::steady_clock::now();
								const auto jsonMs =
									static_cast<std::uint32_t>(
										std::chrono::duration_cast<std::chrono::milliseconds>(tJson1 - tJson0).count());

								const auto tSend0 = std::chrono::steady_clock::now();
								CallJS("copng_setInventory", payload);
								const auto tSend1 = std::chrono::steady_clock::now();
								const auto sendMs =
									static_cast<std::uint32_t>(
										std::chrono::duration_cast<std::chrono::milliseconds>(tSend1 - tSend0).count());

								SKSE::log::info(
									"Inventory: json {}ms + send {}ms for {} items (build {}ms)",
									jsonMs,
									sendMs,
									itemCount,
									buildMs);
							});
						}
					});
					return;
				}

			// Fallback (no task interface): synchronous
				const auto offset = static_cast<std::size_t>(req.page) * static_cast<std::size_t>(req.pageSize);
				auto page = Registration::BuildQuickRegisterList(offset, req.pageSize);
				json payload;
				payload["page"] = req.page;
				payload["pageSize"] = req.pageSize;
				payload["total"] = page.total;
				payload["hasMore"] = page.hasMore;
				json arr = json::array();
				for (const auto& it : page.items) {
					arr.push_back({
						{ "formId", it.formId },
						{ "regKey", it.regKey },
						{ "name", it.name },
						{ "group", it.group },
						{ "groupName", Registration::GetDiscoveryGroupName(it.group) },
						{ "totalCount", it.totalCount },
						{ "safeCount", it.safeCount },
					});
				}
				payload["items"] = std::move(arr);
				CallJS("copng_setInventory", payload);
			}

		void QueueSendRegistered() noexcept
		{
			if (auto* tasks = SKSE::GetTaskInterface(); tasks) {
				tasks->AddTask([]() {
					auto items = Registration::BuildRegisteredList();
					if (auto* uiTasks = SKSE::GetTaskInterface(); uiTasks) {
						uiTasks->AddUITask([items = std::move(items)]() mutable {
							json arr = json::array();
							for (const auto& it : items) {
								arr.push_back({
									{ "formId", it.formId },
									{ "name", it.name },
									{ "group", it.group },
									{ "groupName", Registration::GetDiscoveryGroupName(it.group) },
								});
							}
							CallJS("copng_setRegistered", arr);
						});
					}
				});
				return;
			}

			auto items = Registration::BuildRegisteredList();
			json arr = json::array();
			for (const auto& it : items) {
				arr.push_back({
					{ "formId", it.formId },
					{ "name", it.name },
					{ "group", it.group },
					{ "groupName", Registration::GetDiscoveryGroupName(it.group) },
				});
			}
			CallJS("copng_setRegistered", arr);
		}

		[[nodiscard]] std::string FormatReward(float total, std::string_view fmt) noexcept
		{
			char buf[64];
			if (fmt == "pct") {
				std::snprintf(buf, sizeof(buf), "%+.2f%%", static_cast<double>(total));
				return buf;
			}
			if (fmt == "multPct") {
				std::snprintf(buf, sizeof(buf), "%+.2f%%", static_cast<double>(total * 100.0f));
				return buf;
			}
			std::snprintf(buf, sizeof(buf), "%+.2f", static_cast<double>(total));
			return buf;
		}

		void QueueSendRewards() noexcept
		{
			if (auto* tasks = SKSE::GetTaskInterface(); tasks) {
				tasks->AddTask([]() {
					std::size_t registeredCount = 0;
					std::vector<std::pair<RE::ActorValue, float>> totals;
					{
						auto& state = GetState();
						std::scoped_lock lock(state.mutex);
						registeredCount = state.registeredItems.size();
						totals.reserve(state.rewardTotals.size());
						for (const auto& [av, total] : state.rewardTotals) {
							if (total != 0.0f) {
								totals.emplace_back(av, total);
							}
						}
					}

					const auto settings = GetSettings();
					const auto every = settings.rewardEvery > 0 ? settings.rewardEvery : 0;
					const auto rolls = (every > 0) ? static_cast<std::int32_t>(registeredCount / static_cast<std::size_t>(every)) : 0;

					if (auto* uiTasks = SKSE::GetTaskInterface(); uiTasks) {
						uiTasks->AddUITask([registeredCount, totals = std::move(totals), rolls, every, mult = settings.rewardMultiplier]() mutable {
							json j;
							j["registeredCount"] = registeredCount;
							j["rewardEvery"] = every;
							j["rewardMultiplier"] = mult;
							j["rolls"] = rolls;

							json arr = json::array();
							for (const auto& [av, total] : totals) {
								std::string labelKey;
								std::string fallback;
								std::string fmt = "raw";

								switch (av) {
								case RE::ActorValue::kHealth:
									labelKey = "av.health"; fallback = "Health"; break;
								case RE::ActorValue::kMagicka:
									labelKey = "av.magicka"; fallback = "Magicka"; break;
								case RE::ActorValue::kStamina:
									labelKey = "av.stamina"; fallback = "Stamina"; break;
								case RE::ActorValue::kHealRate:
									labelKey = "av.healRate"; fallback = "Health regen"; break;
								case RE::ActorValue::kMagickaRate:
									labelKey = "av.magickaRate"; fallback = "Magicka regen"; break;
								case RE::ActorValue::kStaminaRate:
									labelKey = "av.staminaRate"; fallback = "Stamina regen"; break;
								case RE::ActorValue::kAttackDamageMult:
									labelKey = "av.attackDamageMult"; fallback = "Attack damage (physical)"; fmt = "multPct"; break;
								case RE::ActorValue::kCriticalChance:
									labelKey = "av.critChance"; fallback = "Critical chance"; fmt = "pct"; break;
								case RE::ActorValue::kUnarmedDamage:
									labelKey = "av.unarmedDamage"; fallback = "Unarmed damage"; break;
								case RE::ActorValue::kReflectDamage:
									labelKey = "av.reflectDamage"; fallback = "Reflect damage"; fmt = "pct"; break;
								case RE::ActorValue::kResistMagic:
									labelKey = "av.magicResist"; fallback = "Magic resist"; fmt = "pct"; break;
								case RE::ActorValue::kResistFire:
									labelKey = "av.fireResist"; fallback = "Fire resist"; fmt = "pct"; break;
								case RE::ActorValue::kResistFrost:
									labelKey = "av.frostResist"; fallback = "Frost resist"; fmt = "pct"; break;
								case RE::ActorValue::kResistShock:
									labelKey = "av.electricResist"; fallback = "Shock resist"; fmt = "pct"; break;
								case RE::ActorValue::kPoisonResist:
									labelKey = "av.poisonResist"; fallback = "Poison resist"; fmt = "pct"; break;
								case RE::ActorValue::kResistDisease:
									labelKey = "av.diseaseResist"; fallback = "Disease resist"; fmt = "pct"; break;
								case RE::ActorValue::kDamageResist:
									labelKey = "av.damageResist"; fallback = "Armor rating"; break;
								case RE::ActorValue::kCarryWeight:
									labelKey = "av.carryWeight"; fallback = "Carry weight"; break;
								case RE::ActorValue::kSpeedMult:
									labelKey = "av.speedMult"; fallback = "Move speed"; fmt = "multPct"; break;
								case RE::ActorValue::kSmithingModifier:
									labelKey = "av.smithingMod"; fallback = "Smithing effectiveness"; break;
								case RE::ActorValue::kAlchemyModifier:
									labelKey = "av.alchemyMod"; fallback = "Alchemy effectiveness"; break;
								case RE::ActorValue::kEnchantingModifier:
									labelKey = "av.enchantingMod"; fallback = "Enchanting effectiveness"; break;
								case RE::ActorValue::kSpeechcraftModifier:
									labelKey = "av.speechcraftMod"; fallback = "Barter effectiveness"; break;
								case RE::ActorValue::kSneakingModifier:
									labelKey = "av.sneakMod"; fallback = "Sneak effectiveness"; break;
								case RE::ActorValue::kLockpickingModifier:
									labelKey = "av.lockpickingMod"; fallback = "Lockpicking effectiveness"; break;
								case RE::ActorValue::kPickpocketModifier:
									labelKey = "av.pickpocketMod"; fallback = "Pickpocket effectiveness"; break;
								case RE::ActorValue::kAlterationModifier:
									labelKey = "av.alterationMod"; fallback = "Alteration cost reduction"; break;
								case RE::ActorValue::kConjurationModifier:
									labelKey = "av.conjurationMod"; fallback = "Conjuration cost reduction"; break;
								case RE::ActorValue::kDestructionModifier:
									labelKey = "av.destructionMod"; fallback = "Destruction cost reduction"; break;
								case RE::ActorValue::kIllusionModifier:
									labelKey = "av.illusionMod"; fallback = "Illusion cost reduction"; break;
								case RE::ActorValue::kRestorationModifier:
									labelKey = "av.restorationMod"; fallback = "Restoration cost reduction"; break;
								case RE::ActorValue::kAbsorbChance:
									labelKey = "av.absorbChance"; fallback = "Spell absorption chance"; fmt = "pct"; break;
								case RE::ActorValue::kShoutRecoveryMult:
									labelKey = "av.shoutRecoveryMult"; fallback = "Shout cooldown"; fmt = "multPct"; break;
								case RE::ActorValue::kOneHanded:
									labelKey = "skill.oneHanded"; fallback = "Skill (One-Handed)"; break;
								case RE::ActorValue::kTwoHanded:
									labelKey = "skill.twoHanded"; fallback = "Skill (Two-Handed)"; break;
								case RE::ActorValue::kArchery:
									labelKey = "skill.marksman"; fallback = "Skill (Archery)"; break;
								case RE::ActorValue::kHeavyArmor:
									labelKey = "skill.heavyArmor"; fallback = "Skill (Heavy Armor)"; break;
								case RE::ActorValue::kLightArmor:
									labelKey = "skill.lightArmor"; fallback = "Skill (Light Armor)"; break;
								case RE::ActorValue::kBlock:
									labelKey = "skill.block"; fallback = "Skill (Block)"; break;
								case RE::ActorValue::kAlchemy:
									labelKey = "skill.alchemy"; fallback = "Skill (Alchemy)"; break;
								case RE::ActorValue::kLockpicking:
									labelKey = "skill.lockpicking"; fallback = "Skill (Lockpicking)"; break;
								case RE::ActorValue::kPickpocket:
									labelKey = "skill.pickpocket"; fallback = "Skill (Pickpocket)"; break;
								default:
									labelKey = ""; fallback = "Unknown"; break;
								}

								const auto label = labelKey.empty() ? fallback : L10n::T(labelKey, fallback);
								arr.push_back({
									{ "av", static_cast<std::uint32_t>(av) },
									{ "label", label },
									{ "total", total },
									{ "format", fmt },
									{ "display", FormatReward(total, fmt) },
								});
							}

							j["totals"] = arr;
							CallJS("copng_setRewards", j);
						});
					}
				});
				return;
			}

			// Fallback (no task interface): minimal synchronous snapshot
			std::size_t registeredCount = 0;
			std::vector<std::pair<RE::ActorValue, float>> totals;
			{
				auto& state = GetState();
				std::scoped_lock lock(state.mutex);
				registeredCount = state.registeredItems.size();
				totals.reserve(state.rewardTotals.size());
				for (const auto& [av, total] : state.rewardTotals) {
					if (total != 0.0f) {
						totals.emplace_back(av, total);
					}
				}
			}

			const auto settings = GetSettings();
			const auto every = settings.rewardEvery > 0 ? settings.rewardEvery : 0;
			const auto rolls = (every > 0) ? static_cast<std::int32_t>(registeredCount / static_cast<std::size_t>(every)) : 0;

			json j;
			j["registeredCount"] = registeredCount;
			j["rewardEvery"] = every;
			j["rewardMultiplier"] = settings.rewardMultiplier;
			j["rolls"] = rolls;

			json arr = json::array();
			for (const auto& [av, total] : totals) {
				arr.push_back({
					{ "av", static_cast<std::uint32_t>(av) },
					{ "label", std::to_string(static_cast<std::uint32_t>(av)) },
					{ "total", total },
					{ "format", "raw" },
					{ "display", FormatReward(total, "raw") },
				});
			}
			j["totals"] = arr;
			CallJS("copng_setRewards", j);
		}

			void OnJsRequestInventory(const char* argument) noexcept
			{
				SKSE::log::info("JS requested inventory");
				QueueSendInventory(ParseInventoryRequest(argument));
			}

		void OnJsRequestRegistered(const char* /*argument*/) noexcept
		{
			SKSE::log::info("JS requested registered list");
			QueueSendRegistered();
		}

		void OnJsRequestRewards(const char* /*argument*/) noexcept
		{
			SKSE::log::info("JS requested rewards");
			QueueSendRewards();
		}

		void OnJsGetSettings(const char* /*argument*/) noexcept
		{
			SKSE::log::info("JS requested settings");
			CallJS("copng_setSettings", SettingsToJson(GetSettings()));
		}

		void OnJsSetSettings(const char* argument) noexcept
		{
			json payload;
			try {
				payload = json::parse(argument ? argument : "{}");
			} catch (...) {
				Toast("error", "Invalid JSON");
				return;
			}

			const auto current = GetSettings();
			const auto next = SettingsFromJson(payload, current);
			if (SettingsEquivalent(current, next)) {
				Toast("info", "No changes");
				CallJS("copng_setSettings", SettingsToJson(current));
				SendStateToUI();
				return;
			}

			const bool reloadL10n = next.languageOverride != current.languageOverride;
			SKSE::log::info(
				"JS requested save settings (lang {} -> {}, toggleKey=0x{:02X}, pauseGame={}, disableFocusMenu={}, destroyOnClose={}, inputScale={:.2f})",
				current.languageOverride,
				next.languageOverride,
				next.toggleKeyCode,
				next.uiPauseGame,
				next.uiDisableFocusMenu,
				next.uiDestroyOnClose,
				next.uiInputScale);

			// Apply in-memory immediately; persist on a background worker to avoid stutter.
			SetSettings(next);
			CallJS("copng_setSettings", SettingsToJson(GetSettings()));
			SendStateToUI();

			QueueSaveSettingsToDisk(SettingsSaveJob{ next, reloadL10n });
		}

		void OnJsRefundRewards(const char* /*argument*/) noexcept
		{
			if (auto* tasks = SKSE::GetTaskInterface(); tasks) {
				tasks->AddTask([]() {
					const auto cleared = Rewards::RefundRewards();
					if (auto* uiTasks = SKSE::GetTaskInterface(); uiTasks) {
						uiTasks->AddUITask([cleared]() {
							Toast("info", "Rewards refunded (" + std::to_string(cleared) + ")");
							SendStateToUI();
							QueueSendRewards();
						});
					}
				});
				return;
			}

			const auto cleared = Rewards::RefundRewards();
			Toast("info", "Rewards refunded (" + std::to_string(cleared) + ")");
			SendStateToUI();
			QueueSendRewards();
		}

		void OnJsRegisterItem(const char* argument) noexcept
		{
			json payload;
			try {
				payload = json::parse(argument ? argument : "{}");
			} catch (...) {
				Toast("error", "Invalid JSON");
				return;
			}

			auto formIdIt = payload.find("formId");
			if (formIdIt == payload.end()) {
				Toast("error", "Missing formId");
				return;
			}

			auto formIdOpt = ParseFormIDFromJson(*formIdIt);
			if (!formIdOpt) {
				Toast("error", "Invalid formId");
				return;
			}

			const auto formId = *formIdOpt;
			SKSE::log::info("JS requested register item (formId: 0x{:08X})", static_cast<std::uint32_t>(formId));

			if (auto* tasks = SKSE::GetTaskInterface(); tasks) {
				tasks->AddTask([formId]() {
					const auto res = Registration::TryRegisterItem(formId);
					if (auto* uiTasks = SKSE::GetTaskInterface(); uiTasks) {
							uiTasks->AddUITask([res]() {
								SKSE::log::info(
									"Register item result: success={} regKey=0x{:08X} group={} total={} msg='{}'",
									res.success,
									static_cast<std::uint32_t>(res.regKey),
									res.group,
									res.totalRegistered,
									res.message);
								Toast(res.success ? "info" : "error", res.message);
								SendStateToUI();
								QueueSendInventory(InventoryRequest{});
								QueueSendRegistered();
								QueueSendRewards();
							});
					}
				});
				return;
			}

				const auto res = Registration::TryRegisterItem(formId);
				SKSE::log::info(
					"Register item result (sync): success={} regKey=0x{:08X} group={} total={} msg='{}'",
					res.success,
					static_cast<std::uint32_t>(res.regKey),
					res.group,
					res.totalRegistered,
					res.message);
				Toast(res.success ? "info" : "error", res.message);
				SendStateToUI();
				QueueSendInventory(InventoryRequest{});
				QueueSendRegistered();
				QueueSendRewards();
			}

		[[nodiscard]] bool EnsureCreatedOnUIThread() noexcept
		{
			auto* api = g_prismaAPI.load(std::memory_order_acquire);
			if (!api) {
				SKSE::log::warn("Prisma UI API unavailable. Is PrismaUI installed?");
				return false;
			}

			const auto existingView = g_view.load(std::memory_order_acquire);
			if (existingView != 0 && api->IsValid(existingView)) {
				return false;
			}

			SKSE::log::info("Creating PrismaView ('{}')", kPrismaUIViewPath);

			g_domReady.store(false, std::memory_order_release);
			const auto newView = api->CreateView(kPrismaUIViewPath, OnDomReady);
			g_view.store(newView, std::memory_order_release);
			if (!newView || !api->IsValid(newView)) {
				SKSE::log::error("Failed to create PrismaView for '{}'", kPrismaUIViewPath);
				g_view.store(0, std::memory_order_release);
				return false;
			}

			SKSE::log::info("PrismaView created: {}", newView);
			// Render on top of most overlays by default.
			// (Higher values render later / in the foreground.)
			api->SetOrder(newView, 10000);
			SKSE::log::info("PrismaView order: {}", api->GetOrder(newView));

			api->RegisterJSListener(newView, "copng_log", OnJsLog);
			api->RegisterJSListener(newView, "copng_requestState", OnJsRequestState);
			api->RegisterJSListener(newView, "copng_requestToggle", OnJsRequestToggle);
			api->RegisterJSListener(newView, "copng_requestInventory", OnJsRequestInventory);
			api->RegisterJSListener(newView, "copng_requestRegistered", OnJsRequestRegistered);
			api->RegisterJSListener(newView, "copng_requestRewards", OnJsRequestRewards);
			api->RegisterJSListener(newView, "copng_getSettings", OnJsGetSettings);
			api->RegisterJSListener(newView, "copng_saveSettings", OnJsSetSettings);
			api->RegisterJSListener(newView, "copng_refundRewards", OnJsRefundRewards);
			api->RegisterJSListener(newView, "copng_registerItem", OnJsRegisterItem);

			// Start hidden; open after DOM ready if requested.
			api->Hide(newView);
			g_viewHidden.store(true, std::memory_order_relaxed);
			g_viewFocused.store(false, std::memory_order_relaxed);

			return true;
		}
	}

		void OnPostLoad() noexcept
		{
			auto* api = PRISMA_UI_API::RequestPluginAPI(PRISMA_UI_API::InterfaceVersion::V1);
		g_prismaAPI.store(static_cast<PRISMA_UI_API::IVPrismaUI1*>(api), std::memory_order_release);

		if (g_prismaAPI.load(std::memory_order_acquire)) {
				SKSE::log::info("Prisma UI API acquired");
		} else {
				SKSE::log::warn("Failed to acquire Prisma UI API (PrismaUI.dll not loaded?)");
		}
	}

	void OnDataLoaded() noexcept
	{
		if (auto* tasks = SKSE::GetTaskInterface(); tasks) {
			tasks->AddUITask([]() { (void)EnsureCreatedOnUIThread(); });
		} else {
			(void)EnsureCreatedOnUIThread();
		}
	}

	void OnPreLoadGame() noexcept
	{
		g_toggleAllowed.store(false, std::memory_order_relaxed);
		g_toggleAllowedAtMs.store(0, std::memory_order_relaxed);
	}

	void OnGameLoaded() noexcept
	{
		g_toggleAllowed.store(true, std::memory_order_relaxed);
		// Grace period after load/new-game to avoid focus/input conflicts while the game is settling.
		g_toggleAllowedAtMs.store(NowMs() + 2000, std::memory_order_relaxed);
	}

	void ToggleUI() noexcept
	{
		if (auto* tasks = SKSE::GetTaskInterface(); tasks) {
			tasks->AddUITask([]() {
				if (!g_toggleAllowed.load(std::memory_order_relaxed)) {
					SKSE::log::info("ToggleUI: ignored (not allowed yet)");
					return;
				}

				const auto now = NowMs();
				const auto allowAt = g_toggleAllowedAtMs.load(std::memory_order_relaxed);
				if (allowAt > 0 && now < allowAt) {
					SKSE::log::info("ToggleUI: ignored (post-load grace period)");
					return;
				}

				if (auto* ui = RE::UI::GetSingleton(); ui) {
					if (ui->IsMenuOpen(RE::MainMenu::MENU_NAME) || ui->IsMenuOpen(RE::LoadingMenu::MENU_NAME)) {
						SKSE::log::info("ToggleUI: ignored (application menu open)");
						return;
					}
				}

				if (auto* main = RE::Main::GetSingleton(); !main || !main->gameActive) {
					SKSE::log::info("ToggleUI: ignored (game not active)");
					return;
				}

				const auto last = g_lastToggleMs.load(std::memory_order_relaxed);
				if (last > 0 && now < last + kToggleDebounceMs) {
					SKSE::log::info("ToggleUI: debounced");
					return;
				}
				g_lastToggleMs.store(now, std::memory_order_relaxed);

				SKSE::log::info("ToggleUI: begin");
				const auto readyBefore = IsReady();
				if (!readyBefore) {
					// Arm open request BEFORE CreateView to avoid a race where OnDomReady fires
					// before ToggleUI sets g_openRequested.
					g_openRequested.store(true, std::memory_order_relaxed);
					g_focusAttemptCount.store(0, std::memory_order_relaxed);
				}

				const auto createdNow = EnsureCreatedOnUIThread();
				if (!IsReady()) {
					SKSE::log::warn("ToggleUI: view not ready");
					g_openRequested.store(false, std::memory_order_relaxed);
					return;
				}

				// If we created the view as part of this toggle, OnDomReady will handle opening.
				if (!readyBefore && createdNow) {
					SKSE::log::info("ToggleUI: view created; will open after DOM ready");
					SendStateToUI();
					SKSE::log::info("ToggleUI: end");
					return;
				}

				if (g_viewHidden.load(std::memory_order_relaxed)) {
					SKSE::log::info("ToggleUI: open (queued)");
					g_openRequested.store(true, std::memory_order_relaxed);
					g_focusAttemptCount.store(0, std::memory_order_relaxed);
					g_viewHidden.store(false, std::memory_order_relaxed);
					QueueOpenIfRequested();
				} else {
					const auto settings = GetSettings();
					if (settings.uiDestroyOnClose) {
						SKSE::log::info("ToggleUI: destroy");
						g_openRequested.store(false, std::memory_order_relaxed);
						g_viewHidden.store(true, std::memory_order_relaxed);
						g_viewFocused.store(false, std::memory_order_relaxed);

						const auto view = g_view.load(std::memory_order_acquire);
						BeginCloseOnUIThread(view, true);
					} else {
						SKSE::log::info("ToggleUI: hide");
						g_openRequested.store(false, std::memory_order_relaxed);
						g_viewHidden.store(true, std::memory_order_relaxed);
						g_viewFocused.store(false, std::memory_order_relaxed);
						const auto view = g_view.load(std::memory_order_acquire);
						BeginCloseOnUIThread(view, false);
					}
				}

				SendStateToUI();
				SKSE::log::info("ToggleUI: end");
			});
			return;
		}

		SKSE::log::warn("ToggleUI: task interface unavailable; refusing to run UI operations synchronously");
	}

	void SendStateToUI() noexcept
	{
		if (!IsReady() || !g_domReady.load(std::memory_order_acquire)) {
			return;
		}

		std::size_t registeredCount = 0;
		std::size_t rewardCount = 0;
		{
			auto& state = GetState();
			std::scoped_lock lock(state.mutex);
			registeredCount = state.registeredItems.size();
			rewardCount = state.rewardTotals.size();
		}

		// Avoid querying PrismaUI view state on-demand here; Focus/Show timing can
		// conflict with internal view task processing and appear as a stall.
		const bool focused = g_viewFocused.load(std::memory_order_relaxed);
		const bool hidden = g_viewHidden.load(std::memory_order_relaxed);

		const auto settings = GetSettings();

		json j;
		j["ui"] = {
			{ "ready", g_domReady.load(std::memory_order_relaxed) },
			{ "focused", focused },
			{ "hidden", hidden },
		};
		j["registeredCount"] = registeredCount;
		j["rewardCount"] = rewardCount;
		j["language"] = L10n::ActiveLanguage();
		j["toggleKeyCode"] = settings.toggleKeyCode;

		CallJS("copng_setState", j);
	}
}
