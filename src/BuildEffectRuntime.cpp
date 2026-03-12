#include "CodexOfPowerNG/BuildEffectRuntime.h"

#include "CodexOfPowerNG/BuildOptionCatalog.h"
#include "CodexOfPowerNG/RewardCaps.h"
#include "CodexOfPowerNG/SerializationStateStore.h"
#include "CodexOfPowerNG/State.h"

#include <RE/Skyrim.h>

#include <RE/T/TESObjectWEAP.h>

#include <algorithm>
#include <cmath>
#include <mutex>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace CodexOfPowerNG::Builds
{
	namespace
	{
		std::mutex g_runtimeMutex;

		[[nodiscard]] const BuildOptionDef* FindOption(std::string_view optionId) noexcept
		{
			for (const auto& option : GetBuildOptionCatalog()) {
				if (option.id == optionId) {
					return &option;
				}
			}
			return nullptr;
		}

		[[nodiscard]] BuildSlotKind SlotKindForId(BuildSlotId slotId) noexcept
		{
			switch (slotId) {
			case BuildSlotId::Attack1:
			case BuildSlotId::Attack2:
				return BuildSlotKind::Attack;
			case BuildSlotId::Defense1:
				return BuildSlotKind::Defense;
			case BuildSlotId::Utility1:
			case BuildSlotId::Utility2:
				return BuildSlotKind::Utility;
			case BuildSlotId::Wildcard1:
				return BuildSlotKind::Wildcard;
			}

			return BuildSlotKind::Wildcard;
		}

		[[nodiscard]] bool IsSlotCompatible(const BuildOptionDef& option, BuildSlotId slotId) noexcept
		{
			const auto slotKind = SlotKindForId(slotId);
			if (slotKind == BuildSlotKind::Wildcard) {
				return option.slotCompatibility == BuildSlotCompatibility::WildcardOnly ||
				       option.slotCompatibility == BuildSlotCompatibility::SameOrWildcard;
			}

			const bool matchesDiscipline =
				(slotKind == BuildSlotKind::Attack && option.discipline == BuildDiscipline::Attack) ||
				(slotKind == BuildSlotKind::Defense && option.discipline == BuildDiscipline::Defense) ||
				(slotKind == BuildSlotKind::Utility && option.discipline == BuildDiscipline::Utility);
			if (!matchesDiscipline) {
				return false;
			}

			return option.slotCompatibility == BuildSlotCompatibility::SameDisciplineOnly ||
			       option.slotCompatibility == BuildSlotCompatibility::SameOrWildcard;
		}

		[[nodiscard]] std::uint32_t ScoreForDiscipline(
			const BuildRuntimeSnapshot& snapshot,
			BuildDiscipline             discipline) noexcept
		{
			switch (discipline) {
			case BuildDiscipline::Attack:
				return snapshot.attackScore;
			case BuildDiscipline::Defense:
				return snapshot.defenseScore;
			case BuildDiscipline::Utility:
				return snapshot.utilityScore;
			}
			return 0u;
		}

		[[nodiscard]] std::optional<float> ToFloatMagnitude(const BuildMagnitude& magnitude) noexcept
		{
			if (std::holds_alternative<float>(magnitude)) {
				return std::get<float>(magnitude);
			}
			return std::nullopt;
		}

		[[nodiscard]] std::optional<std::int32_t> ToIntMagnitude(const BuildMagnitude& magnitude) noexcept
		{
			if (std::holds_alternative<std::int32_t>(magnitude)) {
				return std::get<std::int32_t>(magnitude);
			}
			return std::nullopt;
		}

		[[nodiscard]] std::optional<std::pair<RE::ActorValue, float>> ResolveActorValueDelta(
			BuildEffectType        effectType,
			std::string_view       effectKey,
			const BuildMagnitude& magnitude) noexcept
		{
			switch (effectType) {
			case BuildEffectType::ActorValue: {
				const auto amount = ToFloatMagnitude(magnitude);
				if (!amount.has_value()) {
					return std::nullopt;
				}
				if (effectKey == "attack_damage_mult") {
					return std::pair{ RE::ActorValue::kAttackDamageMult, *amount / 100.0f };
				}
				if (effectKey == "weapon_speed_mult") {
					return std::pair{ RE::ActorValue::kWeaponSpeedMult, *amount / 100.0f };
				}
				if (effectKey == "critical_chance") {
					return std::pair{ RE::ActorValue::kCriticalChance, *amount };
				}
				if (effectKey == "unarmed_damage") {
					return std::pair{ RE::ActorValue::kUnarmedDamage, *amount };
				}
				if (effectKey == "stamina") {
					return std::pair{ RE::ActorValue::kStamina, *amount };
				}
				if (effectKey == "stamina_rate") {
					return std::pair{ RE::ActorValue::kStaminaRate, *amount };
				}
				if (effectKey == "destruction_modifier") {
					return std::pair{ RE::ActorValue::kDestructionModifier, *amount };
				}
				if (effectKey == "damage_resist") {
					return std::pair{ RE::ActorValue::kDamageResist, *amount };
				}
				if (effectKey == "block_power_modifier") {
					return std::pair{ RE::ActorValue::kBlockPowerModifier, *amount };
				}
				if (effectKey == "health") {
					return std::pair{ RE::ActorValue::kHealth, *amount };
				}
				if (effectKey == "heal_rate") {
					return std::pair{ RE::ActorValue::kHealRate, *amount };
				}
				if (effectKey == "restoration_modifier") {
					return std::pair{ RE::ActorValue::kRestorationModifier, *amount };
				}
				if (effectKey == "reflect_damage") {
					return std::pair{ RE::ActorValue::kReflectDamage, *amount };
				}
				if (effectKey == "alteration_modifier") {
					return std::pair{ RE::ActorValue::kAlterationModifier, *amount };
				}
				if (effectKey == "magic_resist") {
					return std::pair{ RE::ActorValue::kResistMagic, *amount };
				}
				if (effectKey == "fire_resist") {
					return std::pair{ RE::ActorValue::kResistFire, *amount };
				}
				if (effectKey == "frost_resist") {
					return std::pair{ RE::ActorValue::kResistFrost, *amount };
				}
				if (effectKey == "shock_resist") {
					return std::pair{ RE::ActorValue::kResistShock, *amount };
				}
				if (effectKey == "poison_resist") {
					return std::pair{ RE::ActorValue::kPoisonResist, *amount };
				}
				if (effectKey == "disease_resist") {
					return std::pair{ RE::ActorValue::kResistDisease, *amount };
				}
				if (effectKey == "absorb_chance") {
					return std::pair{ RE::ActorValue::kAbsorbChance, *amount };
				}
				if (effectKey == "smithing_modifier") {
					return std::pair{ RE::ActorValue::kSmithingModifier, *amount };
				}
				if (effectKey == "alchemy_modifier") {
					return std::pair{ RE::ActorValue::kAlchemyModifier, *amount };
				}
				if (effectKey == "enchanting_modifier") {
					return std::pair{ RE::ActorValue::kEnchantingModifier, *amount };
				}
				if (effectKey == "magicka") {
					return std::pair{ RE::ActorValue::kMagicka, *amount };
				}
				if (effectKey == "magicka_rate") {
					return std::pair{ RE::ActorValue::kMagickaRate, *amount };
				}
				if (effectKey == "lockpicking_modifier") {
					return std::pair{ RE::ActorValue::kLockpickingModifier, *amount };
				}
				if (effectKey == "pickpocket_modifier") {
					return std::pair{ RE::ActorValue::kPickpocketModifier, *amount };
				}
				if (effectKey == "sneaking_modifier") {
					return std::pair{ RE::ActorValue::kSneakingModifier, *amount };
				}
				if (effectKey == "conjuration_modifier") {
					return std::pair{ RE::ActorValue::kConjurationModifier, *amount };
				}
				if (effectKey == "illusion_modifier") {
					return std::pair{ RE::ActorValue::kIllusionModifier, *amount };
				}
				if (effectKey == "speed_mult") {
					return std::pair{ RE::ActorValue::kSpeedMult, *amount / 100.0f };
				}
				if (effectKey == "shout_recovery_mult") {
					return std::pair{ RE::ActorValue::kShoutRecoveryMult, *amount };
				}
				return std::nullopt;
			}
			case BuildEffectType::CarryWeight: {
				const auto amount = ToFloatMagnitude(magnitude);
				if (!amount.has_value() || effectKey != "carry_weight") {
					return std::nullopt;
				}
				return std::pair{ RE::ActorValue::kCarryWeight, *amount };
			}
			case BuildEffectType::Economy: {
				const auto amount = ToIntMagnitude(magnitude);
				if (!amount.has_value() || effectKey != "speechcraft_modifier") {
					return std::nullopt;
				}
				// Keep build-facing economy magnitudes in semantic "% favorable" units while
				// mapping to the legacy speechcraft modifier tuning already used by rewards.
				return std::pair{ RE::ActorValue::kSpeechcraftModifier, static_cast<float>(*amount) * 0.05f };
			}
			case BuildEffectType::UtilityFlag:
				return std::nullopt;
			}

			return std::nullopt;
		}

		void AccumulateEffect(
			std::unordered_map<RE::ActorValue, float, ActorValueHash>& totals,
			BuildEffectType                                           effectType,
			std::string_view                                          effectKey,
			const BuildMagnitude&                                     magnitude) noexcept
		{
			const auto resolved = ResolveActorValueDelta(effectType, effectKey, magnitude);
			if (!resolved.has_value()) {
				return;
			}

			auto& total = totals[resolved->first];
			total += resolved->second;
		}

		[[nodiscard]] BuildRuntimeSnapshot SnapshotCurrentBuildRuntime() noexcept
		{
			const auto state = SerializationStateStore::SnapshotState();
			BuildRuntimeSnapshot snapshot{};
			snapshot.attackScore = state.attackScore;
			snapshot.defenseScore = state.defenseScore;
			snapshot.utilityScore = state.utilityScore;
			snapshot.activeBuildSlots = state.activeBuildSlots;
			return snapshot;
		}

		[[nodiscard]] std::unordered_map<RE::ActorValue, float, ActorValueHash> SnapshotAppliedBuildEffectTotals() noexcept
		{
			auto& state = GetState();
			std::scoped_lock lock(state.mutex);
			return state.buildAppliedEffectTotals;
		}

		void ReplaceAppliedBuildEffectTotals(
			std::unordered_map<RE::ActorValue, float, ActorValueHash> totals) noexcept
		{
			auto& state = GetState();
			std::scoped_lock lock(state.mutex);
			state.buildAppliedEffectTotals = std::move(totals);
		}

		void ClearAppliedBuildEffectTotals() noexcept
		{
			auto& state = GetState();
			std::scoped_lock lock(state.mutex);
			state.buildAppliedEffectTotals.clear();
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
			if (!equippedForm || !equippedForm->As<RE::TESObjectWEAP>()) {
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
	}

	std::vector<std::pair<RE::ActorValue, float>> ComputeDerivedBuildActorValueTotals(
		const BuildRuntimeSnapshot& snapshot) noexcept
	{
		std::unordered_map<RE::ActorValue, float, ActorValueHash> totals;

		for (const auto& milestone : GetBuildBaselineMilestones()) {
			if (ScoreForDiscipline(snapshot, milestone.discipline) < milestone.threshold) {
				continue;
			}
			AccumulateEffect(totals, milestone.effectType, milestone.effectKey, milestone.magnitude);
		}

		std::unordered_set<std::string_view> appliedOptionIds;
		for (const auto slotId : GetInitialBuildSlotLayout()) {
			const auto& optionId = snapshot.activeBuildSlots[ToIndex(slotId)];
			if (optionId.empty()) {
				continue;
			}

			const auto* option = FindOption(optionId);
			if (!option) {
				continue;
			}
			if (ScoreForDiscipline(snapshot, option->discipline) < option->unlockScore) {
				continue;
			}
			if (!IsSlotCompatible(*option, slotId)) {
				continue;
			}
			if (option->stackRule == BuildStackRule::OnceOnly && !appliedOptionIds.insert(option->id).second) {
				continue;
			}

			AccumulateEffect(totals, option->effectType, option->effectKey, option->magnitude);
		}

		std::vector<std::pair<RE::ActorValue, float>> out;
		out.reserve(totals.size());
		for (const auto& [av, total] : totals) {
			if (std::abs(total) <= Rewards::kRewardCapEpsilon) {
				continue;
			}
			out.emplace_back(av, total);
		}

		std::sort(
			out.begin(),
			out.end(),
			[](const auto& lhs, const auto& rhs) {
				return static_cast<std::uint32_t>(lhs.first) < static_cast<std::uint32_t>(rhs.first);
			});
		return out;
	}

	float ClampBuildSyncDeltaForActorValue(
		RE::ActorValue av,
		float          currentActorValue,
		float          desiredDelta) noexcept
	{
		if (!std::isfinite(currentActorValue) || !std::isfinite(desiredDelta)) {
			return 0.0f;
		}

		if (av != RE::ActorValue::kShoutRecoveryMult || desiredDelta >= 0.0f) {
			return desiredDelta;
		}

		constexpr float kShoutRecoveryMinValue = 0.30f;
		if (currentActorValue <= (kShoutRecoveryMinValue + Rewards::kRewardCapEpsilon)) {
			return 0.0f;
		}
		if ((currentActorValue + desiredDelta) < kShoutRecoveryMinValue) {
			return kShoutRecoveryMinValue - currentActorValue;
		}
		return desiredDelta;
	}

	void SyncCurrentBuildEffectsToPlayer() noexcept
	{
		auto* player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			return;
		}
		auto* avOwner = player->AsActorValueOwner();
		if (!avOwner) {
			return;
		}

		const auto desiredList = ComputeDerivedBuildActorValueTotals(SnapshotCurrentBuildRuntime());
		std::unordered_map<RE::ActorValue, float, ActorValueHash> desiredTotals;
		for (const auto& [av, total] : desiredList) {
			desiredTotals.emplace(av, total);
		}

		bool refreshWeaponAbilities = false;
		{
			std::scoped_lock lock(g_runtimeMutex);
			const auto currentAppliedTotals = SnapshotAppliedBuildEffectTotals();
			auto nextAppliedTotals = currentAppliedTotals;
			std::unordered_set<RE::ActorValue, ActorValueHash> actorValues;
			for (const auto& [av, _] : currentAppliedTotals) {
				actorValues.insert(av);
			}
			for (const auto& [av, _] : desiredTotals) {
				actorValues.insert(av);
			}

			for (const auto av : actorValues) {
				const float currentTotal = currentAppliedTotals.contains(av) ? currentAppliedTotals.at(av) : 0.0f;
				const float desiredTotal = desiredTotals.contains(av) ? desiredTotals[av] : 0.0f;
				const float desiredDelta = desiredTotal - currentTotal;
				const float currentActorValue = avOwner->GetActorValue(av);
				const float delta = ClampBuildSyncDeltaForActorValue(av, currentActorValue, desiredDelta);
				if (std::abs(delta) <= Rewards::kRewardCapEpsilon) {
					if (std::abs(currentTotal) <= Rewards::kRewardCapEpsilon) {
						nextAppliedTotals.erase(av);
					} else {
						nextAppliedTotals[av] = currentTotal;
					}
					continue;
				}

				avOwner->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kPermanent, av, delta);
				const float appliedTotal = currentTotal + delta;
				if (std::abs(appliedTotal) <= Rewards::kRewardCapEpsilon) {
					nextAppliedTotals.erase(av);
				} else {
					nextAppliedTotals[av] = appliedTotal;
				}
				if (av == RE::ActorValue::kAttackDamageMult || av == RE::ActorValue::kWeaponSpeedMult) {
					refreshWeaponAbilities = true;
				}
			}
			ReplaceAppliedBuildEffectTotals(std::move(nextAppliedTotals));
		}

		if (refreshWeaponAbilities) {
			RefreshEquippedWeaponAbilities();
		}
	}

	void ResetForLoad() noexcept
	{
		std::scoped_lock lock(g_runtimeMutex);
		ClearAppliedBuildEffectTotals();
	}
}
