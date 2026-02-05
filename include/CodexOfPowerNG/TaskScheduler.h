#pragma once

#include <functional>

namespace CodexOfPowerNG
{
	using ScheduledTask = std::function<void()>;

	class ITaskScheduler
	{
	public:
		virtual ~ITaskScheduler() = default;

		virtual bool AddMainTask(ScheduledTask task) noexcept = 0;
		virtual bool AddUITask(ScheduledTask task) noexcept = 0;
	};

	[[nodiscard]] ITaskScheduler& GetTaskScheduler() noexcept;

	// Test-only hook to replace scheduler behavior.
	void SetTaskSchedulerForTesting(ITaskScheduler* scheduler) noexcept;

	[[nodiscard]] bool QueueMainTask(ScheduledTask task) noexcept;
	[[nodiscard]] bool QueueUITask(ScheduledTask task) noexcept;
}
