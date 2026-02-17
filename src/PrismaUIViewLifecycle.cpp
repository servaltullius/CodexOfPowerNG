#include "PrismaUIInternal.h"
#include "PrismaUIViewState.h"

#include "CodexOfPowerNG/Config.h"
#include "CodexOfPowerNG/Constants.h"
#include "CodexOfPowerNG/PrismaUIManager.h"
#include "CodexOfPowerNG/TaskScheduler.h"

#include <SKSE/SKSE.h>
#include <SKSE/Logger.h>

extern "C" __declspec(dllimport) void* __stdcall GetModuleHandleA(const char* lpModuleName);
extern "C" __declspec(dllimport) void* __stdcall GetProcAddress(void* hModule, const char* lpProcName);

#include "PrismaUI_API.h"

#include <utility>

namespace CodexOfPowerNG::PrismaUIManager::Internal
{
	namespace
	{
		void CallJS(const char* fn, const json& payload) noexcept
		{
			if (!fn || fn[0] == '\0') {
				return;
			}
			if (!State::domReady.load(std::memory_order_acquire)) {
				return;
			}
			auto* api = State::prismaAPI.load(std::memory_order_acquire);
			const auto view = State::view.load(std::memory_order_acquire);
			if (!api || view == 0 || !api->IsValid(view)) {
				return;
			}
			const auto s = payload.dump();
			api->InteropCall(view, fn, s.c_str());
		}

		void Toast(std::string_view level, std::string message) noexcept
		{
			CallJS("copng_toast", json{ { "level", level }, { "message", std::move(message) } });
		}

		void OnDomReady(PrismaView view) noexcept
		{
			auto* api = State::prismaAPI.load(std::memory_order_acquire);
			const auto activeView = State::view.load(std::memory_order_acquire);
			if (!api || view == 0 || activeView == 0 || view != activeView) {
				SKSE::log::info("DOM ready ignored for stale view {} (active {})", view, activeView);
				return;
			}
			if (!api->IsValid(view)) {
				SKSE::log::info("DOM ready ignored for invalid view {}", view);
				return;
			}

			State::domReady.store(true, std::memory_order_release);

			// Route initialization through the task queue to avoid any uncertainty
			// about which thread PrismaUI invokes the DOM-ready callback on.
			if (QueueUITask([view]() {
					auto* api = State::prismaAPI.load(std::memory_order_acquire);
					const auto activeView = State::view.load(std::memory_order_acquire);
					if (!api || activeView == 0 || activeView != view || !api->IsValid(activeView)) {
						return;
					}

					SendStateToUI();
					SendJS("copng_setSettings", BuildSettingsPayload(GetSettings()));

					if (State::openRequested.load(std::memory_order_relaxed) && IsReady()) {
						SKSE::log::info("DOM ready: opening view (queued)");
						QueueOpenIfRequested();
					} else if (IsReady()) {
						if (api && activeView != 0 && api->IsValid(activeView)) {
							api->Hide(activeView);
						}
						State::viewHidden.store(true, std::memory_order_relaxed);
						State::viewFocused.store(false, std::memory_order_relaxed);
						SendStateToUI();
					}
				})) {
				return;
			}

			SendStateToUI();
			SendJS("copng_setSettings", BuildSettingsPayload(GetSettings()));
		}
	}

	void SetPrismaAPI(PRISMA_UI_API::IVPrismaUI1* api) noexcept
	{
		State::prismaAPI.store(api, std::memory_order_release);
	}

	PRISMA_UI_API::IVPrismaUI1* GetPrismaAPI() noexcept
	{
		return State::prismaAPI.load(std::memory_order_acquire);
	}

	bool IsReady() noexcept
	{
		auto* api = State::prismaAPI.load(std::memory_order_acquire);
		const auto view = State::view.load(std::memory_order_acquire);
		return api && view != 0 && api->IsValid(view);
	}

	bool IsDomReady() noexcept
	{
		return State::domReady.load(std::memory_order_acquire);
	}

