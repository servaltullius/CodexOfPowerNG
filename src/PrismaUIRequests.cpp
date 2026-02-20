#include "PrismaUIInternal.h"
#include "PrismaUIRequestOps.h"

#include "CodexOfPowerNG/PrismaUIManager.h"

#include <SKSE/Logger.h>
#include <SKSE/SKSE.h>

extern "C" __declspec(dllimport) void* __stdcall GetModuleHandleA(const char* lpModuleName);
extern "C" __declspec(dllimport) void* __stdcall GetProcAddress(void* hModule, const char* lpProcName);

#include "PrismaUI_API.h"

namespace CodexOfPowerNG::PrismaUIManager::Internal
{
	namespace
	{
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

		void OnJsRequestUndoList(const char* /*argument*/) noexcept
		{
			SKSE::log::info("JS requested undo list");
			QueueSendUndoList();
		}

		void OnJsGetSettings(const char* /*argument*/) noexcept
		{
			SKSE::log::info("JS requested settings");
			SendJS("copng_setSettings", BuildSettingsPayload(GetSettings()));
		}

		void OnJsRefundRewards(const char* /*argument*/) noexcept
		{
			HandleRefundRewardsRequest();
		}

		void OnJsRegisterItem(const char* argument) noexcept
		{
			HandleRegisterItemRequest(argument);
		}

		void OnJsUndoRegisterItem(const char* argument) noexcept
		{
			HandleUndoRegisterRequest(argument);
		}
	}

	void RegisterCoreJSListeners(PRISMA_UI_API::IVPrismaUI1* api, std::uint64_t view) noexcept
	{
		if (!api || view == 0 || !api->IsValid(view)) {
			return;
		}

		api->RegisterJSListener(view, "copng_log", OnJsLog);
		api->RegisterJSListener(view, "copng_requestState", OnJsRequestState);
		api->RegisterJSListener(view, "copng_requestToggle", OnJsRequestToggle);
		api->RegisterJSListener(view, "copng_requestInventory", OnJsRequestInventory);
		api->RegisterJSListener(view, "copng_requestRegistered", OnJsRequestRegistered);
		api->RegisterJSListener(view, "copng_requestRewards", OnJsRequestRewards);
		api->RegisterJSListener(view, "copng_requestUndoList", OnJsRequestUndoList);
		api->RegisterJSListener(view, "copng_getSettings", OnJsGetSettings);
		api->RegisterJSListener(view, "copng_refundRewards", OnJsRefundRewards);
		api->RegisterJSListener(view, "copng_registerItem", OnJsRegisterItem);
		api->RegisterJSListener(view, "copng_undoRegisterItem", OnJsUndoRegisterItem);
	}
}
