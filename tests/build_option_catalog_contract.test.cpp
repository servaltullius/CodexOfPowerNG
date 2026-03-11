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
			if (option.layer != BuildLayer::Slotted || option.unlockScore == 0u) {
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

	bool AllBaselineMilestonesUseDisciplinesOnly()
	{
		for (const auto& milestone : GetBuildBaselineMilestones()) {
			switch (milestone.discipline) {
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
			if (option.titleKey.empty() || option.descriptionKey.empty() || option.effectKey.empty()) {
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
			switch (option.effectType) {
			case BuildEffectType::ActorValue:
			case BuildEffectType::CarryWeight:
				if (!usesFloatMagnitude) {
					return false;
				}
				break;
			case BuildEffectType::Economy:
			case BuildEffectType::UtilityFlag:
				if (!usesIntMagnitude) {
					return false;
				}
				break;
			default:
				return false;
			}
		}
		return true;
	}

	bool HasExpectedCatalogContents()
	{
		struct ExpectedOption
		{
			std::string_view id;
			BuildDiscipline discipline;
			std::uint32_t unlockScore;
			BuildEffectType effectType;
			std::string_view effectKey;
		};

		constexpr std::array expectedOptions{
			ExpectedOption{ "build.attack.ferocity", BuildDiscipline::Attack, 5u, BuildEffectType::ActorValue, "attack_damage_mult" },
			ExpectedOption{ "build.attack.precision", BuildDiscipline::Attack, 15u, BuildEffectType::ActorValue, "weapon_speed_mult" },
			ExpectedOption{ "build.attack.momentum", BuildDiscipline::Attack, 30u, BuildEffectType::UtilityFlag, "attack_chain_window" },
			ExpectedOption{ "build.defense.guard", BuildDiscipline::Defense, 5u, BuildEffectType::ActorValue, "damage_resist" },
			ExpectedOption{ "build.defense.bastion", BuildDiscipline::Defense, 15u, BuildEffectType::ActorValue, "block_percent" },
			ExpectedOption{ "build.defense.stalwart", BuildDiscipline::Defense, 30u, BuildEffectType::CarryWeight, "carry_weight" },
			ExpectedOption{ "build.utility.cache", BuildDiscipline::Utility, 5u, BuildEffectType::CarryWeight, "carry_weight" },
			ExpectedOption{ "build.utility.barter", BuildDiscipline::Utility, 15u, BuildEffectType::Economy, "vendor_price_bonus" },
			ExpectedOption{ "build.utility.scout", BuildDiscipline::Utility, 30u, BuildEffectType::UtilityFlag, "loot_reveal_radius" },
		};

		const auto actual = GetBuildOptionCatalog();
		if (actual.size() != expectedOptions.size()) {
			return false;
		}

		for (std::size_t i = 0; i < expectedOptions.size(); ++i) {
			const auto& expected = expectedOptions[i];
			const auto& option = actual[i];
			if (option.id != expected.id ||
				option.discipline != expected.discipline ||
				option.unlockScore != expected.unlockScore ||
				option.effectType != expected.effectType ||
				option.effectKey != expected.effectKey) {
				return false;
			}
		}

		struct ExpectedMilestone
		{
			BuildDiscipline discipline;
			std::uint32_t threshold;
			BuildEffectType effectType;
			std::string_view effectKey;
		};

		constexpr std::array expectedMilestones{
			ExpectedMilestone{ BuildDiscipline::Attack, 10u, BuildEffectType::ActorValue, "attack_damage_mult" },
			ExpectedMilestone{ BuildDiscipline::Attack, 25u, BuildEffectType::ActorValue, "attack_damage_mult" },
			ExpectedMilestone{ BuildDiscipline::Defense, 10u, BuildEffectType::ActorValue, "damage_resist" },
			ExpectedMilestone{ BuildDiscipline::Defense, 25u, BuildEffectType::ActorValue, "damage_resist" },
			ExpectedMilestone{ BuildDiscipline::Utility, 10u, BuildEffectType::Economy, "vendor_price_bonus" },
			ExpectedMilestone{ BuildDiscipline::Utility, 25u, BuildEffectType::CarryWeight, "carry_weight" },
		};

		const auto actualMilestones = GetBuildBaselineMilestones();
		if (actualMilestones.size() != expectedMilestones.size()) {
			return false;
		}

		for (std::size_t i = 0; i < expectedMilestones.size(); ++i) {
			const auto& expected = expectedMilestones[i];
			const auto& milestone = actualMilestones[i];
			if (milestone.discipline != expected.discipline ||
				milestone.threshold != expected.threshold ||
				milestone.effectType != expected.effectType ||
				milestone.effectKey != expected.effectKey) {
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
	if (!expect(AllBaselineMilestonesUseDisciplinesOnly(), "baseline milestones must use valid disciplines")) {
		return 1;
	}
	if (!expect(HasExpectedInitialSlotLayout(), "initial slot layout must match the agreed build slots")) {
		return 1;
	}
	if (!expect(AllOptionsDeclareRequiredPresentationFields(), "options must declare presentation fields")) {
		return 1;
	}
	if (!expect(AllOptionsDeclareEffectPayloads(), "options must declare effect payloads")) {
		return 1;
	}
	if (!expect(HasExpectedCatalogContents(), "catalog contents must match the fixed MVP contract")) {
		return 1;
	}

	return 0;
}
