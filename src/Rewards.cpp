#include "CodexOfPowerNG/Rewards.h"

#include "CodexOfPowerNG/Config.h"
#include "CodexOfPowerNG/L10n.h"
#include "CodexOfPowerNG/RewardCaps.h"
#include "CodexOfPowerNG/RewardsResync.h"
#include "CodexOfPowerNG/RewardsSyncPolicy.h"
#include "CodexOfPowerNG/State.h"
#include "CodexOfPowerNG/TaskScheduler.h"
#include "CodexOfPowerNG/Util.h"

#include "RewardsInternal.h"

#include <RE/Skyrim.h>

#include <RE/M/Misc.h>

#include <SKSE/Logger.h>

#include <atomic>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace CodexOfPowerNG::Rewards
{
	namespace
	{
		inline constexpr std::uint32_t kRewardSyncPassCount = 180;
		inline constexpr std::uint32_t kRewardSyncMinMissingStreak = 3;
		inline constexpr std::uint64_t kRewardSyncStuckMs = 15000;
		inline constexpr std::uint32_t kCarryWeightQuickResyncMaxAttempts = 3;
		inline constexpr std::uint64_t kCarryWeightQuickResyncStuckMs = 3000;

		std::atomic_bool g_rewardSyncScheduled{ false };
		std::atomic_bool g_rewardSyncRerunRequested{ false };
		std::atomic<std::uint64_t> g_rewardSyncScheduledSinceMs{ 0 };
		std::atomic_bool g_carryWeightQuickResyncScheduled{ false };
		std::atomic_bool g_carryWeightQuickResyncRerunRequested{ false };
		std::atomic<std::uint64_t> g_carryWeightQuickResyncScheduledSinceMs{ 0 };

		struct RewardSyncPassState
		{
			std::unordered_map<RE::ActorValue, std::uint32_t, ActorValueHash> missingStreaks;
			bool normalizeCapsOnFirstPass{ true };
		};

		struct CarryWeightQuickResyncState
		{
			std::uint32_t attempt{ 0 };
		};

		struct RewardActorSnapshot
		{
			float base{ 0.0f };
			float current{ 0.0f };
			float permanent{ 0.0f };
			float permanentModifier{ 0.0f };
			float delta{ 0.0f };
		};

		struct RewardCapAdjustment
		{
			RE::ActorValue av;
			float          before;
			float          after;
		};

		void RunCarryWeightQuickResync(std::shared_ptr<CarryWeightQuickResyncState> state) noexcept;

		[[nodiscard]] float SnapshotRewardTotalForActorValue(RE::ActorValue av) noexcept
		{
			auto& state = GetState();
			std::scoped_lock lock(state.mutex);
			const auto it = state.rewardTotals.find(av);
			if (it == state.rewardTotals.end()) {
				return 0.0f;
			}

			const float clamped = ClampRewardTotal(av, it->second);
			return (std::abs(clamped) > kRewardCapEpsilon) ? clamped : 0.0f;
		}

		[[nodiscard]] RewardActorSnapshot CaptureRewardActorSnapshot(
			RE::PlayerCharacter* player,
			RE::ActorValueOwner* avOwner,
			RE::ActorValue av,
			float expectedTotal) noexcept
		{
			RewardActorSnapshot snapshot{};
			snapshot.base = avOwner->GetBaseActorValue(av);
			snapshot.current = avOwner->GetActorValue(av);
			snapshot.permanent = avOwner->GetPermanentActorValue(av);
			snapshot.permanentModifier =
				player->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kPermanent, av);
			snapshot.delta = ComputeRewardSyncDeltaFromSnapshot(
				snapshot.base,
				snapshot.current,
				snapshot.permanent,
				snapshot.permanentModifier,
				expectedTotal);
			return snapshot;
		}

		void CompleteCarryWeightQuickResync() noexcept
		{
			if (g_carryWeightQuickResyncRerunRequested.exchange(false, std::memory_order_acq_rel)) {
				g_carryWeightQuickResyncScheduledSinceMs.store(NowMs(), std::memory_order_release);
				auto rerunState = std::make_shared<CarryWeightQuickResyncState>();
				if (QueueMainTask([rerunState]() { RunCarryWeightQuickResync(rerunState); })) {
					return;
				}
				RunCarryWeightQuickResync(rerunState);
				return;
			}

			g_carryWeightQuickResyncScheduled.store(false, std::memory_order_release);
			g_carryWeightQuickResyncScheduledSinceMs.store(0, std::memory_order_release);
		}

		[[nodiscard]] bool TryCarryWeightQuickResyncAttempt(std::uint32_t attempt) noexcept
		{
			auto* player = RE::PlayerCharacter::GetSingleton();
			if (!player) {
				return false;
			}
			auto* avOwner = player->AsActorValueOwner();
			if (!avOwner) {
				return false;
			}

			const float total = SnapshotRewardTotalForActorValue(RE::ActorValue::kCarryWeight);
			if (std::abs(total) <= kRewardCapEpsilon) {
				return true;
			}

			auto snapshot = CaptureRewardActorSnapshot(player, avOwner, RE::ActorValue::kCarryWeight, total);
			const float delta = ComputeCarryWeightSyncDelta(
				snapshot.base,
				snapshot.current,
				snapshot.permanent,
				snapshot.permanentModifier,
				total);
			if (std::abs(delta) <= kRewardCapEpsilon) {
				SKSE::log::info(
					"Reward sync (carry weight quick): attempt {} already aligned (expected {:.4f}, current {:.4f})",
					attempt + 1,
					total,
					snapshot.current);
				return true;
			}

			avOwner->ModActorValue(RE::ActorValue::kCarryWeight, delta);
			const auto postSnapshot = CaptureRewardActorSnapshot(player, avOwner, RE::ActorValue::kCarryWeight, total);
			const float postDelta = ComputeCarryWeightSyncDelta(
				postSnapshot.base,
				postSnapshot.current,
				postSnapshot.permanent,
				postSnapshot.permanentModifier,
				total);
			if (std::abs(postDelta) <= kRewardCapEpsilon) {
				SKSE::log::info(
					"Reward sync (carry weight quick): attempt {} applied {:.4f} (expected {:.4f}, current {:.4f})",
					attempt + 1,
					delta,
					total,
					snapshot.current);
				return true;
			}

			SKSE::log::info(
				"Reward sync (carry weight quick): attempt {} pending (expected {:.4f}, delta {:.4f}, postDelta {:.4f})",
				attempt + 1,
				total,
				delta,
				postDelta);
			return false;
		}

		void RunCarryWeightQuickResync(std::shared_ptr<CarryWeightQuickResyncState> state) noexcept
		{
			if (!state) {
				CompleteCarryWeightQuickResync();
				return;
			}

			if (TryCarryWeightQuickResyncAttempt(state->attempt)) {
				CompleteCarryWeightQuickResync();
				return;
			}

			++state->attempt;
			if (state->attempt >= kCarryWeightQuickResyncMaxAttempts) {
				SKSE::log::warn(
					"Reward sync (carry weight quick): unresolved after {} attempts",
					kCarryWeightQuickResyncMaxAttempts);
				CompleteCarryWeightQuickResync();
				return;
			}

			if (QueueMainTask([state]() { RunCarryWeightQuickResync(state); })) {
				return;
			}

			// Fallback when task interface is unavailable: finish synchronously.
			while (state->attempt < kCarryWeightQuickResyncMaxAttempts) {
				if (TryCarryWeightQuickResyncAttempt(state->attempt)) {
					CompleteCarryWeightQuickResync();
					return;
				}
				++state->attempt;
			}

			SKSE::log::warn(
				"Reward sync (carry weight quick): unresolved after {} attempts",
				kCarryWeightQuickResyncMaxAttempts);
			CompleteCarryWeightQuickResync();
		}

		[[nodiscard]] std::vector<RewardCapAdjustment> ClampRewardTotalsInState() noexcept
		{
			std::vector<RewardCapAdjustment> adjustments;
			auto& state = GetState();
			std::scoped_lock lock(state.mutex);
			adjustments.reserve(state.rewardTotals.size());
			for (auto& [av, total] : state.rewardTotals) {
				const float clamped = ClampRewardTotal(av, total);
				if (std::abs(clamped - total) <= kRewardCapEpsilon) {
					continue;
				}

				adjustments.push_back(RewardCapAdjustment{ av, total, clamped });
				total = clamped;
			}
			return adjustments;
		}

		void ApplyRewardCapAdjustmentsToPlayer(const std::vector<RewardCapAdjustment>& adjustments) noexcept
		{
			if (adjustments.empty()) {
				return;
			}

			auto* player = RE::PlayerCharacter::GetSingleton();
			if (!player) {
				return;
			}
			auto* avOwner = player->AsActorValueOwner();
			if (!avOwner) {
				return;
			}

			std::size_t appliedCount = 0;
			for (const auto& entry : adjustments) {
				const float base = avOwner->GetBaseActorValue(entry.av);
				const float cur = avOwner->GetActorValue(entry.av);
				const float permanent = avOwner->GetPermanentActorValue(entry.av);
				const float permanentModifier =
					player->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kPermanent, entry.av);

				const float delta = ComputeCapNormalizationDeltaFromSnapshot(
					base,
					cur,
					permanent,
					permanentModifier,
					entry.after,
					kRewardCapEpsilon);
				if (delta >= -kRewardCapEpsilon) {
					continue;
				}

				avOwner->ModActorValue(entry.av, delta);
				++appliedCount;

				float cap = 0.0f;
				if (TryGetRewardCap(entry.av, cap)) {
					SKSE::log::warn(
						"Reward cap normalize: AV {} total {:.4f} -> {:.4f} (cap {:.2f}, base {:.4f}, current {:.4f}, permanent {:.4f}, permMod {:.4f}, actor delta {:.4f})",
						static_cast<std::uint32_t>(entry.av),
						entry.before,
						entry.after,
						cap,
						base,
						cur,
						permanent,
						permanentModifier,
						delta);
				}
			}

			if (appliedCount > 0) {
				SKSE::log::warn("Reward cap normalize: adjusted {} actor value entries", appliedCount);
			}
		}

		[[nodiscard]] std::vector<std::pair<RE::ActorValue, float>> SnapshotRewardTotals() noexcept
		{
			std::vector<std::pair<RE::ActorValue, float>> totals;
			auto& state = GetState();
			std::scoped_lock lock(state.mutex);
			totals.reserve(state.rewardTotals.size());
			for (const auto& [av, total] : state.rewardTotals) {
				const float clamped = ClampRewardTotal(av, total);
				if (std::abs(clamped) > kRewardCapEpsilon) {
					totals.emplace_back(av, clamped);
				}
			}
			return totals;
		}

		[[nodiscard]] std::size_t ApplyRewardSyncPass(RewardSyncPassState& passState) noexcept
		{
			auto* player = RE::PlayerCharacter::GetSingleton();
			if (!player) {
				return 0;
			}
			auto* avOwner = player->AsActorValueOwner();
			if (!avOwner) {
				return 0;
			}

			const auto totals = SnapshotRewardTotals();
			if (totals.empty()) {
				return 0;
			}

			std::size_t corrected = 0;
			for (const auto& [av, total] : totals) {
				const auto snapshot = CaptureRewardActorSnapshot(player, avOwner, av, total);
				const float base = snapshot.base;
				const float cur = snapshot.current;
				const float permanent = snapshot.permanent;
				const float permanentModifier = snapshot.permanentModifier;
				float delta = (av == RE::ActorValue::kCarryWeight)
					? ComputeCarryWeightSyncDelta(base, cur, permanent, permanentModifier, total)
					: snapshot.delta;
				auto& streak = passState.missingStreaks[av];
				streak = NextMissingStreak(delta, streak);
				if (av == RE::ActorValue::kCarryWeight && std::abs(delta) > kRewardCapEpsilon && streak <= kRewardSyncMinMissingStreak) {
					SKSE::log::info(
						"Reward sync (carry weight): expected {:.4f}, base {:.4f}, current {:.4f}, permanent {:.4f}, permMod {:.4f}, delta {:.4f}, streak {}",
						total,
						base,
						cur,
						permanent,
						permanentModifier,
						delta,
						streak);
				}

				if (!ShouldApplyAfterStreak(delta, streak, kRewardSyncMinMissingStreak)) {
					continue;
				}
				streak = 0;

				// Keep shout cooldown in sane range even if load-time state diverges.
				if (av == RE::ActorValue::kShoutRecoveryMult) {
					const float minValue = 0.30f;
					if (cur <= minValue) {
						continue;
					}
					if ((cur + delta) < minValue) {
						delta = minValue - cur;
					}
				}

				if (delta == 0.0f) {
					continue;
				}

				avOwner->ModActorValue(av, delta);
				++corrected;
				if (av == RE::ActorValue::kCarryWeight) {
					SKSE::log::info(
						"Reward sync (carry weight): applied {:.4f} (expected {:.4f}, current {:.4f})",
						delta,
						total,
						cur);
				}
				SKSE::log::info(
					"Reward sync: AV {} adjusted by {:.4f} (base {:.4f}, current {:.4f}, expectedTotal {:.4f})",
					static_cast<std::uint32_t>(av),
					delta,
					base,
					cur,
					total);
			}

			if (corrected > 0) {
				SKSE::log::info("Reward sync: corrected {} actor value entries", corrected);
			}

			return corrected;
		}

		void RunRewardSyncPasses(std::shared_ptr<RewardSyncPassState> passState, std::uint32_t remainingPasses) noexcept
		{
			if (passState->normalizeCapsOnFirstPass) {
				passState->normalizeCapsOnFirstPass = false;
				const auto capAdjustments = ClampRewardTotalsInState();
				ApplyRewardCapAdjustmentsToPlayer(capAdjustments);
			}

			(void)ApplyRewardSyncPass(*passState);

			if (remainingPasses <= 1) {
				if (g_rewardSyncRerunRequested.exchange(false, std::memory_order_acq_rel)) {
					g_rewardSyncScheduledSinceMs.store(NowMs(), std::memory_order_release);
					auto rerunPassState = std::make_shared<RewardSyncPassState>();
					if (QueueMainTask([rerunPassState]() { RunRewardSyncPasses(rerunPassState, kRewardSyncPassCount); })) {
						return;
					}
					RunRewardSyncPasses(rerunPassState, kRewardSyncPassCount);
					return;
				}

				g_rewardSyncScheduled.store(false, std::memory_order_release);
				g_rewardSyncScheduledSinceMs.store(0, std::memory_order_release);
				return;
			}

			if (QueueMainTask([passState, remainingPasses]() { RunRewardSyncPasses(passState, remainingPasses - 1); })) {
				return;
			}

			// Fallback when task interface is unavailable: finish synchronously.
			for (std::uint32_t i = 1; i < remainingPasses; ++i) {
				(void)ApplyRewardSyncPass(*passState);
			}
			g_rewardSyncScheduled.store(false, std::memory_order_release);
			g_rewardSyncScheduledSinceMs.store(0, std::memory_order_release);
		}
	}

	void MaybeGrantRegistrationReward(std::uint32_t group, std::int32_t totalRegistered) noexcept
	{
		const auto settings = GetSettings();
		if (!settings.enableRewards) {
			return;
		}

		const auto every = settings.rewardEvery;
		if (every <= 0) {
			return;
		}

		if (totalRegistered <= 0) {
			return;
		}

		if ((totalRegistered % every) != 0) {
			return;
		}

		Internal::GrantWeightedRandomReward(group);
	}

	void SyncRewardTotalsToPlayer() noexcept
	{
		const auto nowMs = NowMs();
		const auto action = DecideSyncRequestAction(
			g_rewardSyncScheduled.load(std::memory_order_acquire),
			g_rewardSyncScheduledSinceMs.load(std::memory_order_acquire),
			nowMs,
			kRewardSyncStuckMs);

		if (action == SyncRequestAction::kMarkRerun) {
			g_rewardSyncRerunRequested.store(true, std::memory_order_release);
			return;
		}

		if (action == SyncRequestAction::kForceRestartAndStart) {
			SKSE::log::warn("Reward sync watchdog: force restarting stale sync worker");
			g_rewardSyncScheduled.store(false, std::memory_order_release);
			g_rewardSyncRerunRequested.store(false, std::memory_order_release);
		}

		if (g_rewardSyncScheduled.exchange(true, std::memory_order_acq_rel)) {
			g_rewardSyncRerunRequested.store(true, std::memory_order_release);
			return;
		}

		g_rewardSyncScheduledSinceMs.store(nowMs, std::memory_order_release);
		auto passState = std::make_shared<RewardSyncPassState>();

		if (QueueMainTask([passState]() { RunRewardSyncPasses(passState, kRewardSyncPassCount); })) {
			return;
		}

		RunRewardSyncPasses(passState, kRewardSyncPassCount);
	}

	void ScheduleCarryWeightQuickResync() noexcept
	{
		const auto nowMs = NowMs();
		const auto action = DecideSyncRequestAction(
			g_carryWeightQuickResyncScheduled.load(std::memory_order_acquire),
			g_carryWeightQuickResyncScheduledSinceMs.load(std::memory_order_acquire),
			nowMs,
			kCarryWeightQuickResyncStuckMs);

		if (action == SyncRequestAction::kForceRestartAndStart) {
			SKSE::log::warn("Reward sync (carry weight quick) watchdog: force restarting stale worker");
			g_carryWeightQuickResyncScheduled.store(false, std::memory_order_release);
			g_carryWeightQuickResyncRerunRequested.store(false, std::memory_order_release);
			g_carryWeightQuickResyncScheduledSinceMs.store(0, std::memory_order_release);
		}

		if (g_carryWeightQuickResyncScheduled.exchange(true, std::memory_order_acq_rel)) {
			g_carryWeightQuickResyncRerunRequested.store(true, std::memory_order_release);
			return;
		}
		g_carryWeightQuickResyncScheduledSinceMs.store(nowMs, std::memory_order_release);

		auto state = std::make_shared<CarryWeightQuickResyncState>();
		if (QueueMainTask([state]() { RunCarryWeightQuickResync(state); })) {
			return;
		}

		RunCarryWeightQuickResync(state);
	}

	std::size_t RefundRewards() noexcept
	{
		auto* player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			return 0;
		}
		auto* avOwner = player->AsActorValueOwner();
		if (!avOwner) {
			return 0;
		}

		const auto capAdjustments = ClampRewardTotalsInState();
		ApplyRewardCapAdjustmentsToPlayer(capAdjustments);

		auto totals = SnapshotRewardTotals();
		{
			auto& state = GetState();
			std::scoped_lock lock(state.mutex);
			state.rewardTotals.clear();
		}

		std::size_t cleared = 0;
		for (const auto& [av, total] : totals) {
			avOwner->ModActorValue(av, -total);
			++cleared;
		}

		const auto msg =
			L10n::T("msg.rewardResetPrefix", "Codex of Power: Rewards reset (") +
			std::to_string(cleared) +
			L10n::T("msg.countSuffix", " items)");
		RE::DebugNotification(msg.c_str());

		return cleared;
	}
}
