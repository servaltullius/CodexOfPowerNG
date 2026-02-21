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
#include <RE/T/TESObjectWEAP.h>

#include <SKSE/Logger.h>

#include <atomic>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
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

		std::atomic_bool g_rewardSyncScheduled{ false };
		std::atomic_bool g_rewardSyncRerunRequested{ false };
		std::atomic<std::uint64_t> g_rewardSyncScheduledSinceMs{ 0 };
		std::atomic_bool g_carryWeightQuickResyncScheduled{ false };
		std::atomic_bool g_carryWeightQuickResyncRerunRequested{ false };
		std::atomic<std::uint64_t> g_carryWeightQuickResyncScheduledSinceMs{ 0 };
		std::atomic<std::uint64_t> g_rewardSyncGeneration{ 1 };

		struct RewardSyncPassState
		{
			std::unordered_map<RE::ActorValue, std::uint32_t, ActorValueHash> missingStreaks;
			std::unordered_set<RE::ActorValue, ActorValueHash>                nonConvergingActorValues;
			bool normalizeCapsOnFirstPass{ true };
			bool weaponAbilityRefreshRequested{ false };
			std::uint64_t generation{ 0 };
			std::uint64_t readinessDeadlineMs{ 0 };
		};

		struct RewardSyncPassResult
		{
			std::size_t correctedCount{ 0 };
			std::size_t pendingCount{ 0 };
		};

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

		[[nodiscard]] std::uint64_t CurrentSyncGeneration() noexcept
		{
			return g_rewardSyncGeneration.load(std::memory_order_acquire);
		}

		[[nodiscard]] std::uint64_t BumpSyncGenerationAndClearSchedulers() noexcept
		{
			const auto generation = g_rewardSyncGeneration.fetch_add(1, std::memory_order_acq_rel) + 1;
			g_rewardSyncScheduled.store(false, std::memory_order_release);
			g_rewardSyncRerunRequested.store(false, std::memory_order_release);
			g_rewardSyncScheduledSinceMs.store(0, std::memory_order_release);
			g_carryWeightQuickResyncScheduled.store(false, std::memory_order_release);
			g_carryWeightQuickResyncRerunRequested.store(false, std::memory_order_release);
			g_carryWeightQuickResyncScheduledSinceMs.store(0, std::memory_order_release);
			return generation;
		}

		[[nodiscard]] bool IsCurrentSyncGeneration(std::uint64_t generation) noexcept
		{
			return generation == CurrentSyncGeneration();
		}

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

		[[nodiscard]] RE::ExtraDataList* PickFirstExtraDataList(RE::InventoryEntryData* entry) noexcept
		{
			if (!entry || !entry->extraLists) {
				return nullptr;
			}

			for (auto* extraData : *entry->extraLists) {
				if (extraData) {
					return extraData;
				}
			}

			return nullptr;
		}

		void RefreshEquippedWeaponAbilityForHand(RE::Actor* actor, bool leftHand) noexcept
		{
			if (!actor) {
				return;
			}

			auto* equippedForm = actor->GetEquippedObject(leftHand);
			if (!equippedForm) {
				return;
			}
			if (!equippedForm->As<RE::TESObjectWEAP>()) {
				return;
			}

			auto* entry = actor->GetEquippedEntryData(leftHand);
			auto* extraData = PickFirstExtraDataList(entry);
			actor->UpdateWeaponAbility(equippedForm, extraData, leftHand);
		}

		void RefreshEquippedWeaponAbilities() noexcept
		{
			auto* player = RE::PlayerCharacter::GetSingleton();
			if (!player) {
				return;
			}

			RefreshEquippedWeaponAbilityForHand(player, false);
			RefreshEquippedWeaponAbilityForHand(player, true);
		}

		void MigrateLegacyAttackDamageMultReward(RewardSyncPassState& passState) noexcept
		{
			float legacyTotal = 0.0f;
			{
				auto& state = GetState();
				std::scoped_lock lock(state.mutex);
				const auto it = state.rewardTotals.find(RE::ActorValue::kAttackDamageMult);
				if (it == state.rewardTotals.end()) {
					return;
				}
				legacyTotal = it->second;
				state.rewardTotals.erase(it);
			}

			if (std::abs(legacyTotal) <= kRewardCapEpsilon) {
				return;
			}

			auto* player = RE::PlayerCharacter::GetSingleton();
			if (!player) {
				SKSE::log::warn(
					"Legacy reward migration: cleared AttackDamageMult total {:.4f} from state (player unavailable)",
					legacyTotal);
				return;
			}
			auto* avOwner = player->AsActorValueOwner();
			if (!avOwner) {
				SKSE::log::warn(
					"Legacy reward migration: cleared AttackDamageMult total {:.4f} from state (AV owner unavailable)",
					legacyTotal);
				return;
			}

			const float base = avOwner->GetBaseActorValue(RE::ActorValue::kAttackDamageMult);
			const float cur = avOwner->GetActorValue(RE::ActorValue::kAttackDamageMult);
			const float permanent = avOwner->GetPermanentActorValue(RE::ActorValue::kAttackDamageMult);
			const float permanentModifier =
				player->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kPermanent, RE::ActorValue::kAttackDamageMult);

			const float observed = SelectObservedRewardTotal(
				legacyTotal,
				cur - base,
				permanent - base,
				permanentModifier);
			const float removable = (std::clamp)(observed, 0.0f, legacyTotal);

			if (removable > kRewardCapEpsilon) {
				avOwner->ModActorValue(RE::ActorValue::kAttackDamageMult, -removable);
				passState.weaponAbilityRefreshRequested = true;
				SKSE::log::warn(
					"Legacy reward migration: removed AttackDamageMult {:.4f} (stored {:.4f}, observed {:.4f})",
					removable,
					legacyTotal,
					observed);
			} else {
				SKSE::log::warn(
					"Legacy reward migration: cleared AttackDamageMult total {:.4f} (no observable actor delta)",
					legacyTotal);
			}
		}

		void CompleteRewardSyncRun(std::uint64_t generation) noexcept
		{
			if (!IsCurrentSyncGeneration(generation)) {
				return;
			}
			g_rewardSyncScheduled.store(false, std::memory_order_release);
			g_rewardSyncRerunRequested.store(false, std::memory_order_release);
			g_rewardSyncScheduledSinceMs.store(0, std::memory_order_release);
		}

		void FinalizeRewardSyncRun(std::shared_ptr<RewardSyncPassState> passState) noexcept
		{
			if (!passState) {
				return;
			}
			if (!IsCurrentSyncGeneration(passState->generation)) {
				return;
			}

			if (passState->weaponAbilityRefreshRequested) {
				RefreshEquippedWeaponAbilities();
				SKSE::log::info("Reward sync: refreshed equipped weapon ability after attack damage sync");
			}

			CompleteRewardSyncRun(passState->generation);
		}

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

		void CompleteCarryWeightQuickResync(std::uint64_t generation) noexcept
		{
			if (!IsCurrentSyncGeneration(generation)) {
				return;
			}

			if (g_carryWeightQuickResyncRerunRequested.exchange(false, std::memory_order_acq_rel)) {
				g_carryWeightQuickResyncScheduledSinceMs.store(NowMs(), std::memory_order_release);
				auto rerunState = std::make_shared<CarryWeightQuickResyncState>();
				rerunState->generation = generation;
				if (QueueMainTask([rerunState]() { RunCarryWeightQuickResync(rerunState); })) {
					return;
				}
				RunCarryWeightQuickResync(rerunState);
				return;
			}

			g_carryWeightQuickResyncScheduled.store(false, std::memory_order_release);
			g_carryWeightQuickResyncScheduledSinceMs.store(0, std::memory_order_release);
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

			const float total = SnapshotRewardTotalForActorValue(RE::ActorValue::kCarryWeight);
			if (std::abs(total) <= kRewardCapEpsilon) {
				return CarryWeightQuickResyncAttemptResult::kDone;
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
				return CarryWeightQuickResyncAttemptResult::kDone;
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
			if (!IsCurrentSyncGeneration(state->generation)) {
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
			while (state->attempt < kCarryWeightQuickResyncMaxAttempts && IsCurrentSyncGeneration(state->generation)) {
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

		[[nodiscard]] RewardSyncPassResult ApplyRewardSyncPass(RewardSyncPassState& passState) noexcept
		{
			RewardSyncPassResult passResult{};

			auto* player = RE::PlayerCharacter::GetSingleton();
			if (!player) {
				return passResult;
			}
			auto* avOwner = player->AsActorValueOwner();
			if (!avOwner) {
				return passResult;
			}

			const auto totals = SnapshotRewardTotals();
			if (totals.empty()) {
				return passResult;
			}
			for (const auto& [av, total] : totals) {
				if (passState.nonConvergingActorValues.contains(av)) {
					continue;
				}
				const auto snapshot = CaptureRewardActorSnapshot(player, avOwner, av, total);
				const float base = snapshot.base;
				const float cur = snapshot.current;
				const float permanent = snapshot.permanent;
				const float permanentModifier = snapshot.permanentModifier;
				float delta = 0.0f;
				float cap = 0.0f;
				const bool hasRewardCap = TryGetRewardCap(av, cap);
				bool forceImmediate = false;
				if (av == RE::ActorValue::kCarryWeight) {
					delta = ComputeCarryWeightSyncDelta(base, cur, permanent, permanentModifier, total);
					// Carry weight desync on load is high-impact in gameplay.
					// Apply immediately when a non-trivial delta is observed.
					forceImmediate = std::abs(delta) > kRewardCapEpsilon;
				} else if (hasRewardCap) {
					delta = ComputeCappedRewardSyncDeltaFromSnapshot(
						base,
						cur,
						permanent,
						permanentModifier,
						total,
						cap,
						kRewardCapEpsilon);
					forceImmediate = cur > (base + cap + kRewardCapEpsilon);
				} else {
					delta = snapshot.delta;
				}
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

				if (!forceImmediate && !ShouldApplyAfterStreak(delta, streak, kRewardSyncMinMissingStreak)) {
					if (std::abs(delta) > kRewardCapEpsilon) {
						++passResult.pendingCount;
					}
					continue;
				}
				streak = 0;

				if (forceImmediate) {
					SKSE::log::warn(
						"Reward sync cap guard: AV {} above cap (base {:.4f}, current {:.4f}, cap {:.2f}); applying {:.4f}",
						static_cast<std::uint32_t>(av),
						base,
						cur,
						cap,
						delta);
				}

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

				if (std::abs(delta) <= kRewardCapEpsilon) {
					continue;
				}

				avOwner->ModActorValue(av, delta);
				++passResult.correctedCount;
				if (av == RE::ActorValue::kAttackDamageMult) {
					passState.weaponAbilityRefreshRequested = true;
				}

				const auto postSnapshot = CaptureRewardActorSnapshot(player, avOwner, av, total);
				float postDelta = 0.0f;
				if (av == RE::ActorValue::kCarryWeight) {
					postDelta = ComputeCarryWeightSyncDelta(
						postSnapshot.base,
						postSnapshot.current,
						postSnapshot.permanent,
						postSnapshot.permanentModifier,
						total);
				} else if (hasRewardCap) {
					postDelta = ComputeCappedRewardSyncDeltaFromSnapshot(
						postSnapshot.base,
						postSnapshot.current,
						postSnapshot.permanent,
						postSnapshot.permanentModifier,
						total,
						cap,
						kRewardCapEpsilon);
				} else {
					postDelta = postSnapshot.delta;
				}

				// Some AVs (e.g. multiplicative channels) can report no observable convergence
				// even after ModActorValue succeeds, which would cause repeated re-application
				// across this pass loop. Stop re-applying that AV within the same run.
				if (std::abs(postDelta) > kRewardCapEpsilon &&
				    std::abs(postDelta - delta) <= kRewardCapEpsilon) {
					passState.nonConvergingActorValues.insert(av);
					SKSE::log::warn(
						"Reward sync guard: AV {} appears non-converging (delta {:.4f}, postDelta {:.4f}); skipping further applies this run",
						static_cast<std::uint32_t>(av),
						delta,
						postDelta);
				}
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

			if (passResult.correctedCount > 0) {
				SKSE::log::info("Reward sync: corrected {} actor value entries", passResult.correctedCount);
			}

			return passResult;
		}

		void RunRewardSyncPasses(std::shared_ptr<RewardSyncPassState> passState, std::uint32_t remainingPasses) noexcept
		{
			if (!passState) {
				return;
			}
			if (!IsCurrentSyncGeneration(passState->generation)) {
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

			if (passState->normalizeCapsOnFirstPass) {
				passState->normalizeCapsOnFirstPass = false;
				MigrateLegacyAttackDamageMultReward(*passState);
				const auto capAdjustments = ClampRewardTotalsInState();
				ApplyRewardCapAdjustmentsToPlayer(capAdjustments);
			}

			const auto passResult = ApplyRewardSyncPass(*passState);

			const auto scheduleRerunIfRequested = [&]() -> bool {
				if (!g_rewardSyncRerunRequested.exchange(false, std::memory_order_acq_rel)) {
					return false;
				}

				g_rewardSyncScheduledSinceMs.store(NowMs(), std::memory_order_release);
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
				if (!IsCurrentSyncGeneration(passState->generation)) {
					return;
				}
				if (!IsRewardSyncEnvironmentReady()) {
					SKSE::log::warn("Reward sync: fallback pass stopped because game is not ready");
					CompleteRewardSyncRun(passState->generation);
					return;
				}
				(void)ApplyRewardSyncPass(*passState);
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
		auto generation = CurrentSyncGeneration();
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
			generation = BumpSyncGenerationAndClearSchedulers();
			SKSE::log::warn("Reward sync watchdog: force restarting stale sync worker (generation {})", generation);
		}

		if (g_rewardSyncScheduled.exchange(true, std::memory_order_acq_rel)) {
			g_rewardSyncRerunRequested.store(true, std::memory_order_release);
			return;
		}

		g_rewardSyncScheduledSinceMs.store(nowMs, std::memory_order_release);
		auto passState = std::make_shared<RewardSyncPassState>();
		passState->generation = generation;
		passState->weaponAbilityRefreshRequested =
			std::abs(SnapshotRewardTotalForActorValue(RE::ActorValue::kAttackDamageMult)) > kRewardCapEpsilon;

		if (QueueMainTask([passState]() { RunRewardSyncPasses(passState, kRewardSyncPassCount); })) {
			return;
		}

		RunRewardSyncPasses(passState, kRewardSyncPassCount);
	}

	void ResetSyncSchedulersForLoad() noexcept
	{
		const auto generation = BumpSyncGenerationAndClearSchedulers();
		SKSE::log::info("Reward sync schedulers reset for load boundary (generation {})", generation);
	}

	void ScheduleCarryWeightQuickResync() noexcept
	{
		auto generation = CurrentSyncGeneration();
		const auto nowMs = NowMs();
		const auto action = DecideSyncRequestAction(
			g_carryWeightQuickResyncScheduled.load(std::memory_order_acquire),
			g_carryWeightQuickResyncScheduledSinceMs.load(std::memory_order_acquire),
			nowMs,
			kCarryWeightQuickResyncStuckMs);

		if (action == SyncRequestAction::kForceRestartAndStart) {
			generation = BumpSyncGenerationAndClearSchedulers();
			SKSE::log::warn(
				"Reward sync (carry weight quick) watchdog: force restarting stale worker (generation {})",
				generation);
		}

		if (g_carryWeightQuickResyncScheduled.exchange(true, std::memory_order_acq_rel)) {
			g_carryWeightQuickResyncRerunRequested.store(true, std::memory_order_release);
			return;
		}
		g_carryWeightQuickResyncScheduledSinceMs.store(nowMs, std::memory_order_release);

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
				const float appliedActorDelta = next - previousTotal;
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