	bool IsOpenRequested() noexcept
	{
		return State::openRequested.load(std::memory_order_relaxed);
	}

	bool IsViewHidden() noexcept
	{
		return State::viewHidden.load(std::memory_order_relaxed);
	}

	bool IsViewFocused() noexcept
	{
		return State::viewFocused.load(std::memory_order_relaxed);
	}

	PrismaView CurrentView() noexcept
	{
		return State::view.load(std::memory_order_acquire);
	}

	void SetOpenRequested(bool value) noexcept
	{
		State::openRequested.store(value, std::memory_order_relaxed);
	}

	void SetViewHidden(bool value) noexcept
	{
		State::viewHidden.store(value, std::memory_order_relaxed);
	}

	void SetViewFocused(bool value) noexcept
	{
		State::viewFocused.store(value, std::memory_order_relaxed);
	}

	void ResetFocusAttempts() noexcept
	{
		State::focusAttemptCount.store(0, std::memory_order_relaxed);
	}

	void QueueOpenIfRequested() noexcept
	{
		// Open on the next task tick to avoid re-entrancy around DOM ready / view creation.
		(void)QueueUITask([]() {
			if (!IsReady() || !State::domReady.load(std::memory_order_acquire)) {
				return;
			}
			if (!State::openRequested.load(std::memory_order_relaxed)) {
				return;
			}

			auto* api = State::prismaAPI.load(std::memory_order_acquire);
			const auto view = State::view.load(std::memory_order_acquire);
			if (!api || view == 0 || !api->IsValid(view)) {
				return;
			}

			SKSE::log::info("Open: begin");
			SKSE::log::info("Open: Show() begin");
			api->Show(view);
			State::viewHidden.store(false, std::memory_order_relaxed);
			SKSE::log::info("Open: Show() end");

			// Give PrismaUI time to apply Show() before focusing (avoids races where IsHidden
			// stays true for a short window even though the view renders).
			QueueDelayedFocusAndState(50);
		});
	}

	bool EnsureCreatedOnUIThread() noexcept
	{
		auto* api = State::prismaAPI.load(std::memory_order_acquire);
		if (!api) {
			SKSE::log::warn("Prisma UI API unavailable. Is PrismaUI installed?");
			return false;
		}

		const auto existingView = State::view.load(std::memory_order_acquire);
		if (existingView != 0 && api->IsValid(existingView)) {
			return false;
		}

		SKSE::log::info("Creating PrismaView ('{}')", kPrismaUIViewPath);

		State::domReady.store(false, std::memory_order_release);
		const auto newView = api->CreateView(kPrismaUIViewPath, OnDomReady);
		State::view.store(newView, std::memory_order_release);
		if (!newView || !api->IsValid(newView)) {
			SKSE::log::error("Failed to create PrismaView for '{}'", kPrismaUIViewPath);
			State::view.store(0, std::memory_order_release);
			return false;
		}

		SKSE::log::info("PrismaView created: {}", newView);
		// Render on top of most overlays by default.
		// (Higher values render later / in the foreground.)
		api->SetOrder(newView, 10000);
		SKSE::log::info("PrismaView order: {}", api->GetOrder(newView));

		RegisterCoreJSListeners(api, newView);
		RegisterSettingsJSListener(api, newView);

		// Start hidden; open after DOM ready if requested.
		api->Hide(newView);
		State::viewHidden.store(true, std::memory_order_relaxed);
		State::viewFocused.store(false, std::memory_order_relaxed);

		return true;
	}

	void RegisterSettingsJSListener(PRISMA_UI_API::IVPrismaUI1* api, std::uint64_t view) noexcept
	{
		if (!api || view == 0 || !api->IsValid(view)) {
			return;
		}
		api->RegisterJSListener(view, "copng_saveSettings", HandleSaveSettingsRequest);
	}

	void SendJS(const char* fn, const json& payload) noexcept
	{
		CallJS(fn, payload);
	}

	void ShowToast(std::string_view level, std::string message) noexcept
	{
		Toast(level, std::move(message));
	}
}
