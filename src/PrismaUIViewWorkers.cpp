#include "PrismaUIInternal.h"
#include "PrismaUIViewState.h"

#include "CodexOfPowerNG/Config.h"
#include "CodexOfPowerNG/PrismaUIManager.h"
#include "CodexOfPowerNG/TaskScheduler.h"

#include <RE/Skyrim.h>

#include <SKSE/SKSE.h>

extern "C" __declspec(dllimport) void* __stdcall GetModuleHandleA(const char* lpModuleName);
extern "C" __declspec(dllimport) void* __stdcall GetProcAddress(void* hModule, const char* lpProcName);

#include "PrismaUI_API.h"

#include <RE/M/MenuCursor.h>
#include <RE/U/UIMessageQueue.h>

#include <SKSE/Logger.h>

#include <chrono>
#include <future>
#include <memory>
#include <thread>

namespace CodexOfPowerNG::PrismaUIManager::Internal
{
	namespace
	{
		void QueueForceHideFocusMenu() noexcept
		{
			// PrismaUI uses a native menu overlay ("PrismaUI_FocusMenu") for cursor/input capture.
			// In some cases it can remain open even after Hide/Unfocus/Destroy, leaving the cursor visible and the game paused.
			// Force-hide it on the main thread as a safety net.
			(void)QueueMainTask([]() {
				if (auto* queue = RE::UIMessageQueue::GetSingleton(); queue) {
					queue->AddMessage(RE::BSFixedString("PrismaUI_FocusMenu"), RE::UI_MESSAGE_TYPE::kForceHide, nullptr);
				}
				SKSE::log::info("Close: queued force-hide PrismaUI_FocusMenu");
			});
		}

		void QueueHideSkyrimCursor() noexcept
		{
			(void)QueueMainTask([]() {
				if (auto* cursor = RE::MenuCursor::GetSingleton(); cursor) {
					cursor->SetCursorVisibility(false);
				}
				SKSE::log::info("Close: queued MenuCursor hide");
			});
		}

		void ResetViewStateOnUIThread() noexcept
		{
			State::openRequested.store(false, std::memory_order_relaxed);
			State::viewHidden.store(true, std::memory_order_relaxed);
			State::viewFocused.store(false, std::memory_order_relaxed);
			State::focusDelayArmed.store(false, std::memory_order_relaxed);
			State::focusAttemptCount.store(0, std::memory_order_relaxed);

			State::domReady.store(false, std::memory_order_release);
			State::view.store(0, std::memory_order_release);
		}

		void QueueCloseRetry(PrismaView view, bool destroyOnClose) noexcept
		{
			constexpr std::uint32_t kMaxAttempts = 20;

			{
				std::scoped_lock lock(State::workerMutex);
				State::JoinIfJoinable(State::closeRetryThread);
				State::closeRetryThread = std::thread([view, destroyOnClose, kMaxAttempts]() {
					for (std::uint32_t attempt = 1; attempt <= kMaxAttempts; ++attempt) {
						std::this_thread::sleep_for(std::chrono::milliseconds(50));

						if (State::shuttingDown.load(std::memory_order_relaxed)) {
							return;
						}

						enum class CloseResult
						{
							kDone,
							kRetry
						};
						auto promise = std::make_shared<std::promise<CloseResult>>();
						auto future = promise->get_future();

						if (!QueueUITask([promise, view, destroyOnClose, attempt]() {
								if (State::openRequested.load(std::memory_order_relaxed)) {
									SKSE::log::info("Close: aborted (re-open requested)");
									promise->set_value(CloseResult::kDone);
									return;
								}

								auto* api = State::prismaAPI.load(std::memory_order_acquire);
								const auto activeView = State::view.load(std::memory_order_acquire);
								if (!api || view == 0 || activeView == 0 || activeView != view || !api->IsValid(view)) {
									promise->set_value(CloseResult::kDone);
									return;
								}

								if (api->HasFocus(view)) {
									SKSE::log::warn("Close: still focused (attempt {}); retrying Unfocus/Hide", attempt);
									api->Unfocus(view);
									api->Hide(view);
									QueueForceHideFocusMenu();
									QueueHideSkyrimCursor();
									promise->set_value(CloseResult::kRetry);
									return;
								}

								if (destroyOnClose) {
									api->Destroy(view);
									SKSE::log::info("PrismaView destroyed: {}", view);
									ResetViewStateOnUIThread();
								}
								promise->set_value(CloseResult::kDone);
							})) {
							return;
						}

						// Block until UI task completes (no spin-wait)
						const auto result = future.get();
						if (result == CloseResult::kDone) {
							return;
						}
						// kRetry -> loop continues
					}

					SKSE::log::error("Close: focus did not clear after {} attempts; leaving view {}", kMaxAttempts, view);
					QueueForceHideFocusMenu();
					QueueHideSkyrimCursor();
				});
			}
		}
	}

	void SetShuttingDown(bool value) noexcept
	{
		State::shuttingDown.store(value, std::memory_order_relaxed);
	}

	void JoinLifecycleWorkers() noexcept
	{
		std::scoped_lock lock(State::workerMutex);
		State::JoinIfJoinable(State::closeRetryThread);
		State::JoinIfJoinable(State::focusDelayThread);
	}

	void QueueDelayedFocusAndState(std::uint32_t delayMs) noexcept
	{
		bool expected = false;
		if (!State::focusDelayArmed.compare_exchange_strong(expected, true, std::memory_order_relaxed)) {
			return;
		}

		{
			std::scoped_lock lock(State::workerMutex);
			State::JoinIfJoinable(State::focusDelayThread);
			State::focusDelayThread = std::thread([delayMs]() {
				std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));

				if (State::shuttingDown.load(std::memory_order_relaxed)) {
					State::focusDelayArmed.store(false, std::memory_order_relaxed);
					return;
				}

				if (!QueueUITask([]() {
						State::focusDelayArmed.store(false, std::memory_order_relaxed);

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

						// Ensure the view is visible (Show is idempotent).
						api->Show(view);
						State::viewHidden.store(false, std::memory_order_relaxed);

						const auto settings = GetSettings();
						SKSE::log::info(
							"Focusing PrismaView (pauseGame={}, disableFocusMenu={})",
							settings.uiPauseGame,
							settings.uiDisableFocusMenu);

						const auto ok = api->Focus(view, settings.uiPauseGame, settings.uiDisableFocusMenu);
						if (!ok) {
							const auto attempt = State::focusAttemptCount.fetch_add(1, std::memory_order_relaxed) + 1;
							SKSE::log::warn("Focus() failed (attempt {})", attempt);
							if (attempt < 5) {
								QueueDelayedFocusAndState(150);
							}
						} else {
							State::focusAttemptCount.store(0, std::memory_order_relaxed);
						}
						State::viewFocused.store(ok, std::memory_order_relaxed);
						SKSE::log::info("Focus() -> {}", ok);
						SendStateToUI();
					})) {
					State::focusDelayArmed.store(false, std::memory_order_relaxed);
				}
			});
		}
	}

	void BeginCloseOnUIThread(PrismaView view, bool destroyOnClose) noexcept
	{
		auto* api = State::prismaAPI.load(std::memory_order_acquire);
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

		QueueCloseRetry(view, destroyOnClose);
	}
}
