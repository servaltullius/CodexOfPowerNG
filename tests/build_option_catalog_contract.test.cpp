#include "CodexOfPowerNG/BuildOptionCatalog.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <string_view>
#include <unordered_set>
#include <variant>

namespace
{
	using namespace CodexOfPowerNG::Builds;

	bool HasUniqueOptionIds()
	{
		std::unordered_set<std::string_view> ids;
		for (const auto& option : GetBuildOptionCatalog()) {
			if (!ids.insert(option.id).second) {
				return false;
			}
		}
		return true;
	}

	bool AllOptionsUseValidDiscipline()
	{
		for (const auto& option : GetBuildOptionCatalog()) {
			switch (option.discipline) {
			case BuildDiscipline::Attack:
			case BuildDiscipline::Defense:
			case BuildDiscipline::Utility:
				break;
			default:
				return false;
			}
		}
		return true;
	}

	bool AllOptionsUseThresholdUnlocks()
	{
		for (const auto& option : GetBuildOptionCatalog()) {
			if (option.layer != BuildLayer::Slotted || option.unlockPointsCenti == 0u) {
				return false;
			}
		}
		return true;
	}

	bool AllOptionsUseOnceOnlyStackRule()
	{
		return std::all_of(
			GetBuildOptionCatalog().begin(),
			GetBuildOptionCatalog().end(),
			[](const BuildOptionDef& option) { return option.stackRule == BuildStackRule::OnceOnly; });
	}

	bool HasNoBaselineMilestones()
	{
		return GetBuildBaselineMilestones().empty();
	}

	bool HasExpectedInitialSlotLayout()
	{
		constexpr std::array expected{
			BuildSlotId::Attack1,
			BuildSlotId::Attack2,
			BuildSlotId::Defense1,
			BuildSlotId::Utility1,
			BuildSlotId::Utility2,
			BuildSlotId::Wildcard1,
		};
		const auto actual = GetInitialBuildSlotLayout();
		return std::equal(expected.begin(), expected.end(), actual.begin(), actual.end());
	}

	bool AllOptionsDeclareRequiredPresentationFields()
	{
		for (const auto& option : GetBuildOptionCatalog()) {
			if (option.titleKey.empty() ||
				option.descriptionKey.empty() ||
				option.effectKey.empty() ||
				option.themeId.empty() ||
				option.themeTitleKey.empty() ||
				option.hierarchy.empty()) {
				return false;
			}
		}
		return true;
	}

	bool AllOptionsUseStableThemeAndHierarchyIds()
	{
		for (const auto& option : GetBuildOptionCatalog()) {
			const auto hierarchy = option.hierarchy;
			if (hierarchy != "signpost" &&
				hierarchy != "standard" &&
				hierarchy != "special") {
				return false;
			}

			switch (option.discipline) {
			case BuildDiscipline::Attack:
				if (option.themeId != "devastation" &&
					option.themeId != "precision" &&
					option.themeId != "fury") {
					return false;
				}
				break;
			case BuildDiscipline::Defense:
				if (option.themeId != "guard" &&
					option.themeId != "bastion" &&
					option.themeId != "resistance") {
					return false;
				}
				break;
			case BuildDiscipline::Utility:
				if (option.themeId != "livelihood" &&
					option.themeId != "exploration" &&
					option.themeId != "trickery") {
					return false;
				}
				break;
			default:
				return false;
			}
		}
		return true;
	}

	bool AllOptionsDeclareEffectPayloads()
	{
		for (const auto& option : GetBuildOptionCatalog()) {
			const bool usesFloatMagnitude = std::holds_alternative<float>(option.magnitude);
			const bool usesIntMagnitude = std::holds_alternative<std::int32_t>(option.magnitude);
			const bool usesFloatTierMagnitude = std::holds_alternative<float>(option.magnitudePerTier);
			const bool usesIntTierMagnitude = std::holds_alternative<std::int32_t>(option.magnitudePerTier);
			switch (option.effectType) {
			case BuildEffectType::ActorValue:
			case BuildEffectType::CarryWeight:
				if (!usesFloatMagnitude || !usesFloatTierMagnitude) {
					return false;
				}
				break;
			case BuildEffectType::Economy:
			case BuildEffectType::UtilityFlag:
				if (!usesIntMagnitude || !usesIntTierMagnitude) {
					return false;
				}
				break;
			default:
				return false;
			}
		}
		return true;
	}

