#include "PrismaUIInternal.h"
#include "PrismaUISettingsInternal.h"

#include "CodexOfPowerNG/Config.h"
#include "CodexOfPowerNG/L10n.h"
#include "CodexOfPowerNG/PrismaUIManager.h"
#include "CodexOfPowerNG/Registration.h"
#include "CodexOfPowerNG/TaskScheduler.h"

#include <RE/Skyrim.h>

#include <SKSE/Logger.h>

#include <atomic>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>

namespace CodexOfPowerNG::PrismaUIManager::Internal
{
	namespace
	{
		struct SettingsSaveJob
		{
			Settings settings{};
			bool     reloadL10n{ false };
		};

		std::mutex                    g_settingsWorkerMutex;
		std::thread                   g_settingsSaveThread;
		std::mutex                    g_settingsSaveMutex;
		std::optional<SettingsSaveJob> g_pendingSettingsSave;
		std::atomic_bool              g_settingsSaveWorkerRunning{ false };
		std::atomic_bool              g_settingsSaveWorkerStopping{ false };

		void JoinIfJoinable(std::thread& t) noexcept
		{
			if (t.joinable()) {
				t.join();
			}
		}

		void QueueSaveSettingsToDisk(SettingsSaveJob job) noexcept
		{
			g_settingsSaveWorkerStopping.store(false, std::memory_order_relaxed);

			{
				std::scoped_lock lock(g_settingsSaveMutex);
				g_pendingSettingsSave = std::move(job);
			}

			bool expected = false;
			if (!g_settingsSaveWorkerRunning.compare_exchange_strong(expected, true, std::memory_order_relaxed)) {
				return;
			}

			{
				std::scoped_lock lock(g_settingsWorkerMutex);
				JoinIfJoinable(g_settingsSaveThread);
				g_settingsSaveThread = std::thread([]() {
					for (;;) {
						if (g_settingsSaveWorkerStopping.load(std::memory_order_relaxed)) {
							g_settingsSaveWorkerRunning.store(false, std::memory_order_relaxed);
							return;
						}

						SettingsSaveJob next{};
						{
							std::scoped_lock lock(g_settingsSaveMutex);
							if (!g_pendingSettingsSave) {
								g_settingsSaveWorkerRunning.store(false, std::memory_order_relaxed);
								return;
							}
							next = std::move(*g_pendingSettingsSave);
							g_pendingSettingsSave.reset();
						}

						const auto ok = SaveSettingsToDisk(next.settings);
						bool needsMainThreadL10n = false;
						if (ok && next.reloadL10n) {
							// Reload localization. Avoid calling RE::GetINISetting from a background thread when in auto mode.
							const auto explicitLang = (next.settings.languageOverride == "en" || next.settings.languageOverride == "ko");
							if (explicitLang) {
								L10n::Load();
							} else {
								needsMainThreadL10n = true;
							}
						}

						(void)QueueUITask([ok, needsMainThreadL10n]() {
							if (ok) {
								if (needsMainThreadL10n) {
									L10n::Load();
								}
								ShowToast("info", "Settings saved");
							} else {
								ShowToast("error", "Failed to save settings");
							}
							SendJS("copng_setSettings", BuildSettingsPayload(GetSettings()));
							SendStateToUI();
						});
					}
				});
			}
		}
	}

	void QueueSettingsSave(Settings settings, bool reloadL10n) noexcept
	{
		QueueSaveSettingsToDisk(SettingsSaveJob{ std::move(settings), reloadL10n });
	}

	void ShutdownSettingsWorker() noexcept
	{
		g_settingsSaveWorkerStopping.store(true, std::memory_order_relaxed);
		{
			std::scoped_lock lock(g_settingsSaveMutex);
			g_pendingSettingsSave.reset();
		}
		{
			std::scoped_lock lock(g_settingsWorkerMutex);
			JoinIfJoinable(g_settingsSaveThread);
		}
		g_settingsSaveWorkerRunning.store(false, std::memory_order_relaxed);
	}

	void HandleSaveSettingsRequest(const char* argument) noexcept
	{
		const auto current = GetSettings();
		const auto nextOpt = ParseSettingsPayload(argument, current);
		if (!nextOpt) {
			ShowToast("error", "Invalid JSON");
			return;
		}

		const auto next = *nextOpt;
		if (SettingsEquivalent(current, next)) {
			ShowToast("info", "No changes");
			SendJS("copng_setSettings", BuildSettingsPayload(current));
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
		Registration::InvalidateQuickRegisterCache();
		SendJS("copng_setSettings", BuildSettingsPayload(GetSettings()));
		SendStateToUI();

		QueueSettingsSave(next, reloadL10n);
	}
}
