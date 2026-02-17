#include "PrismaUIInternal.h"

#include "CodexOfPowerNG/Config.h"
#include "CodexOfPowerNG/PrismaUIManager.h"

#include <RE/Skyrim.h>

#include <SKSE/Logger.h>

namespace CodexOfPowerNG::PrismaUIManager::Internal
{
	namespace
	{
		void QueueOpenTransition() noexcept
		{
			SKSE::log::info("ToggleUI: open (queued)");
			SetOpenRequested(true);
			ResetFocusAttempts();
			SetViewHidden(false);
			QueueOpenIfRequested();
		}

		void QueueCloseTransition() noexcept
		{
			const auto settings = GetSettings();
			SetOpenRequested(false);
			SetViewHidden(true);
			SetViewFocused(false);

			const auto view = CurrentView();
			if (settings.uiDestroyOnClose) {
				SKSE::log::info("ToggleUI: destroy");
				BeginCloseOnUIThread(view, true);
			} else {
				SKSE::log::info("ToggleUI: hide");
				BeginCloseOnUIThread(view, false);
			}
		}
	}

	void ExecuteToggleTransition() noexcept
	{
		SKSE::log::info("ToggleUI: begin");
		const auto readyBefore = IsReady();
		if (!readyBefore) {
			// Arm open request BEFORE CreateView to avoid a race where OnDomReady fires
			// before ToggleUI sets open-request state.
			SetOpenRequested(true);
			ResetFocusAttempts();
		}

		const auto createdNow = EnsureCreatedOnUIThread();
		if (!IsReady()) {
			SKSE::log::warn("ToggleUI: view not ready");
			SetOpenRequested(false);
			return;
		}

		// If we created the view as part of this toggle, DOM-ready will handle opening.
		if (!readyBefore && createdNow) {
			SKSE::log::info("ToggleUI: view created; will open after DOM ready");
			SendStateToUI();
			SKSE::log::info("ToggleUI: end");
			return;
		}

		if (IsViewHidden()) {
			QueueOpenTransition();
		} else {
			QueueCloseTransition();
		}

		SendStateToUI();
		SKSE::log::info("ToggleUI: end");
	}
}