	bool UsesLinearBuildPointProgression()
	{
		return kBuildPointScale == 100u &&
		       kBuildPointsPerTierCenti == 800u &&
		       GetBuildPointsTier(0u) == 0u &&
		       GetBuildPointsTier(799u) == 0u &&
		       GetBuildPointsTier(800u) == 1u &&
		       GetBuildPointsTier(2400u) == 3u &&
		       GetNextBuildPointsThresholdCenti(0u) == 800u &&
		       GetNextBuildPointsThresholdCenti(800u) == 1600u &&
		       GetNextBuildPointsThresholdCenti(2400u) == 3200u &&
		       GetBuildPointsToNextTierCenti(0u) == 800u &&
		       GetBuildPointsToNextTierCenti(799u) == 1u &&
		       GetBuildPointsToNextTierCenti(800u) == 800u &&
		       GetBuildPointsToNextTierCenti(3199u) == 1u;
	}

	bool HasExpectedCatalogContents()
	{
		struct ExpectedOption
		{
			std::string_view id;
			BuildDiscipline discipline;
			std::string_view themeId;
			std::string_view themeTitleKey;
			std::string_view hierarchy;
			BuildPointCenti unlockPointsCenti;
			BuildEffectType effectType;
			std::string_view effectKey;
		};

		constexpr std::array expectedOptions{
			ExpectedOption{ "build.attack.ferocity", BuildDiscipline::Attack, "devastation", "build.theme.attack.devastation", "signpost", 400u, BuildEffectType::ActorValue, "attack_damage_mult" },
			ExpectedOption{ "build.attack.brawler", BuildDiscipline::Attack, "devastation", "build.theme.attack.devastation", "special", 2000u, BuildEffectType::ActorValue, "unarmed_damage" },
			ExpectedOption{ "build.attack.destruction", BuildDiscipline::Attack, "devastation", "build.theme.attack.devastation", "special", 2400u, BuildEffectType::ActorValue, "destruction_modifier" },
			ExpectedOption{ "build.attack.precision", BuildDiscipline::Attack, "precision", "build.theme.attack.precision", "standard", 1200u, BuildEffectType::ActorValue, "weapon_speed_mult" },
			ExpectedOption{ "build.attack.pinpoint", BuildDiscipline::Attack, "precision", "build.theme.attack.precision", "standard", 1600u, BuildEffectType::ActorValue, "critical_chance" },
			ExpectedOption{ "build.attack.vitals", BuildDiscipline::Attack, "precision", "build.theme.attack.precision", "special", 2800u, BuildEffectType::ActorValue, "melee_damage" },
			ExpectedOption{ "build.attack.reserve", BuildDiscipline::Attack, "fury", "build.theme.attack.fury", "signpost", 400u, BuildEffectType::ActorValue, "stamina" },
			ExpectedOption{ "build.attack.secondwind", BuildDiscipline::Attack, "fury", "build.theme.attack.fury", "standard", 800u, BuildEffectType::ActorValue, "stamina_rate" },
			ExpectedOption{ "build.defense.guard", BuildDiscipline::Defense, "guard", "build.theme.defense.guard", "signpost", 400u, BuildEffectType::ActorValue, "damage_resist" },
			ExpectedOption{ "build.defense.bulwark", BuildDiscipline::Defense, "guard", "build.theme.defense.guard", "standard", 1600u, BuildEffectType::ActorValue, "health" },
			ExpectedOption{ "build.defense.recovery", BuildDiscipline::Defense, "guard", "build.theme.defense.guard", "standard", 2400u, BuildEffectType::ActorValue, "heal_rate" },
			ExpectedOption{ "build.defense.restoration", BuildDiscipline::Defense, "guard", "build.theme.defense.guard", "special", 3200u, BuildEffectType::ActorValue, "restoration_modifier" },
			ExpectedOption{ "build.defense.bastion", BuildDiscipline::Defense, "bastion", "build.theme.defense.bastion", "standard", 1200u, BuildEffectType::ActorValue, "block_power_modifier" },
			ExpectedOption{ "build.defense.reprisal", BuildDiscipline::Defense, "bastion", "build.theme.defense.bastion", "special", 2000u, BuildEffectType::ActorValue, "reflect_damage" },
			ExpectedOption{ "build.defense.alteration", BuildDiscipline::Defense, "bastion", "build.theme.defense.bastion", "special", 2400u, BuildEffectType::ActorValue, "alteration_modifier" },
			ExpectedOption{ "build.defense.endurance", BuildDiscipline::Defense, "guard", "build.theme.defense.guard", "special", 2800u, BuildEffectType::ActorValue, "stamina" },
			ExpectedOption{ "build.defense.warding", BuildDiscipline::Defense, "resistance", "build.theme.defense.resistance", "signpost", 800u, BuildEffectType::ActorValue, "magic_resist_bundle" },
			ExpectedOption{ "build.defense.elementalWard", BuildDiscipline::Defense, "resistance", "build.theme.defense.resistance", "standard", 1600u, BuildEffectType::ActorValue, "elemental_resist_bundle" },
			ExpectedOption{ "build.defense.purification", BuildDiscipline::Defense, "resistance", "build.theme.defense.resistance", "standard", 2000u, BuildEffectType::ActorValue, "status_resist_bundle" },
			ExpectedOption{ "build.defense.absorption", BuildDiscipline::Defense, "resistance", "build.theme.defense.resistance", "special", 2800u, BuildEffectType::ActorValue, "absorb_chance" },
			ExpectedOption{ "build.utility.cache", BuildDiscipline::Utility, "livelihood", "build.theme.utility.livelihood", "signpost", 800u, BuildEffectType::CarryWeight, "carry_weight" },
			ExpectedOption{ "build.utility.barter", BuildDiscipline::Utility, "livelihood", "build.theme.utility.livelihood", "standard", 2000u, BuildEffectType::Economy, "speechcraft_modifier" },
			ExpectedOption{ "build.utility.smithing", BuildDiscipline::Utility, "livelihood", "build.theme.utility.livelihood", "standard", 1600u, BuildEffectType::ActorValue, "smithing_modifier" },
			ExpectedOption{ "build.utility.alchemy", BuildDiscipline::Utility, "livelihood", "build.theme.utility.livelihood", "standard", 2000u, BuildEffectType::ActorValue, "alchemy_modifier" },
			ExpectedOption{ "build.utility.meditation", BuildDiscipline::Utility, "livelihood", "build.theme.utility.livelihood", "standard", 2400u, BuildEffectType::ActorValue, "magicka_rate" },
			ExpectedOption{ "build.utility.enchanting", BuildDiscipline::Utility, "livelihood", "build.theme.utility.livelihood", "special", 2800u, BuildEffectType::ActorValue, "enchanting_modifier" },
			ExpectedOption{ "build.utility.magicka", BuildDiscipline::Utility, "livelihood", "build.theme.utility.livelihood", "special", 2400u, BuildEffectType::ActorValue, "magicka" },
			ExpectedOption{ "build.utility.hauler", BuildDiscipline::Utility, "livelihood", "build.theme.utility.livelihood", "standard", 2400u, BuildEffectType::ActorValue, "stamina" },
			ExpectedOption{ "build.utility.mobility", BuildDiscipline::Utility, "exploration", "build.theme.utility.exploration", "signpost", 1600u, BuildEffectType::ActorValue, "speed_mult" },
			ExpectedOption{ "build.utility.wayfinder", BuildDiscipline::Utility, "exploration", "build.theme.utility.exploration", "standard", 2400u, BuildEffectType::ActorValue, "stamina_rate" },
			ExpectedOption{ "build.utility.echo", BuildDiscipline::Utility, "exploration", "build.theme.utility.exploration", "special", 2800u, BuildEffectType::ActorValue, "shout_recovery_mult" },
			ExpectedOption{ "build.utility.sneak", BuildDiscipline::Utility, "trickery", "build.theme.utility.trickery", "signpost", 800u, BuildEffectType::ActorValue, "sneaking_modifier" },
			ExpectedOption{ "build.utility.lockpicking", BuildDiscipline::Utility, "trickery", "build.theme.utility.trickery", "standard", 1200u, BuildEffectType::ActorValue, "lockpicking_modifier" },
			ExpectedOption{ "build.utility.pickpocket", BuildDiscipline::Utility, "trickery", "build.theme.utility.trickery", "standard", 1600u, BuildEffectType::ActorValue, "pickpocket_modifier" },
			ExpectedOption{ "build.utility.conjuration", BuildDiscipline::Utility, "trickery", "build.theme.utility.trickery", "special", 2000u, BuildEffectType::ActorValue, "conjuration_modifier" },
			ExpectedOption{ "build.utility.illusion", BuildDiscipline::Utility, "trickery", "build.theme.utility.trickery", "special", 2400u, BuildEffectType::ActorValue, "illusion_modifier" },
		};

		const auto actual = GetBuildOptionCatalog();
		if (actual.size() != expectedOptions.size()) {
			std::cerr << "catalog size mismatch: actual=" << actual.size()
			          << " expected=" << expectedOptions.size() << '\n';
			return false;
		}

		for (std::size_t i = 0; i < expectedOptions.size(); ++i) {
			const auto& expected = expectedOptions[i];
			const auto& option = actual[i];
			if (option.id != expected.id ||
				option.discipline != expected.discipline ||
				option.themeId != expected.themeId ||
				option.themeTitleKey != expected.themeTitleKey ||
				option.hierarchy != expected.hierarchy ||
				option.unlockPointsCenti != expected.unlockPointsCenti ||
				option.effectType != expected.effectType ||
				option.effectKey != expected.effectKey) {
				std::cerr << "catalog option mismatch at index " << i
				          << " actual=" << option.id
				          << " expected=" << expected.id << '\n';
				return false;
			}
		}

		struct ExpectedScaling
		{
			std::string_view id;
			BuildMagnitude baseMagnitude;
			BuildMagnitude perTierMagnitude;
		};

		constexpr std::array expectedScaling{
			ExpectedScaling{ "build.attack.ferocity", 3.0f, 0.5f },
			ExpectedScaling{ "build.attack.precision", 2.5f, 0.35f },
			ExpectedScaling{ "build.attack.destruction", 0.06f, 0.01f },
			ExpectedScaling{ "build.attack.pinpoint", 1.5f, 0.3f },
			ExpectedScaling{ "build.defense.guard", 8.0f, 1.5f },
			ExpectedScaling{ "build.defense.bulwark", 6.0f, 2.5f },
			ExpectedScaling{ "build.defense.recovery", 0.01f, 0.005f },
			ExpectedScaling{ "build.defense.restoration", 0.05f, 0.01f },
			ExpectedScaling{ "build.defense.bastion", 5.0f, 0.75f },
			ExpectedScaling{ "build.defense.reprisal", 0.1f, 0.05f },
			ExpectedScaling{ "build.defense.alteration", 0.05f, 0.01f },
			ExpectedScaling{ "build.defense.warding", 2.0f, 0.5f },
			ExpectedScaling{ "build.defense.elementalWard", 2.0f, 0.5f },
			ExpectedScaling{ "build.defense.purification", 2.0f, 0.5f },
			ExpectedScaling{ "build.attack.vitals", 2.0f, 0.5f },
			ExpectedScaling{ "build.utility.cache", 20.0f, 1.0f },
			ExpectedScaling{ "build.utility.barter", 8, 1 },
			ExpectedScaling{ "build.utility.echo", -0.007f, -0.002f },
			ExpectedScaling{ "build.utility.smithing", 0.03f, 0.005f },
			ExpectedScaling{ "build.utility.alchemy", 0.03f, 0.005f },
			ExpectedScaling{ "build.utility.enchanting", 0.03f, 0.005f },
			ExpectedScaling{ "build.defense.endurance", 10.0f, 3.0f },
			ExpectedScaling{ "build.utility.hauler", 8.0f, 2.0f },
			ExpectedScaling{ "build.utility.mobility", 2.0f, 0.15f },
			ExpectedScaling{ "build.utility.wayfinder", 0.05f, 0.01f },
			ExpectedScaling{ "build.utility.sneak", 0.03f, 0.005f },
			ExpectedScaling{ "build.utility.lockpicking", 0.03f, 0.005f },
			ExpectedScaling{ "build.utility.pickpocket", 0.03f, 0.005f },
			ExpectedScaling{ "build.utility.conjuration", 0.03f, 0.005f },
			ExpectedScaling{ "build.utility.illusion", 0.03f, 0.005f },
		};

		struct ExpectedCompatibility
		{
			std::string_view       id;
			BuildSlotCompatibility slotCompatibility;
		};

		constexpr std::array expectedCompatibility{
			ExpectedCompatibility{ "build.attack.ferocity", BuildSlotCompatibility::SameDisciplineOnly },
			ExpectedCompatibility{ "build.attack.precision", BuildSlotCompatibility::SameDisciplineOnly },
			ExpectedCompatibility{ "build.attack.reserve", BuildSlotCompatibility::SameDisciplineOnly },
			ExpectedCompatibility{ "build.attack.pinpoint", BuildSlotCompatibility::SameOrWildcard },
			ExpectedCompatibility{ "build.attack.vitals", BuildSlotCompatibility::SameOrWildcard },
			ExpectedCompatibility{ "build.attack.secondwind", BuildSlotCompatibility::SameOrWildcard },
			ExpectedCompatibility{ "build.attack.brawler", BuildSlotCompatibility::SameOrWildcard },
			ExpectedCompatibility{ "build.attack.destruction", BuildSlotCompatibility::SameOrWildcard },
			ExpectedCompatibility{ "build.defense.guard", BuildSlotCompatibility::SameDisciplineOnly },
			ExpectedCompatibility{ "build.defense.warding", BuildSlotCompatibility::SameDisciplineOnly },
			ExpectedCompatibility{ "build.defense.reprisal", BuildSlotCompatibility::SameOrWildcard },
			ExpectedCompatibility{ "build.defense.absorption", BuildSlotCompatibility::SameOrWildcard },
			ExpectedCompatibility{ "build.utility.cache", BuildSlotCompatibility::SameDisciplineOnly },
			ExpectedCompatibility{ "build.utility.meditation", BuildSlotCompatibility::SameDisciplineOnly },
			ExpectedCompatibility{ "build.utility.magicka", BuildSlotCompatibility::SameDisciplineOnly },
			ExpectedCompatibility{ "build.utility.hauler", BuildSlotCompatibility::SameDisciplineOnly },
			ExpectedCompatibility{ "build.utility.mobility", BuildSlotCompatibility::SameDisciplineOnly },
			ExpectedCompatibility{ "build.utility.wayfinder", BuildSlotCompatibility::SameDisciplineOnly },
			ExpectedCompatibility{ "build.utility.sneak", BuildSlotCompatibility::SameDisciplineOnly },
			ExpectedCompatibility{ "build.utility.lockpicking", BuildSlotCompatibility::SameDisciplineOnly },
			ExpectedCompatibility{ "build.utility.pickpocket", BuildSlotCompatibility::SameDisciplineOnly },
			ExpectedCompatibility{ "build.utility.barter", BuildSlotCompatibility::SameOrWildcard },
			ExpectedCompatibility{ "build.utility.smithing", BuildSlotCompatibility::SameOrWildcard },
			ExpectedCompatibility{ "build.utility.alchemy", BuildSlotCompatibility::SameOrWildcard },
			ExpectedCompatibility{ "build.utility.enchanting", BuildSlotCompatibility::SameOrWildcard },
			ExpectedCompatibility{ "build.utility.echo", BuildSlotCompatibility::SameOrWildcard },
			ExpectedCompatibility{ "build.utility.conjuration", BuildSlotCompatibility::SameOrWildcard },
			ExpectedCompatibility{ "build.utility.illusion", BuildSlotCompatibility::SameOrWildcard },
		};

		for (const auto& expected : expectedScaling) {
			const auto it = std::find_if(
				actual.begin(),
				actual.end(),
				[&expected](const BuildOptionDef& option) { return option.id == expected.id; });
			if (it == actual.end() ||
				it->magnitude != expected.baseMagnitude ||
				it->magnitudePerTier != expected.perTierMagnitude) {
				std::cerr << "catalog scaling mismatch for " << expected.id << '\n';
				return false;
			}
		}

		for (const auto& expected : expectedCompatibility) {
			const auto it = std::find_if(
				actual.begin(),
				actual.end(),
				[&expected](const BuildOptionDef& option) { return option.id == expected.id; });
			if (it == actual.end() || it->slotCompatibility != expected.slotCompatibility) {
				std::cerr << "catalog compatibility mismatch for " << expected.id << '\n';
				return false;
			}
		}

		return true;
	}
}

