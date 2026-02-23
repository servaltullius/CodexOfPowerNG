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
#include <exception>
#include <future>
#include <memory>
#include <thread>

namespace CodexOfPowerNG::PrismaUIManager::Internal
{
	namespace
	{
		// Workers whose slot was replaced before they finished.
		// Joined during shutdown by JoinLifecycleWorkers().
		// Protected by State::workerMutex (callers always hold it).
		std::vector<std::thread> g_staleWorkerThreads;

		void ReplaceWorkerThread(std::thread& slot, std::thread next) noexcept
		{
			if (slot.joinable()) {
				g_staleWorkerThreads.push_back(std::move(slot));
			}
			slot = std::move(next);
		}

		void ClearFocusDelayArmedIfCurrent(std::uint64_t generation) noexcept
		{
			if (generation == State::focusDelayGeneration.load(std::memory_order_acquire)) {
				State::focusDelayArmed.store(false, std::memory_order_relaxed);
			}
		}

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
			constexpr auto          kCloseTaskTimeout = std::chrono::milliseconds(500);
			const auto              generation = State::closeRetryGeneration.fetch_add(1, std::memory_order_acq_rel) + 1;

			{
				std::scoped_lock lock(State::workerMutex);
				ReplaceWorkerThread(
					State::closeRetryThread,
					std::thread([view, destroyOnClose, kMaxAttempts, kCloseTaskTimeout, generation]() {
						for (std::uint32_t attempt = 1; attempt <= kMaxAttempts; ++attempt) {
							std::this_thread::sleep_for(std::chrono::milliseconds(50));

							if (State::shuttingDown.load(std::memory_order_relaxed)) {
								return;
							}
							if (generation != State::closeRetryGeneration.load(std::memory_order_acquire)) {
								return;
							}

							enum class CloseResult
							{
								kDone,
								kRetry
							};
							auto promise = std::make_shared<std::promise<CloseResult>>();
							auto future = promise->get_future();

							if (!QueueUITask([promise, view, destroyOnClose, attempt, generation]() {
									if (generation != State::closeRetryGeneration.load(std::memory_order_acquire)) {
										promise->set_value(CloseResult::kDone);
										return;
									}

									if (State::openRequested.load(std::memory_order_relaxed)) {
										SKSE::log::info("Close: aborted (re-open requested)");
										promise->set_value(CloseResult::kDone);
										return;
									}

									auto* api = GetPrismaAPI();
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
								if (attempt == 1 || (attempt % 5) == 0 || attempt == kMaxAttempts) {
									SKSE::log::warn(
										"Close: failed to queue UI task (attempt {}/{})",
										attempt,
										kMaxAttempts);
								}
								QueueForceHideFocusMenu();
								QueueHideSkyrimCursor();
								continue;
							}

							const auto status = future.wait_for(kCloseTaskTimeout);
							if (status == std::future_status::timeout) {
								if (attempt == 1 || (attempt % 5) == 0 || attempt == kMaxAttempts) {
									SKSE::log::warn(
										"Close: UI task timeout after {}ms (attempt {}/{})",
										kCloseTaskTimeout.count(),
										attempt,
										kMaxAttempts);
								}
								QueueForceHideFocusMenu();
								QueueHideSkyrimCursor();
								continue;
							}

							try {
								const auto result = future.get();
								if (result == CloseResult::kDone) {
									return;
								}
							} catch (const std::future_error& e) {
								SKSE::log::error("Close: future error while waiting for UI task: {}", e.what());
								QueueForceHideFocusMenu();
								QueueHideSkyrimCursor();
								continue;
							} catch (const std::exception& e) {
								SKSE::log::error("Close: unexpected exception while waiting for UI task: {}", e.what());
								QueueForceHideFocusMenu();
								QueueHideSkyrimCursor();
								continue;
							}
							// kRetry -> loop continues
						}

						if (generation != State::closeRetryGeneration.load(std::memory_order_acquire)) {
							return;
						}

						SKSE::log::error("Close: focus did not clear after {} attempts; leaving view {}", kMaxAttempts, view);
						QueueForceHideFocusMenu();
						QueueHideSkyrimCursor();
					}));
			}
		}
	}

	void SetShuttingDown(bool value) noexcept
	{
		State::shuttingDown.store(value, std::memory_order_relaxed);
		if (value) {
			State::closeRetryGeneration.fetch_add(1, std::memory_order_acq_rel);
			State::focusDelayGeneration.fetch_add(1, std::memory_order_acq_rel);
			State::focusDelayArmed.store(false, std::memory_order_relaxed);
		}
	}

	void JoinLifecycleWorkers() noexcept
	{
		std::scoped_lock lock(State::workerMutex);
		State::JoinIfJoinable(State::closeRetryThread);
		State::JoinIfJoinable(State::focusDelayThread);
		for (auto& t : g_staleWorkerThreads) {
			State::JoinIfJoinable(t);
		}
		g_staleWorkerThreads.clear();
	}

	void ForceCleanupForLoadBoundary() noexcept
	{
		// PreLoadGame can happen while PrismaUI has focus/cursor capture active.
		// Make a best-effort attempt to release focus and hide any residual overlays/cursor.
		State::openRequested.store(false, std::memory_order_relaxed);

		const auto view = State::view.load(std::memory_order_acquire);

		QueueForceHideFocusMenu();
		QueueHideSkyrimCursor();

		if (!QueueUITask([view]() {
				auto* api = GetPrismaAPI();
				if (api && view != 0 && api->IsValid(view)) {
					api->Unfocus(view);
					api->Hide(view);
					api->Destroy(view);
					SKSE::log::info("PreLoadGame: forced PrismaView destroy: {}", view);
				}

				ResetViewStateOnUIThread();
			})) {
			SKSE::log::warn("PreLoadGame: failed to queue UI cleanup task");
			ResetViewStateOnUIThread();
		}
	}

	void QueueDelayedFocusAndState(std::uint32_t delayMs) noexcept
	{
		bool expected = false;
		if (!State::focusDelayArmed.compare_exchange_strong(expected, true, std::memory_order_relaxed)) {
			return;
		}
		const auto generation = State::focusDelayGeneration.fetch_add(1, std::memory_order_acq_rel) + 1;

		{
			std::scoped_lock lock(State::workerMutex);
			ReplaceWorkerThread(
				State::focusDelayThread,
				std::thread([delayMs, generation]() {
					std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));

					if (State::shuttingDown.load(std::memory_order_relaxed)) {
						ClearFocusDelayArmedIfCurrent(generation);
						return;
					}
					if (generation != State::focusDelayGeneration.load(std::memory_order_acquire)) {
						ClearFocusDelayArmedIfCurrent(generation);
						return;
					}

					if (!QueueUITask([generation]() {
							ClearFocusDelayArmedIfCurrent(generation);

							if (generation != State::focusDelayGeneration.load(std::memory_order_acquire)) {
								return;
							}

							if (!IsReady() || !State::domReady.load(std::memory_order_acquire)) {
								return;
							}
							if (!State::openRequested.load(std::memory_order_relaxed)) {
								return;
							}

							auto* api = GetPrismaAPI();
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
						ClearFocusDelayArmedIfCurrent(generation);
					}
				}));
		}
	}

	void BeginCloseOnUIThread(PrismaView view, bool destroyOnClose) noexcept
	{
		auto* api = GetPrismaAPI();
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
