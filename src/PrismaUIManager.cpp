#include "CodexOfPowerNG/PrismaUIManager.h"

#include "CodexOfPowerNG/TaskScheduler.h"
#include "CodexOfPowerNG/Util.h"
#include "PrismaUIInternal.h"

#include <SKSE/SKSE.h>
#include <SKSE/Logger.h>

extern "C" __declspec(dllimport) void* __stdcall GetModuleHandleA(const char* lpModuleName);
extern "C" __declspec(dllimport) void* __stdcall GetProcAddress(void* hModule, const char* lpProcName);

#include "PrismaUI_API.h"

#include <cstdint>

namespace CodexOfPowerNG::PrismaUIManager
{
	namespace
	{
		constexpr std::uint64_t kToggleDebounceMs = 350;
	}

	void OnPostLoad() noexcept
	{
		Internal::SetShuttingDown(false);

		auto* api = PRISMA_UI_API::RequestPluginAPI(PRISMA_UI_API::InterfaceVersion::V1);
		Internal::SetPrismaAPI(static_cast<PRISMA_UI_API::IVPrismaUI1*>(api));

		if (Internal::GetPrismaAPI()) {
			SKSE::log::info("Prisma UI API acquired");
		} else {
			SKSE::log::warn("Failed to acquire Prisma UI API (PrismaUI.dll not loaded?)");
		}
	}

	void Shutdown() noexcept
	{
		Internal::SetShuttingDown(true);
		Internal::ShutdownSettingsWorker();
		Internal::JoinLifecycleWorkers();
	}

	void OnDataLoaded() noexcept
	{
		if (!QueueUITask([]() { (void)Internal::EnsureCreatedOnUIThread(); })) {
			(void)Internal::EnsureCreatedOnUIThread();
		}
	}

	void OnPreLoadGame() noexcept
	{
		Shutdown();
		Internal::ForceCleanupForLoadBoundary();
		Internal::ResetTogglePolicyForPreLoad();
	}

	void OnGameLoaded() noexcept
	{
		Internal::SetShuttingDown(false);
		Internal::ArmTogglePolicyAfterGameLoaded(NowMs(), 2000);
	}

	void ToggleUI() noexcept
	{
		if (QueueUITask([]() {
			const auto now = NowMs();
			if (!Internal::CanToggleNow(now, kToggleDebounceMs)) {
				return;
			}
			Internal::ExecuteToggleTransition();
		})) {
			return;
		}

		SKSE::log::warn("ToggleUI: task interface unavailable; refusing to run UI operations synchronously");
	}

	void SendStateToUI() noexcept
	{
		if (!Internal::IsReady() || !Internal::IsDomReady()) {
			return;
		}

		Internal::SendJS("copng_setState", Internal::BuildRuntimeStatePayload());
	}
}
