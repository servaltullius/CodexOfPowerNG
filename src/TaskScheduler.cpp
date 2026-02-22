#include "CodexOfPowerNG/TaskScheduler.h"

#include <SKSE/SKSE.h>
#include <SKSE/Logger.h>

#include <atomic>
#include <exception>
#include <utility>

namespace CodexOfPowerNG
{
	namespace
	{
		class SKSETaskScheduler final : public ITaskScheduler
		{
		public:
			bool AddMainTask(ScheduledTask task) noexcept override
			{
				if (!task) {
					return false;
				}

				if (auto* tasks = SKSE::GetTaskInterface(); tasks) {
					tasks->AddTask([task = std::move(task)]() mutable {
						try {
							task();
						} catch (const std::exception& e) {
							SKSE::log::error("Unhandled exception in queued main task: {}", e.what());
						} catch (...) {
							SKSE::log::error("Unhandled exception in queued main task");
						}
					});
					return true;
				}
				return false;
			}

			bool AddUITask(ScheduledTask task) noexcept override
			{
				if (!task) {
					return false;
				}

				if (auto* tasks = SKSE::GetTaskInterface(); tasks) {
					tasks->AddUITask([task = std::move(task)]() mutable {
						try {
							task();
						} catch (const std::exception& e) {
							SKSE::log::error("Unhandled exception in queued UI task: {}", e.what());
						} catch (...) {
							SKSE::log::error("Unhandled exception in queued UI task");
						}
					});
					return true;
				}
				return false;
			}
		};

		SKSETaskScheduler g_defaultScheduler;
		std::atomic<ITaskScheduler*> g_overrideScheduler{ nullptr };
	}

	ITaskScheduler& GetTaskScheduler() noexcept
	{
		if (auto* overrideScheduler = g_overrideScheduler.load(std::memory_order_acquire)) {
			return *overrideScheduler;
		}
		return g_defaultScheduler;
	}

	void SetTaskSchedulerForTesting(ITaskScheduler* scheduler) noexcept
	{
		g_overrideScheduler.store(scheduler, std::memory_order_release);
	}

	bool QueueMainTask(ScheduledTask task) noexcept
	{
		return GetTaskScheduler().AddMainTask(std::move(task));
	}

	bool QueueUITask(ScheduledTask task) noexcept
	{
		return GetTaskScheduler().AddUITask(std::move(task));
	}
}
