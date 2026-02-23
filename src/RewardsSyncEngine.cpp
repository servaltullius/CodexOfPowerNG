#include "RewardsSyncEngine.h"

#include "CodexOfPowerNG/RewardCaps.h"
#include "CodexOfPowerNG/RewardsResync.h"
#include "CodexOfPowerNG/RewardsSyncPolicy.h"

#include <RE/Skyrim.h>

#include <RE/T/TESObjectWEAP.h>

#include <SKSE/Logger.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace CodexOfPowerNG::Rewards::Engine
{
	namespace
	{
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
	}

	float SnapshotRewardTotalForActorValue(RE::ActorValue av) noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		const auto it = state.rewardTotals.find(av);
		if (it == state.rewardTotals.end()) {
			return 0.0f;
		}

		const float clamped = ClampRewardTotal(av, it->second);
		const float applied = ActorAppliedRewardTotal(av, clamped);
		return (std::abs(applied) > kRewardCapEpsilon) ? applied : 0.0f;
	}

	void NormalizeRewardCapsOnStateAndPlayer() noexcept
	{
		const auto adjustments = ClampRewardTotalsInState();
		ApplyRewardCapAdjustmentsToPlayer(adjustments);
	}

	std::vector<std::pair<RE::ActorValue, float>> SnapshotRewardTotals() noexcept
	{
		std::vector<std::pair<RE::ActorValue, float>> totals;
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		totals.reserve(state.rewardTotals.size());
		for (const auto& [av, total] : state.rewardTotals) {
			const float clamped = ClampRewardTotal(av, total);
			const float applied = ActorAppliedRewardTotal(av, clamped);
			if (std::abs(applied) > kRewardCapEpsilon) {
				totals.emplace_back(av, applied);
			}
		}
		return totals;
	}

	void PrepareRewardSyncPass(RewardSyncPassState& passState) noexcept
	{
		if (!passState.normalizeCapsOnFirstPass) {
			return;
		}

		passState.normalizeCapsOnFirstPass = false;
		MigrateLegacyAttackDamageMultReward(passState);
		NormalizeRewardCapsOnStateAndPlayer();
	}

	RewardSyncPassResult ApplyRewardSyncPass(RewardSyncPassState& passState, std::uint32_t minMissingStreak) noexcept
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
			bool applyImmediately = false;
			if (av == RE::ActorValue::kCarryWeight) {
				delta = ComputeCarryWeightSyncDelta(base, cur, permanent, permanentModifier, total);
				// Carry weight desync on load is high-impact in gameplay.
				// Apply immediately when a non-trivial delta is observed.
				applyImmediately = std::abs(delta) > kRewardCapEpsilon;
			} else if (hasRewardCap) {
				delta = ComputeCappedRewardSyncDeltaFromSnapshot(
					base,
					cur,
					permanent,
					permanentModifier,
					total,
					cap,
					kRewardCapEpsilon);
			} else {
				delta = snapshot.delta;
			}
			auto& streak = passState.missingStreaks[av];
			streak = NextMissingStreak(delta, streak);
			if (av == RE::ActorValue::kCarryWeight && std::abs(delta) > kRewardCapEpsilon && streak <= minMissingStreak) {
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

			if (!applyImmediately && !ShouldApplyAfterStreak(delta, streak, minMissingStreak)) {
				if (std::abs(delta) > kRewardCapEpsilon) {
					++passResult.pendingCount;
				}
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

	void FinalizeRewardSyncPass(RewardSyncPassState& passState) noexcept
	{
		if (!passState.weaponAbilityRefreshRequested) {
			return;
		}

		RefreshEquippedWeaponAbilities();
		SKSE::log::info("Reward sync: refreshed equipped weapon ability after attack damage sync");
	}
}
