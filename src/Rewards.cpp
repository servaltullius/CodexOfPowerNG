#include "CodexOfPowerNG/Rewards.h"

#include "CodexOfPowerNG/Config.h"
#include "CodexOfPowerNG/L10n.h"
#include "CodexOfPowerNG/RewardCaps.h"
#include "CodexOfPowerNG/RewardsResync.h"
#include "CodexOfPowerNG/RewardsSyncRuntime.h"
#include "CodexOfPowerNG/RewardsSyncPolicy.h"
#include "CodexOfPowerNG/State.h"
#include "CodexOfPowerNG/TaskScheduler.h"
#include "CodexOfPowerNG/Util.h"

#include "RewardsInternal.h"
#include "RewardsSyncEngine.h"

#include <RE/Skyrim.h>

#include <RE/M/Misc.h>
#include <SKSE/Logger.h>

#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
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
		inline constexpr std::uint64_t kRewardSyncReadinessTimeoutMs = 20000;
		inline constexpr std::uint64_t kCarryWeightQuickResyncReadinessTimeoutMs = 10000;

		using RewardSyncPassState = Engine::RewardSyncPassState;

		struct CarryWeightQuickResyncState
		{
			std::uint32_t attempt{ 0 };
			std::uint64_t generation{ 0 };
			std::uint64_t readinessDeadlineMs{ 0 };
		};

		enum class CarryWeightQuickResyncAttemptResult : std::uint8_t
		{
			kDone,
			kRetryWithoutAttemptConsume,
			kRetryWithAttemptConsume
		};

		void RunCarryWeightQuickResync(std::shared_ptr<CarryWeightQuickResyncState> state) noexcept;

		[[nodiscard]] bool IsRewardSyncEnvironmentReady() noexcept
		{
			if (!RE::PlayerCharacter::GetSingleton()) {
				return false;
			}
			const auto* main = RE::Main::GetSingleton();
			if (!main) {
				return false;
			}
			return main->gameActive;
		}

		void CompleteRewardSyncRun(std::uint64_t generation) noexcept
		{
			SyncRuntime::CompleteRewardSyncRun(generation);
		}

		void FinalizeRewardSyncRun(std::shared_ptr<RewardSyncPassState> passState) noexcept
		{
			if (!passState) {
				return;
			}
			if (!SyncRuntime::IsCurrentGeneration(passState->generation)) {
				return;
			}

			Engine::FinalizeRewardSyncPass(*passState);
			CompleteRewardSyncRun(passState->generation);
		}

		void CompleteCarryWeightQuickResync(std::uint64_t generation) noexcept
		{
			if (!SyncRuntime::IsCurrentGeneration(generation)) {
				return;
			}

			if (SyncRuntime::ConsumeCarryWeightQuickResyncRerunRequested()) {
				SyncRuntime::TouchCarryWeightQuickResyncScheduledSince(NowMs());
				auto rerunState = std::make_shared<CarryWeightQuickResyncState>();
				rerunState->generation = generation;
				if (QueueMainTask([rerunState]() { RunCarryWeightQuickResync(rerunState); })) {
					return;
				}
				RunCarryWeightQuickResync(rerunState);
				return;
			}

			SyncRuntime::CompleteCarryWeightQuickResyncRun(generation);
		}

		[[nodiscard]] CarryWeightQuickResyncAttemptResult TryCarryWeightQuickResyncAttempt(std::uint32_t attempt) noexcept
		{
			if (!IsRewardSyncEnvironmentReady()) {
				return CarryWeightQuickResyncAttemptResult::kRetryWithoutAttemptConsume;
			}

			auto* player = RE::PlayerCharacter::GetSingleton();
			if (!player) {
				return CarryWeightQuickResyncAttemptResult::kRetryWithoutAttemptConsume;
			}
			auto* avOwner = player->AsActorValueOwner();
			if (!avOwner) {
				return CarryWeightQuickResyncAttemptResult::kRetryWithoutAttemptConsume;
			}

			const float total = Engine::SnapshotRewardTotalForActorValue(RE::ActorValue::kCarryWeight);
			if (std::abs(total) <= kRewardCapEpsilon) {
				return CarryWeightQuickResyncAttemptResult::kDone;
			}

			const float base = avOwner->GetBaseActorValue(RE::ActorValue::kCarryWeight);
			const float current = avOwner->GetActorValue(RE::ActorValue::kCarryWeight);
			const float permanent = avOwner->GetPermanentActorValue(RE::ActorValue::kCarryWeight);
			const float permanentModifier = player->GetActorValueModifier(
				RE::ACTOR_VALUE_MODIFIER::kPermanent,
				RE::ActorValue::kCarryWeight);
			const float delta = ComputeCarryWeightSyncDelta(
				base,
				current,
				permanent,
				permanentModifier,
				total);
			if (std::abs(delta) <= kRewardCapEpsilon) {
				SKSE::log::info(
					"Reward sync (carry weight quick): attempt {} already aligned (expected {:.4f}, current {:.4f})",
					attempt + 1,
					total,
					current);
				return CarryWeightQuickResyncAttemptResult::kDone;
			}

			avOwner->ModActorValue(RE::ActorValue::kCarryWeight, delta);
			const float postBase = avOwner->GetBaseActorValue(RE::ActorValue::kCarryWeight);
			const float postCurrent = avOwner->GetActorValue(RE::ActorValue::kCarryWeight);
			const float postPermanent = avOwner->GetPermanentActorValue(RE::ActorValue::kCarryWeight);
			const float postPermanentModifier = player->GetActorValueModifier(
				RE::ACTOR_VALUE_MODIFIER::kPermanent,
				RE::ActorValue::kCarryWeight);
			const float postDelta = ComputeCarryWeightSyncDelta(
				postBase,
				postCurrent,
				postPermanent,
				postPermanentModifier,
				total);
			if (std::abs(postDelta) <= kRewardCapEpsilon) {
				SKSE::log::info(
					"Reward sync (carry weight quick): attempt {} applied {:.4f} (expected {:.4f}, current {:.4f})",
					attempt + 1,
					delta,
					total,
					current);
				return CarryWeightQuickResyncAttemptResult::kDone;
			}

			SKSE::log::info(
				"Reward sync (carry weight quick): attempt {} pending (expected {:.4f}, delta {:.4f}, postDelta {:.4f})",
				attempt + 1,
				total,
				delta,
				postDelta);
			return CarryWeightQuickResyncAttemptResult::kRetryWithAttemptConsume;
		}

		void RunCarryWeightQuickResync(std::shared_ptr<CarryWeightQuickResyncState> state) noexcept
		{
			if (!state) {
				return;
			}
			if (!SyncRuntime::IsCurrentGeneration(state->generation)) {
				return;
			}

			const auto result = TryCarryWeightQuickResyncAttempt(state->attempt);
			if (result == CarryWeightQuickResyncAttemptResult::kDone) {
				CompleteCarryWeightQuickResync(state->generation);
				return;
			}

			if (result == CarryWeightQuickResyncAttemptResult::kRetryWithoutAttemptConsume) {
				const auto nowMs = NowMs();
				if (state->readinessDeadlineMs == 0) {
					state->readinessDeadlineMs = nowMs + kCarryWeightQuickResyncReadinessTimeoutMs;
				}
				if (nowMs >= state->readinessDeadlineMs) {
					SKSE::log::warn(
						"Reward sync (carry weight quick): skipped after waiting {} ms for load readiness",
						kCarryWeightQuickResyncReadinessTimeoutMs);
					CompleteCarryWeightQuickResync(state->generation);
					return;
				}
				if (QueueMainTask([state]() { RunCarryWeightQuickResync(state); })) {
					return;
				}

				SKSE::log::warn("Reward sync (carry weight quick): scheduler unavailable while waiting for load readiness");
				CompleteCarryWeightQuickResync(state->generation);
				return;
			}

			++state->attempt;
			if (state->attempt >= kCarryWeightQuickResyncMaxAttempts) {
				SKSE::log::warn(
					"Reward sync (carry weight quick): unresolved after {} attempts",
					kCarryWeightQuickResyncMaxAttempts);
				CompleteCarryWeightQuickResync(state->generation);
				return;
			}

			if (QueueMainTask([state]() { RunCarryWeightQuickResync(state); })) {
				return;
			}

			// Fallback when task interface is unavailable: finish synchronously.
			bool abortedForReadiness = false;
			while (
				state->attempt < kCarryWeightQuickResyncMaxAttempts &&
				SyncRuntime::IsCurrentGeneration(state->generation)) {
				const auto fallbackResult = TryCarryWeightQuickResyncAttempt(state->attempt);
				if (fallbackResult == CarryWeightQuickResyncAttemptResult::kDone) {
					CompleteCarryWeightQuickResync(state->generation);
					return;
				}
				if (fallbackResult == CarryWeightQuickResyncAttemptResult::kRetryWithoutAttemptConsume) {
					SKSE::log::warn("Reward sync (carry weight quick): not ready during fallback; aborting this quick pass");
					abortedForReadiness = true;
					break;
				}
				++state->attempt;
			}
			if (abortedForReadiness) {
				CompleteCarryWeightQuickResync(state->generation);
				return;
			}

			SKSE::log::warn(
				"Reward sync (carry weight quick): unresolved after {} attempts",
				kCarryWeightQuickResyncMaxAttempts);
			CompleteCarryWeightQuickResync(state->generation);
		}

		void RunRewardSyncPasses(std::shared_ptr<RewardSyncPassState> passState, std::uint32_t remainingPasses) noexcept
		{
			if (!passState) {
				return;
			}
			if (!SyncRuntime::IsCurrentGeneration(passState->generation)) {
				return;
			}

			if (!IsRewardSyncEnvironmentReady()) {
				const auto nowMs = NowMs();
				if (passState->readinessDeadlineMs == 0) {
					passState->readinessDeadlineMs = nowMs + kRewardSyncReadinessTimeoutMs;
				}
				if (nowMs >= passState->readinessDeadlineMs) {
					SKSE::log::warn(
						"Reward sync: skipped after waiting {} ms for load readiness",
						kRewardSyncReadinessTimeoutMs);
					CompleteRewardSyncRun(passState->generation);
					return;
				}
				if (QueueMainTask([passState, remainingPasses]() { RunRewardSyncPasses(passState, remainingPasses); })) {
					return;
				}

				SKSE::log::warn("Reward sync: scheduler unavailable while waiting for load readiness");
				CompleteRewardSyncRun(passState->generation);
				return;
			}

			Engine::PrepareRewardSyncPass(*passState);
			const auto passResult = Engine::ApplyRewardSyncPass(*passState, kRewardSyncMinMissingStreak);

			const auto scheduleRerunIfRequested = [&]() -> bool {
				if (!SyncRuntime::ConsumeRewardSyncRerunRequested()) {
					return false;
				}

				SyncRuntime::TouchRewardSyncScheduledSince(NowMs());
				auto rerunPassState = std::make_shared<RewardSyncPassState>();
				rerunPassState->weaponAbilityRefreshRequested = passState->weaponAbilityRefreshRequested;
				rerunPassState->generation = passState->generation;
				if (QueueMainTask([rerunPassState]() { RunRewardSyncPasses(rerunPassState, kRewardSyncPassCount); })) {
					return true;
				}
				RunRewardSyncPasses(rerunPassState, kRewardSyncPassCount);
				return true;
			};

			if (ShouldStopSyncAfterPass(passResult.correctedCount, passResult.pendingCount)) {
				const auto remainingAfterThisPass = remainingPasses > 0 ? (remainingPasses - 1) : 0;
				SKSE::log::info("Reward sync: converged early (remaining passes: {})", remainingAfterThisPass);
				if (scheduleRerunIfRequested()) {
					return;
				}
				FinalizeRewardSyncRun(passState);
				return;
			}

			if (remainingPasses <= 1) {
				if (scheduleRerunIfRequested()) {
					return;
				}

				FinalizeRewardSyncRun(passState);
				return;
			}

			if (QueueMainTask([passState, remainingPasses]() { RunRewardSyncPasses(passState, remainingPasses - 1); })) {
				return;
			}

			// Fallback when task interface is unavailable: finish synchronously.
			for (std::uint32_t i = 1; i < remainingPasses; ++i) {
				if (!SyncRuntime::IsCurrentGeneration(passState->generation)) {
					return;
				}
				if (!IsRewardSyncEnvironmentReady()) {
					SKSE::log::warn("Reward sync: fallback pass stopped because game is not ready");
					CompleteRewardSyncRun(passState->generation);
					return;
				}
				(void)Engine::ApplyRewardSyncPass(*passState, kRewardSyncMinMissingStreak);
			}
			FinalizeRewardSyncRun(passState);
		}
		}

	std::vector<Registration::RewardDelta> MaybeGrantRegistrationReward(
		std::uint32_t group,
		std::int32_t totalRegistered) noexcept
	{
		std::vector<Registration::RewardDelta> appliedDeltas;
		const auto settings = GetSettings();
		if (!settings.enableRewards) {
			return appliedDeltas;
		}

		const auto every = settings.rewardEvery;
		if (every <= 0) {
			return appliedDeltas;
		}

		if (totalRegistered <= 0) {
			return appliedDeltas;
		}

		if ((totalRegistered % every) != 0) {
			return appliedDeltas;
		}

		Internal::BeginRewardDeltaCapture(appliedDeltas);
		Internal::GrantWeightedRandomReward(group);
		Internal::EndRewardDeltaCapture();
		return appliedDeltas;
	}

	void SyncRewardTotalsToPlayer() noexcept
	{
		auto generation = SyncRuntime::CurrentGeneration();
		const auto nowMs = NowMs();
		const auto action = SyncRuntime::DecideRewardSyncRequestAction(nowMs, kRewardSyncStuckMs);

		if (action == SyncRequestAction::kMarkRerun) {
			SyncRuntime::MarkRewardSyncRerunRequested();
			return;
		}

		if (action == SyncRequestAction::kForceRestartAndStart) {
			generation = SyncRuntime::BumpGenerationAndClearSchedulers();
			SKSE::log::warn("Reward sync watchdog: force restarting stale sync worker (generation {})", generation);
		}

			if (!SyncRuntime::TryStartRewardSync(nowMs)) {
				return;
			}
			auto passState = std::make_shared<RewardSyncPassState>();
			passState->generation = generation;
			passState->weaponAbilityRefreshRequested =
				std::abs(Engine::SnapshotRewardTotalForActorValue(RE::ActorValue::kAttackDamageMult)) > kRewardCapEpsilon;

		if (QueueMainTask([passState]() { RunRewardSyncPasses(passState, kRewardSyncPassCount); })) {
			return;
		}

		RunRewardSyncPasses(passState, kRewardSyncPassCount);
	}

	void ResetSyncSchedulersForLoad() noexcept
	{
		const auto generation = SyncRuntime::BumpGenerationAndClearSchedulers();
		SKSE::log::info("Reward sync schedulers reset for load boundary (generation {})", generation);
	}

	void ScheduleCarryWeightQuickResync() noexcept
	{
		auto generation = SyncRuntime::CurrentGeneration();
		const auto nowMs = NowMs();
		const auto action = SyncRuntime::DecideCarryWeightQuickResyncRequestAction(nowMs, kCarryWeightQuickResyncStuckMs);

		if (action == SyncRequestAction::kForceRestartAndStart) {
			generation = SyncRuntime::BumpGenerationAndClearSchedulers();
			SKSE::log::warn(
				"Reward sync (carry weight quick) watchdog: force restarting stale worker (generation {})",
				generation);
		}

		if (!SyncRuntime::TryStartCarryWeightQuickResync(nowMs)) {
			return;
		}

		auto state = std::make_shared<CarryWeightQuickResyncState>();
		state->generation = generation;
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

			Engine::NormalizeRewardCapsOnStateAndPlayer();
			auto totals = Engine::SnapshotRewardTotals();
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

	std::size_t RollbackRewardDeltas(const std::vector<Registration::RewardDelta>& deltas) noexcept
	{
		if (deltas.empty()) {
			return 0;
		}

		auto* player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			return 0;
		}
		auto* avOwner = player->AsActorValueOwner();
		if (!avOwner) {
			return 0;
		}

		std::vector<Registration::RewardDelta> actorAdjustments;
		actorAdjustments.reserve(deltas.size());

		{
			auto& state = GetState();
			std::scoped_lock lock(state.mutex);

			for (const auto& deltaEntry : deltas) {
				const auto av = deltaEntry.av;
				const auto delta = deltaEntry.delta;
				if (std::abs(delta) <= kRewardCapEpsilon) {
					continue;
				}

				const auto it = state.rewardTotals.find(av);
				if (it == state.rewardTotals.end()) {
					continue;
				}

				const float previousTotal = it->second;
				const float next = ClampRewardTotal(av, previousTotal - delta);
				const float previousApplied = ActorAppliedRewardTotal(av, previousTotal);
				const float nextApplied = ActorAppliedRewardTotal(av, next);
				const float appliedActorDelta = nextApplied - previousApplied;
				if (std::abs(next) <= kRewardCapEpsilon) {
					state.rewardTotals.erase(it);
				} else {
					it->second = next;
				}

				if (std::abs(appliedActorDelta) > kRewardCapEpsilon) {
					actorAdjustments.push_back(Registration::RewardDelta{ av, appliedActorDelta });
				}
			}
		}

		bool carryWeightTouched = false;
		for (const auto& adjustment : actorAdjustments) {
			avOwner->ModActorValue(adjustment.av, adjustment.delta);
			if (adjustment.av == RE::ActorValue::kCarryWeight) {
				carryWeightTouched = true;
			}
		}

		if (carryWeightTouched) {
			ScheduleCarryWeightQuickResync();
		}

		return actorAdjustments.size();
	}
}
