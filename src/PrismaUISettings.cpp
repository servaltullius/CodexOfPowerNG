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
#include <exception>
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
			Settings persistedSettings{};
			bool     reloadL10n{ false };
		};

		std::mutex                    g_settingsWorkerMutex;
		std::thread                   g_settingsSaveThread;
		std::mutex                    g_settingsSaveMutex;
		std::optional<SettingsSaveJob> g_pendingSettingsSave;
		std::atomic_bool              g_settingsSaveWorkerRunning{ false };
		std::atomic_bool              g_settingsSaveWorkerStopping{ false };
		std::mutex                    g_lastPersistedSettingsMutex;
		std::optional<Settings>        g_lastPersistedSettings;

		[[nodiscard]] Settings SnapshotLastPersistedSettings(const Settings& fallback) noexcept
		{
			std::scoped_lock lock(g_lastPersistedSettingsMutex);
			if (!g_lastPersistedSettings) {
				g_lastPersistedSettings = ClampSettings(fallback);
			}
			return *g_lastPersistedSettings;
		}

		void RecordPersistedSettings(const Settings& settings) noexcept
		{
			std::scoped_lock lock(g_lastPersistedSettingsMutex);
			g_lastPersistedSettings = ClampSettings(settings);
		}

		void JoinIfJoinable(std::thread& t) noexcept
		{
			if (t.joinable()) {
				t.join();
			}
		}

		void RevertSettingsAfterSaveFailure(const SettingsSaveJob& job) noexcept
		{
			SetSettings(job.persistedSettings);
			Registration::InvalidateQuickRegisterCache();
		}

		[[nodiscard]] bool PersistSettingsToDisk(const Settings& settings) noexcept
		{
			try {
				return SaveSettingsSnapshotToDisk(settings);
			} catch (const std::exception& e) {
				SKSE::log::error("Settings save worker: SaveSettingsSnapshotToDisk threw: {}", e.what());
			} catch (...) {
				SKSE::log::error("Settings save worker: SaveSettingsSnapshotToDisk threw (unknown exception)");
			}
			return false;
		}

		[[nodiscard]] bool ReloadL10nIfNeeded(const SettingsSaveJob& job) noexcept
		{
			if (!job.reloadL10n) {
				return false;
			}

			// Reload localization. Avoid calling RE::GetINISetting from a background thread when in auto mode.
			const auto explicitLang = (job.settings.languageOverride == "en" || job.settings.languageOverride == "ko");
			if (explicitLang) {
				L10n::Load();
				return false;
			}
			return true;
		}

		void QueueSettingsFeedback(bool ok, bool needsMainThreadL10n) noexcept
		{
			if (!QueueUITask([ok, needsMainThreadL10n]() {
					if (ok) {
						if (needsMainThreadL10n) {
							L10n::Load();
						}
						ShowToast("info", "Settings saved");
					} else {
						ShowToast("error", "Failed to save settings (reverted)");
					}
					SendJS("copng_setSettings", BuildSettingsPayload(GetSettings()));
					SendStateToUI();
				})) {
				SKSE::log::warn("Settings save worker: failed to queue UI feedback task (ok={})", ok);
			}
		}

		[[nodiscard]] bool TryTakePendingSettingsSave(SettingsSaveJob& outJob) noexcept
		{
			{
				std::scoped_lock lock(g_settingsSaveMutex);
				if (!g_pendingSettingsSave) {
					g_settingsSaveWorkerRunning.store(false, std::memory_order_relaxed);
					return false;
				}
				outJob = std::move(*g_pendingSettingsSave);
				g_pendingSettingsSave.reset();
			}
			return true;
		}

		void RunSettingsSaveWorkerLoop() noexcept
		{
			try {
				for (;;) {
					if (g_settingsSaveWorkerStopping.load(std::memory_order_relaxed)) {
						g_settingsSaveWorkerRunning.store(false, std::memory_order_relaxed);
						return;
					}

					SettingsSaveJob next{};
					if (!TryTakePendingSettingsSave(next)) {
						return;
					}

					const bool ok = PersistSettingsToDisk(next.settings);
					if (!ok) {
						RevertSettingsAfterSaveFailure(next);
					} else {
						RecordPersistedSettings(next.settings);
					}

					const bool needsMainThreadL10n = ok ? ReloadL10nIfNeeded(next) : false;
					QueueSettingsFeedback(ok, needsMainThreadL10n);
				}
			} catch (const std::exception& e) {
				SKSE::log::error("Settings save worker crashed: {}", e.what());
			} catch (...) {
				SKSE::log::error("Settings save worker crashed (unknown exception)");
			}

			g_settingsSaveWorkerRunning.store(false, std::memory_order_relaxed);
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
				g_settingsSaveThread = std::thread(RunSettingsSaveWorkerLoop);
			}
		}
	}

	void QueueSettingsSave(Settings settings, Settings persistedSettings, bool reloadL10n) noexcept
	{
		QueueSaveSettingsToDisk(SettingsSaveJob{ std::move(settings), std::move(persistedSettings), reloadL10n });
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
		const auto persistedSettings = SnapshotLastPersistedSettings(current);
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

		QueueSettingsSave(next, persistedSettings, reloadL10n);
	}
}
