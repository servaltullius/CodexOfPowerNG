#pragma once

#include "CodexOfPowerNG/Config.h"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <string>
#include <string_view>

namespace PRISMA_UI_API
{
	class IVPrismaUI1;
}

namespace CodexOfPowerNG::PrismaUIManager::Internal
{
	using json = nlohmann::json;
	using PrismaView = std::uint64_t;

	void SetPrismaAPI(PRISMA_UI_API::IVPrismaUI1* api) noexcept;
	[[nodiscard]] PRISMA_UI_API::IVPrismaUI1* GetPrismaAPI() noexcept;
	void SetShuttingDown(bool value) noexcept;
	void JoinLifecycleWorkers() noexcept;

	[[nodiscard]] bool IsReady() noexcept;
	[[nodiscard]] bool IsDomReady() noexcept;
	[[nodiscard]] bool IsOpenRequested() noexcept;
	[[nodiscard]] bool IsViewHidden() noexcept;
	[[nodiscard]] bool IsViewFocused() noexcept;
	[[nodiscard]] PrismaView CurrentView() noexcept;

	void SetOpenRequested(bool value) noexcept;
	void SetViewHidden(bool value) noexcept;
	void SetViewFocused(bool value) noexcept;
	void ResetFocusAttempts() noexcept;
	void QueueDelayedFocusAndState(std::uint32_t delayMs) noexcept;
	void ResetTogglePolicyForPreLoad() noexcept;
	void ArmTogglePolicyAfterGameLoaded(std::uint64_t nowMs, std::uint64_t graceMs) noexcept;
	[[nodiscard]] bool CanToggleNow(std::uint64_t nowMs, std::uint64_t debounceMs) noexcept;
	void ExecuteToggleTransition() noexcept;

	void QueueOpenIfRequested() noexcept;
	void BeginCloseOnUIThread(PrismaView view, bool destroyOnClose) noexcept;
	[[nodiscard]] bool EnsureCreatedOnUIThread() noexcept;

	[[nodiscard]] json BuildSettingsPayload(const Settings& settings);
	[[nodiscard]] json BuildRuntimeStatePayload();
	void               SendJS(const char* fn, const json& payload) noexcept;
	void               ShowToast(std::string_view level, std::string message) noexcept;
	void               QueueSettingsSave(Settings settings, bool reloadL10n) noexcept;
	void               ShutdownSettingsWorker() noexcept;
	void               HandleSaveSettingsRequest(const char* argument) noexcept;

	void RegisterCoreJSListeners(PRISMA_UI_API::IVPrismaUI1* api, std::uint64_t view) noexcept;
	void RegisterSettingsJSListener(PRISMA_UI_API::IVPrismaUI1* api, std::uint64_t view) noexcept;
}