int main()
{
	const auto expect = [](bool condition, const char* message) {
		if (!condition) {
			std::cerr << "build_option_catalog_contract: " << message << '\n';
			return false;
		}
		return true;
	};

	if (!expect(HasUniqueOptionIds(), "option ids must be unique")) {
		return 1;
	}
	if (!expect(AllOptionsUseValidDiscipline(), "options must use valid disciplines")) {
		return 1;
	}
	if (!expect(AllOptionsUseThresholdUnlocks(), "options must use threshold unlocks")) {
		return 1;
	}
	if (!expect(AllOptionsUseOnceOnlyStackRule(), "options must use once-only stack rules")) {
		return 1;
	}
	if (!expect(HasNoBaselineMilestones(), "baseline milestones must be removed from the catalog")) {
		return 1;
	}
	if (!expect(HasExpectedInitialSlotLayout(), "initial slot layout must match the agreed build slots")) {
		return 1;
	}
	if (!expect(AllOptionsDeclareRequiredPresentationFields(), "options must declare presentation fields")) {
		return 1;
	}
	if (!expect(AllOptionsUseStableThemeAndHierarchyIds(), "options must use stable theme ids and hierarchy values")) {
		return 1;
	}
	if (!expect(AllOptionsDeclareEffectPayloads(), "options must declare effect payloads")) {
		return 1;
	}
	if (!expect(UsesLinearBuildPointProgression(), "build progression must use the approved linear point tiers")) {
		return 1;
	}
	if (!expect(HasExpectedCatalogContents(), "catalog contents must match the fixed MVP contract")) {
		return 1;
	}

	return 0;
}
