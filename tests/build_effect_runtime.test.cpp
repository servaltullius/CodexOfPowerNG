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
		BuildRuntimeSnapshot snapshot{};
		snapshot.attackScore = 30u;
		snapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Attack1)] = "build.attack.ferocity";
		snapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Attack2)] = "build.attack.precision";
		snapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Wildcard1)] = "build.attack.vitals";

		const auto totals = ComputeDerivedBuildActorValueTotals(snapshot);
		return LookupTotal(totals, RE::ActorValue::kAttackDamageMult).has_value() &&
		       NearlyEqual(*LookupTotal(totals, RE::ActorValue::kAttackDamageMult), 0.11f) &&
		       LookupTotal(totals, RE::ActorValue::kWeaponSpeedMult).has_value() &&
		       NearlyEqual(*LookupTotal(totals, RE::ActorValue::kWeaponSpeedMult), 0.03f) &&
		       LookupTotal(totals, RE::ActorValue::kCriticalChance).has_value() &&
		       NearlyEqual(*LookupTotal(totals, RE::ActorValue::kCriticalChance), 3.0f);
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

		snapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Defense1)] = "build.defense.endurance";
		snapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Wildcard1)].clear();
		const auto enduranceTotals = ComputeDerivedBuildActorValueTotals(snapshot);
		return LookupTotal(enduranceTotals, RE::ActorValue::kHealth).has_value() &&
		       NearlyEqual(*LookupTotal(enduranceTotals, RE::ActorValue::kHealth), 15.0f) &&
		       !LookupTotal(enduranceTotals, RE::ActorValue::kBlockPowerModifier).has_value();
	}

	bool CuratedUtilityEffectsResolveToConcreteActorValues()
	{
		BuildRuntimeSnapshot snapshot{};
		snapshot.utilityScore = 30u;
		snapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Utility1)] = "build.utility.cache";
		snapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Utility2)] = "build.utility.barter";
		snapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Wildcard1)] = "build.utility.mobility";

		const auto totals = ComputeDerivedBuildActorValueTotals(snapshot);
		return LookupTotal(totals, RE::ActorValue::kCarryWeight).has_value() &&
		       NearlyEqual(*LookupTotal(totals, RE::ActorValue::kCarryWeight), 40.0f) &&
		       LookupTotal(totals, RE::ActorValue::kSpeechcraftModifier).has_value() &&
		       NearlyEqual(*LookupTotal(totals, RE::ActorValue::kSpeechcraftModifier), 0.75f) &&
		       LookupTotal(totals, RE::ActorValue::kSpeedMult).has_value() &&
		       NearlyEqual(*LookupTotal(totals, RE::ActorValue::kSpeedMult), 0.03f);
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
