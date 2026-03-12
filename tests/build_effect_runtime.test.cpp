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
		    !NearlyEqual(*LookupTotal(anchorTotals, RE::ActorValue::kAttackDamageMult), 0.11f) ||
		    !LookupTotal(anchorTotals, RE::ActorValue::kWeaponSpeedMult).has_value() ||
		    !NearlyEqual(*LookupTotal(anchorTotals, RE::ActorValue::kWeaponSpeedMult), 0.03f) ||
		    !LookupTotal(anchorTotals, RE::ActorValue::kCriticalChance).has_value() ||
		    !NearlyEqual(*LookupTotal(anchorTotals, RE::ActorValue::kCriticalChance), 3.0f)) {
			return false;
		}

		BuildRuntimeSnapshot expansionSnapshot{};
		expansionSnapshot.attackScore = 30u;
		expansionSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Attack1)] = "build.attack.precision";
		expansionSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Attack2)] = "build.attack.pinpoint";
		expansionSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Wildcard1)] = "build.attack.vitals";

		const auto expansionTotals = ComputeDerivedBuildActorValueTotals(expansionSnapshot);
		return LookupTotal(expansionTotals, RE::ActorValue::kWeaponSpeedMult).has_value() &&
		       NearlyEqual(*LookupTotal(expansionTotals, RE::ActorValue::kWeaponSpeedMult), 0.03f) &&
		       LookupTotal(expansionTotals, RE::ActorValue::kCriticalChance).has_value() &&
		       NearlyEqual(*LookupTotal(expansionTotals, RE::ActorValue::kCriticalChance), 5.0f);
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
		    !NearlyEqual(*LookupTotal(bulwarkTotals, RE::ActorValue::kDamageResist), 33.0f)) {
			return false;
		}

		BuildRuntimeSnapshot enduranceSnapshot{};
		enduranceSnapshot.defenseScore = 30u;
		enduranceSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Defense1)] = "build.defense.endurance";
		const auto enduranceTotals = ComputeDerivedBuildActorValueTotals(enduranceSnapshot);
		return LookupTotal(enduranceTotals, RE::ActorValue::kHealth).has_value() &&
		       NearlyEqual(*LookupTotal(enduranceTotals, RE::ActorValue::kHealth), 15.0f) &&
		       !LookupTotal(enduranceTotals, RE::ActorValue::kBlockPowerModifier).has_value();
	}

	bool CuratedUtilityEffectsResolveToConcreteActorValues()
	{
		BuildRuntimeSnapshot livelihoodSnapshot{};
		livelihoodSnapshot.utilityScore = 30u;
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

		BuildRuntimeSnapshot explorationSnapshot{};
		explorationSnapshot.utilityScore = 30u;
		explorationSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Utility1)] = "build.utility.wayfinder";
		explorationSnapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Wildcard1)] = "build.utility.mobility";

		const auto explorationTotals = ComputeDerivedBuildActorValueTotals(explorationSnapshot);
		return LookupTotal(explorationTotals, RE::ActorValue::kSpeedMult).has_value() &&
		       NearlyEqual(*LookupTotal(explorationTotals, RE::ActorValue::kSpeedMult), 0.045f);
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
	if (!expect(CuratedUtilityEffectsResolveToConcreteActorValues(), "utility effects must resolve to concrete actor values")) {
		return 1;
	}
	if (!expect(OnceOnlyEffectsDoNotDoubleStackAcrossSlots(), "once-only effects must not double stack across slots")) {
		return 1;
	}

	return 0;
}
