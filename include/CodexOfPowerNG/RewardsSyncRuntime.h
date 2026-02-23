#pragma once

#include "CodexOfPowerNG/RewardsSyncPolicy.h"

#include <atomic>
#include <cstdint>

namespace CodexOfPowerNG::Rewards::SyncRuntime
{
	namespace Detail
	{
		struct SyncSlotState
		{
			std::atomic_bool          scheduled{ false };
			std::atomic_bool          rerunRequested{ false };
			std::atomic<std::uint64_t> scheduledSinceMs{ 0 };
		};

		struct RuntimeState
		{
			SyncSlotState             rewardSync{};
			SyncSlotState             carryWeightQuickResync{};
			std::atomic<std::uint64_t> generation{ 1 };
		};

		inline RuntimeState g_runtimeState{};

		inline void ClearSyncSlot(SyncSlotState& slot) noexcept
		{
			slot.scheduled.store(false, std::memory_order_release);
			slot.rerunRequested.store(false, std::memory_order_release);
			slot.scheduledSinceMs.store(0, std::memory_order_release);
		}

		[[nodiscard]] inline SyncRequestAction DecideRequestAction(
			const SyncSlotState& slot,
			std::uint64_t nowMs,
			std::uint64_t stuckMs) noexcept
		{
			return DecideSyncRequestAction(
				slot.scheduled.load(std::memory_order_acquire),
				slot.scheduledSinceMs.load(std::memory_order_acquire),
				nowMs,
				stuckMs);
		}

		[[nodiscard]] inline bool TryStartSyncSlot(SyncSlotState& slot, std::uint64_t nowMs) noexcept
		{
			if (slot.scheduled.exchange(true, std::memory_order_acq_rel)) {
				slot.rerunRequested.store(true, std::memory_order_release);
				return false;
			}
			slot.scheduledSinceMs.store(nowMs, std::memory_order_release);
			return true;
		}
	}

	struct RuntimeSnapshot
	{
		bool          rewardSyncScheduled{ false };
		bool          rewardSyncRerunRequested{ false };
		std::uint64_t rewardSyncScheduledSinceMs{ 0 };
		bool          carryWeightQuickResyncScheduled{ false };
		bool          carryWeightQuickResyncRerunRequested{ false };
		std::uint64_t carryWeightQuickResyncScheduledSinceMs{ 0 };
		std::uint64_t generation{ 1 };
	};

	[[nodiscard]] inline RuntimeSnapshot Snapshot() noexcept
	{
		const auto& state = Detail::g_runtimeState;
		return RuntimeSnapshot{
			state.rewardSync.scheduled.load(std::memory_order_acquire),
			state.rewardSync.rerunRequested.load(std::memory_order_acquire),
			state.rewardSync.scheduledSinceMs.load(std::memory_order_acquire),
			state.carryWeightQuickResync.scheduled.load(std::memory_order_acquire),
			state.carryWeightQuickResync.rerunRequested.load(std::memory_order_acquire),
			state.carryWeightQuickResync.scheduledSinceMs.load(std::memory_order_acquire),
			state.generation.load(std::memory_order_acquire)
		};
	}

	inline void ResetForTesting(std::uint64_t generation = 1) noexcept
	{
		auto& state = Detail::g_runtimeState;
		Detail::ClearSyncSlot(state.rewardSync);
		Detail::ClearSyncSlot(state.carryWeightQuickResync);
		state.generation.store(generation, std::memory_order_release);
	}

	[[nodiscard]] inline std::uint64_t CurrentGeneration() noexcept
	{
		return Detail::g_runtimeState.generation.load(std::memory_order_acquire);
	}

	[[nodiscard]] inline bool IsCurrentGeneration(std::uint64_t generation) noexcept
	{
		return generation == CurrentGeneration();
	}

	[[nodiscard]] inline std::uint64_t BumpGenerationAndClearSchedulers() noexcept
	{
		auto& state = Detail::g_runtimeState;
		const auto generation = state.generation.fetch_add(1, std::memory_order_acq_rel) + 1;
		Detail::ClearSyncSlot(state.rewardSync);
		Detail::ClearSyncSlot(state.carryWeightQuickResync);
		return generation;
	}

	[[nodiscard]] inline SyncRequestAction DecideRewardSyncRequestAction(
		std::uint64_t nowMs,
		std::uint64_t stuckMs) noexcept
	{
		return Detail::DecideRequestAction(Detail::g_runtimeState.rewardSync, nowMs, stuckMs);
	}

	[[nodiscard]] inline SyncRequestAction DecideCarryWeightQuickResyncRequestAction(
		std::uint64_t nowMs,
		std::uint64_t stuckMs) noexcept
	{
		return Detail::DecideRequestAction(Detail::g_runtimeState.carryWeightQuickResync, nowMs, stuckMs);
	}

	inline void MarkRewardSyncRerunRequested() noexcept
	{
		Detail::g_runtimeState.rewardSync.rerunRequested.store(true, std::memory_order_release);
	}

	[[nodiscard]] inline bool ConsumeRewardSyncRerunRequested() noexcept
	{
		return Detail::g_runtimeState.rewardSync.rerunRequested.exchange(false, std::memory_order_acq_rel);
	}

	[[nodiscard]] inline bool TryStartRewardSync(std::uint64_t nowMs) noexcept
	{
		return Detail::TryStartSyncSlot(Detail::g_runtimeState.rewardSync, nowMs);
	}

	inline void TouchRewardSyncScheduledSince(std::uint64_t nowMs) noexcept
	{
		Detail::g_runtimeState.rewardSync.scheduledSinceMs.store(nowMs, std::memory_order_release);
	}

	inline void CompleteRewardSyncRun(std::uint64_t generation) noexcept
	{
		if (!IsCurrentGeneration(generation)) {
			return;
		}
		Detail::ClearSyncSlot(Detail::g_runtimeState.rewardSync);
	}

	inline void MarkCarryWeightQuickResyncRerunRequested() noexcept
	{
		Detail::g_runtimeState.carryWeightQuickResync.rerunRequested.store(true, std::memory_order_release);
	}

	[[nodiscard]] inline bool ConsumeCarryWeightQuickResyncRerunRequested() noexcept
	{
		return Detail::g_runtimeState.carryWeightQuickResync.rerunRequested.exchange(
			false,
			std::memory_order_acq_rel);
	}

	[[nodiscard]] inline bool TryStartCarryWeightQuickResync(std::uint64_t nowMs) noexcept
	{
		return Detail::TryStartSyncSlot(Detail::g_runtimeState.carryWeightQuickResync, nowMs);
	}

	inline void TouchCarryWeightQuickResyncScheduledSince(std::uint64_t nowMs) noexcept
	{
		Detail::g_runtimeState.carryWeightQuickResync.scheduledSinceMs.store(nowMs, std::memory_order_release);
	}

	inline void CompleteCarryWeightQuickResyncRun(std::uint64_t generation) noexcept
	{
		if (!IsCurrentGeneration(generation)) {
			return;
		}
		Detail::ClearSyncSlot(Detail::g_runtimeState.carryWeightQuickResync);
	}
}
