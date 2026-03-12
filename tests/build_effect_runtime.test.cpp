#include "CodexOfPowerNG/BuildEffectRuntime.h"

#include <cmath>
#include <iostream>
#include <optional>
#include <vector>

namespace
{
	using CodexOfPowerNG::Builds::BuildRuntimeSnapshot;
	using CodexOfPowerNG::Builds::BuildSlotId;
	using CodexOfPowerNG::Builds::ComputeDerivedBuildActorValueTotals;

	[[nodiscard]] bool NearlyEqual(float lhs, float rhs, float epsilon = 0.0001f) noexcept
	{
		return std::abs(lhs - rhs) <= epsilon;
	}

	[[nodiscard]] std::optional<float> LookupTotal(
		const std::vector<std::pair<RE::ActorValue, float>>& totals,
		RE::ActorValue av) noexcept
	{
		for (const auto& [entryAv, total] : totals) {
			if (entryAv == av) {
				return total;
			}
		}
		return std::nullopt;
	}

	bool CuratedAttackEffectsResolveToConcreteActorValues()
	{
		BuildRuntimeSnapshot anchorSnapshot{};
		anchorSnapshot.attackScore = 30u;
		anchorSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Attack1)] = "build.attack.ferocity";
		anchorSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Attack2)] = "build.attack.precision";
		anchorSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Wildcard1)] = "build.attack.vitals";

		const auto anchorTotals = ComputeDerivedBuildActorValueTotals(anchorSnapshot);
		if (!LookupTotal(anchorTotals, RE::ActorValue::kAttackDamageMult).has_value() ||
		    !NearlyEqual(*LookupTotal(anchorTotals, RE::ActorValue::kAttackDamageMult), 0.14f) ||
		    !LookupTotal(anchorTotals, RE::ActorValue::kWeaponSpeedMult).has_value() ||
		    !NearlyEqual(*LookupTotal(anchorTotals, RE::ActorValue::kWeaponSpeedMult), 0.03f) ||
		    LookupTotal(anchorTotals, RE::ActorValue::kCriticalChance).has_value()) {
			return false;
		}

		BuildRuntimeSnapshot expansionSnapshot{};
		expansionSnapshot.attackScore = 30u;
		expansionSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Attack1)] = "build.attack.precision";
		expansionSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Attack2)] = "build.attack.pinpoint";
		expansionSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Wildcard1)] = "build.attack.vitals";

		const auto expansionTotals = ComputeDerivedBuildActorValueTotals(expansionSnapshot);
		if (!LookupTotal(expansionTotals, RE::ActorValue::kAttackDamageMult).has_value() ||
		    !NearlyEqual(*LookupTotal(expansionTotals, RE::ActorValue::kAttackDamageMult), 0.09f) ||
		    !LookupTotal(expansionTotals, RE::ActorValue::kWeaponSpeedMult).has_value() ||
		    !NearlyEqual(*LookupTotal(expansionTotals, RE::ActorValue::kWeaponSpeedMult), 0.03f) ||
		    !LookupTotal(expansionTotals, RE::ActorValue::kCriticalChance).has_value() ||
		    !NearlyEqual(*LookupTotal(expansionTotals, RE::ActorValue::kCriticalChance), 2.0f)) {
			return false;
		}

		BuildRuntimeSnapshot furySnapshot{};
		furySnapshot.attackScore = 30u;
		furySnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Attack1)] = "build.attack.reserve";
		furySnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Attack2)] = "build.attack.secondwind";
		const auto furyTotals = ComputeDerivedBuildActorValueTotals(furySnapshot);
		if (!LookupTotal(furyTotals, RE::ActorValue::kStamina).has_value() ||
		    !NearlyEqual(*LookupTotal(furyTotals, RE::ActorValue::kStamina), 2.0f) ||
		    !LookupTotal(furyTotals, RE::ActorValue::kStaminaRate).has_value() ||
		    !NearlyEqual(*LookupTotal(furyTotals, RE::ActorValue::kStaminaRate), 0.1f)) {
			return false;
		}

		BuildRuntimeSnapshot devastationSnapshot{};
		devastationSnapshot.attackScore = 30u;
		devastationSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Attack1)] = "build.attack.brawler";
		devastationSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Wildcard1)] = "build.attack.destruction";
		const auto devastationTotals = ComputeDerivedBuildActorValueTotals(devastationSnapshot);
		return LookupTotal(devastationTotals, RE::ActorValue::kUnarmedDamage).has_value() &&
		       NearlyEqual(*LookupTotal(devastationTotals, RE::ActorValue::kUnarmedDamage), 0.2f) &&
		       LookupTotal(devastationTotals, RE::ActorValue::kDestructionModifier).has_value() &&
		       NearlyEqual(*LookupTotal(devastationTotals, RE::ActorValue::kDestructionModifier), 0.5f);
	}

	bool CuratedDefenseEffectsResolveToConcreteActorValues()
	{
		BuildRuntimeSnapshot snapshot{};
		snapshot.defenseScore = 30u;
		snapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Defense1)] = "build.defense.guard";
		snapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Wildcard1)] = "build.defense.bastion";

		const auto guardTotals = ComputeDerivedBuildActorValueTotals(snapshot);
		if (!LookupTotal(guardTotals, RE::ActorValue::kDamageResist).has_value() ||
		    !NearlyEqual(*LookupTotal(guardTotals, RE::ActorValue::kDamageResist), 25.0f) ||
		    !LookupTotal(guardTotals, RE::ActorValue::kBlockPowerModifier).has_value() ||
		    !NearlyEqual(*LookupTotal(guardTotals, RE::ActorValue::kBlockPowerModifier), 15.0f)) {
			return false;
		}

		BuildRuntimeSnapshot bulwarkSnapshot{};
		bulwarkSnapshot.defenseScore = 30u;
		bulwarkSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Defense1)] = "build.defense.guard";
		bulwarkSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Wildcard1)] = "build.defense.bulwark";
		const auto bulwarkTotals = ComputeDerivedBuildActorValueTotals(bulwarkSnapshot);
		if (!LookupTotal(bulwarkTotals, RE::ActorValue::kDamageResist).has_value() ||
		    !NearlyEqual(*LookupTotal(bulwarkTotals, RE::ActorValue::kDamageResist), 25.0f) ||
		    !LookupTotal(bulwarkTotals, RE::ActorValue::kHealth).has_value() ||
		    !NearlyEqual(*LookupTotal(bulwarkTotals, RE::ActorValue::kHealth), 8.0f)) {
			return false;
		}

		BuildRuntimeSnapshot enduranceSnapshot{};
		enduranceSnapshot.defenseScore = 30u;
		enduranceSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Defense1)] = "build.defense.endurance";
		const auto enduranceTotals = ComputeDerivedBuildActorValueTotals(enduranceSnapshot);
		if (!LookupTotal(enduranceTotals, RE::ActorValue::kHealth).has_value() ||
		    !NearlyEqual(*LookupTotal(enduranceTotals, RE::ActorValue::kHealth), 15.0f) ||
		    LookupTotal(enduranceTotals, RE::ActorValue::kBlockPowerModifier).has_value()) {
			return false;
		}

		BuildRuntimeSnapshot sustainSnapshot{};
		sustainSnapshot.defenseScore = 35u;
		sustainSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Defense1)] = "build.defense.recovery";
		sustainSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Wildcard1)] = "build.defense.restoration";
		const auto sustainTotals = ComputeDerivedBuildActorValueTotals(sustainSnapshot);
		if (!LookupTotal(sustainTotals, RE::ActorValue::kHealRate).has_value() ||
		    !NearlyEqual(*LookupTotal(sustainTotals, RE::ActorValue::kHealRate), 0.02f) ||
		    !LookupTotal(sustainTotals, RE::ActorValue::kRestorationModifier).has_value() ||
		    !NearlyEqual(*LookupTotal(sustainTotals, RE::ActorValue::kRestorationModifier), 0.5f)) {
			return false;
		}

		BuildRuntimeSnapshot bastionSpecialSnapshot{};
		bastionSpecialSnapshot.defenseScore = 30u;
		bastionSpecialSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Defense1)] = "build.defense.reprisal";
		bastionSpecialSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Wildcard1)] = "build.defense.alteration";
		const auto bastionSpecialTotals = ComputeDerivedBuildActorValueTotals(bastionSpecialSnapshot);
		return LookupTotal(bastionSpecialTotals, RE::ActorValue::kReflectDamage).has_value() &&
		       NearlyEqual(*LookupTotal(bastionSpecialTotals, RE::ActorValue::kReflectDamage), 0.3f) &&
		       LookupTotal(bastionSpecialTotals, RE::ActorValue::kAlterationModifier).has_value() &&
		       NearlyEqual(*LookupTotal(bastionSpecialTotals, RE::ActorValue::kAlterationModifier), 0.5f);
	}

	bool CuratedResistanceEffectsResolveToConcreteActorValues()
	{
		BuildRuntimeSnapshot wardingSnapshot{};
		wardingSnapshot.defenseScore = 35u;
		wardingSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Defense1)] = "build.defense.warding";
		wardingSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Wildcard1)] = "build.defense.absorption";
		const auto wardingTotals = ComputeDerivedBuildActorValueTotals(wardingSnapshot);
		if (!LookupTotal(wardingTotals, RE::ActorValue::kResistMagic).has_value() ||
		    !NearlyEqual(*LookupTotal(wardingTotals, RE::ActorValue::kResistMagic), 0.75f) ||
		    !LookupTotal(wardingTotals, RE::ActorValue::kAbsorbChance).has_value() ||
		    !NearlyEqual(*LookupTotal(wardingTotals, RE::ActorValue::kAbsorbChance), 0.4f)) {
			return false;
		}

		BuildRuntimeSnapshot elementalSnapshot{};
		elementalSnapshot.defenseScore = 35u;
		elementalSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Defense1)] = "build.defense.fireward";
		elementalSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Wildcard1)] = "build.defense.frostward";
		const auto elementalTotals = ComputeDerivedBuildActorValueTotals(elementalSnapshot);
		if (!LookupTotal(elementalTotals, RE::ActorValue::kResistFire).has_value() ||
		    !NearlyEqual(*LookupTotal(elementalTotals, RE::ActorValue::kResistFire), 1.0f) ||
		    !LookupTotal(elementalTotals, RE::ActorValue::kResistFrost).has_value() ||
		    !NearlyEqual(*LookupTotal(elementalTotals, RE::ActorValue::kResistFrost), 1.0f)) {
			return false;
		}

		BuildRuntimeSnapshot statusSnapshot{};
		statusSnapshot.defenseScore = 35u;
		statusSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Defense1)] = "build.defense.stormward";
		statusSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Wildcard1)] = "build.defense.antidote";
		const auto statusTotals = ComputeDerivedBuildActorValueTotals(statusSnapshot);
		if (!LookupTotal(statusTotals, RE::ActorValue::kResistShock).has_value() ||
		    !NearlyEqual(*LookupTotal(statusTotals, RE::ActorValue::kResistShock), 1.0f) ||
		    !LookupTotal(statusTotals, RE::ActorValue::kPoisonResist).has_value() ||
		    !NearlyEqual(*LookupTotal(statusTotals, RE::ActorValue::kPoisonResist), 1.0f)) {
			return false;
		}

		BuildRuntimeSnapshot puritySnapshot{};
		puritySnapshot.defenseScore = 35u;
		puritySnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Defense1)] = "build.defense.purity";
		const auto purityTotals = ComputeDerivedBuildActorValueTotals(puritySnapshot);
		return LookupTotal(purityTotals, RE::ActorValue::kResistDisease).has_value() &&
		       NearlyEqual(*LookupTotal(purityTotals, RE::ActorValue::kResistDisease), 1.0f);
	}

	bool CuratedUtilityEffectsResolveToConcreteActorValues()
	{
		BuildRuntimeSnapshot livelihoodSnapshot{};
		livelihoodSnapshot.utilityScore = 35u;
		livelihoodSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Utility1)] = "build.utility.cache";
		livelihoodSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Utility2)] = "build.utility.barter";
		livelihoodSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Wildcard1)] = "build.utility.hauler";

		const auto livelihoodTotals = ComputeDerivedBuildActorValueTotals(livelihoodSnapshot);
		if (!LookupTotal(livelihoodTotals, RE::ActorValue::kCarryWeight).has_value() ||
		    !NearlyEqual(*LookupTotal(livelihoodTotals, RE::ActorValue::kCarryWeight), 55.0f) ||
		    !LookupTotal(livelihoodTotals, RE::ActorValue::kSpeechcraftModifier).has_value() ||
		    !NearlyEqual(*LookupTotal(livelihoodTotals, RE::ActorValue::kSpeechcraftModifier), 0.75f)) {
			return false;
		}

		BuildRuntimeSnapshot craftingSnapshot{};
		craftingSnapshot.utilityScore = 35u;
		craftingSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Utility1)] = "build.utility.smithing";
		craftingSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Utility2)] = "build.utility.alchemy";
		craftingSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Wildcard1)] = "build.utility.enchanting";
		const auto craftingTotals = ComputeDerivedBuildActorValueTotals(craftingSnapshot);
		if (!LookupTotal(craftingTotals, RE::ActorValue::kSmithingModifier).has_value() ||
		    !NearlyEqual(*LookupTotal(craftingTotals, RE::ActorValue::kSmithingModifier), 0.5f) ||
		    !LookupTotal(craftingTotals, RE::ActorValue::kAlchemyModifier).has_value() ||
		    !NearlyEqual(*LookupTotal(craftingTotals, RE::ActorValue::kAlchemyModifier), 0.5f) ||
		    !LookupTotal(craftingTotals, RE::ActorValue::kEnchantingModifier).has_value() ||
		    !NearlyEqual(*LookupTotal(craftingTotals, RE::ActorValue::kEnchantingModifier), 0.5f)) {
			return false;
		}

		BuildRuntimeSnapshot explorationSnapshot{};
		explorationSnapshot.utilityScore = 30u;
		explorationSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Utility1)] = "build.utility.wayfinder";
		explorationSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Wildcard1)] = "build.utility.mobility";

		const auto explorationTotals = ComputeDerivedBuildActorValueTotals(explorationSnapshot);
		if (!LookupTotal(explorationTotals, RE::ActorValue::kSpeedMult).has_value() ||
		    !NearlyEqual(*LookupTotal(explorationTotals, RE::ActorValue::kSpeedMult), 0.045f)) {
			return false;
		}

		BuildRuntimeSnapshot trickerySnapshot{};
		trickerySnapshot.utilityScore = 30u;
		trickerySnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Utility1)] = "build.utility.sneak";
		trickerySnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Utility2)] = "build.utility.lockpicking";
		trickerySnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Wildcard1)] = "build.utility.conjuration";
		const auto trickeryTotals = ComputeDerivedBuildActorValueTotals(trickerySnapshot);
		if (!LookupTotal(trickeryTotals, RE::ActorValue::kSneakingModifier).has_value() ||
		    !NearlyEqual(*LookupTotal(trickeryTotals, RE::ActorValue::kSneakingModifier), 0.5f) ||
		    !LookupTotal(trickeryTotals, RE::ActorValue::kLockpickingModifier).has_value() ||
		    !NearlyEqual(*LookupTotal(trickeryTotals, RE::ActorValue::kLockpickingModifier), 0.5f) ||
		    !LookupTotal(trickeryTotals, RE::ActorValue::kConjurationModifier).has_value() ||
		    !NearlyEqual(*LookupTotal(trickeryTotals, RE::ActorValue::kConjurationModifier), 0.5f)) {
			return false;
		}

		BuildRuntimeSnapshot trickerySpecialSnapshot{};
		trickerySpecialSnapshot.utilityScore = 30u;
		trickerySpecialSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Utility1)] = "build.utility.pickpocket";
		trickerySpecialSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Wildcard1)] = "build.utility.illusion";
		const auto trickerySpecialTotals = ComputeDerivedBuildActorValueTotals(trickerySpecialSnapshot);
		if (!LookupTotal(trickerySpecialTotals, RE::ActorValue::kPickpocketModifier).has_value() ||
		    !NearlyEqual(*LookupTotal(trickerySpecialTotals, RE::ActorValue::kPickpocketModifier), 0.5f) ||
		    !LookupTotal(trickerySpecialTotals, RE::ActorValue::kIllusionModifier).has_value() ||
		    !NearlyEqual(*LookupTotal(trickerySpecialTotals, RE::ActorValue::kIllusionModifier), 0.5f)) {
			return false;
		}

		BuildRuntimeSnapshot livelihoodResourceSnapshot{};
		livelihoodResourceSnapshot.utilityScore = 35u;
		livelihoodResourceSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Utility1)] = "build.utility.magicka";
		livelihoodResourceSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Utility2)] = "build.utility.meditation";
		const auto livelihoodResourceTotals = ComputeDerivedBuildActorValueTotals(livelihoodResourceSnapshot);
		if (!LookupTotal(livelihoodResourceTotals, RE::ActorValue::kMagicka).has_value() ||
		    !NearlyEqual(*LookupTotal(livelihoodResourceTotals, RE::ActorValue::kMagicka), 2.0f) ||
		    !LookupTotal(livelihoodResourceTotals, RE::ActorValue::kMagickaRate).has_value() ||
		    !NearlyEqual(*LookupTotal(livelihoodResourceTotals, RE::ActorValue::kMagickaRate), 0.1f)) {
			return false;
		}

		BuildRuntimeSnapshot explorationSpecialSnapshot{};
		explorationSpecialSnapshot.utilityScore = 35u;
		explorationSpecialSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Utility1)] = "build.utility.echo";
		const auto explorationSpecialTotals = ComputeDerivedBuildActorValueTotals(explorationSpecialSnapshot);
		return LookupTotal(explorationSpecialTotals, RE::ActorValue::kShoutRecoveryMult).has_value() &&
		       NearlyEqual(*LookupTotal(explorationSpecialTotals, RE::ActorValue::kShoutRecoveryMult), -0.02f);
	}

	bool OnceOnlyEffectsDoNotDoubleStackAcrossSlots()
	{
		BuildRuntimeSnapshot snapshot{};
		snapshot.attackScore = 30u;
		snapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Attack1)] = "build.attack.ferocity";
		snapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Wildcard1)] = "build.attack.ferocity";

		const auto totals = ComputeDerivedBuildActorValueTotals(snapshot);
		return LookupTotal(totals, RE::ActorValue::kAttackDamageMult).has_value() &&
		       NearlyEqual(*LookupTotal(totals, RE::ActorValue::kAttackDamageMult), 0.11f);
	}

	bool BuildSyncClampHonorsShoutRecoveryFloor()
	{
		const float blockedDelta =
			CodexOfPowerNG::Builds::ClampBuildSyncDeltaForActorValue(
				RE::ActorValue::kShoutRecoveryMult,
				0.30f,
				-0.02f);
		if (!NearlyEqual(blockedDelta, 0.0f)) {
			return false;
		}

		const float clampedDelta =
			CodexOfPowerNG::Builds::ClampBuildSyncDeltaForActorValue(
				RE::ActorValue::kShoutRecoveryMult,
				0.31f,
				-0.02f);
		if (!NearlyEqual(clampedDelta, -0.01f)) {
			return false;
		}

		const float passthroughDelta =
			CodexOfPowerNG::Builds::ClampBuildSyncDeltaForActorValue(
				RE::ActorValue::kShoutRecoveryMult,
				0.50f,
				-0.02f);
		if (!NearlyEqual(passthroughDelta, -0.02f)) {
			return false;
		}

		const float positiveDelta =
			CodexOfPowerNG::Builds::ClampBuildSyncDeltaForActorValue(
				RE::ActorValue::kShoutRecoveryMult,
				0.30f,
				0.05f);
		return NearlyEqual(positiveDelta, 0.05f);
	}
}

int main()
{
	const auto expect = [](bool condition, const char* message) {
		if (!condition) {
			std::cerr << "build_effect_runtime: " << message << '\n';
			return false;
		}
		return true;
	};

	if (!expect(CuratedAttackEffectsResolveToConcreteActorValues(), "attack effects must resolve to concrete actor values")) {
		return 1;
	}
	if (!expect(CuratedDefenseEffectsResolveToConcreteActorValues(), "defense effects must resolve to concrete actor values")) {
		return 1;
	}
	if (!expect(CuratedResistanceEffectsResolveToConcreteActorValues(), "resistance effects must resolve to concrete actor values")) {
		return 1;
	}
	if (!expect(CuratedUtilityEffectsResolveToConcreteActorValues(), "utility effects must resolve to concrete actor values")) {
		return 1;
	}
	if (!expect(OnceOnlyEffectsDoNotDoubleStackAcrossSlots(), "once-only effects must not double stack across slots")) {
		return 1;
	}
	if (!expect(BuildSyncClampHonorsShoutRecoveryFloor(), "build sync must honor the shout recovery floor")) {
		return 1;
	}

	return 0;
}
