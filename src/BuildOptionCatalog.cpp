#include "CodexOfPowerNG/BuildOptionCatalog.h"

#include <array>

namespace CodexOfPowerNG::Builds
{
	namespace
	{
		constexpr std::array kBuildOptions{
			BuildOptionDef{
				"build.attack.ferocity",
				BuildDiscipline::Attack,
				"devastation",
				"build.theme.attack.devastation",
				"signpost",
				BuildLayer::Slotted,
				5u,
				BuildSlotCompatibility::SameOrWildcard,
				BuildEffectType::ActorValue,
				"attack_damage_mult",
				5.0f,
				"",
				BuildStackRule::OnceOnly,
				"build.attack.ferocity.title",
				"build.attack.ferocity.description",
			},
			BuildOptionDef{
				"build.attack.precision",
				BuildDiscipline::Attack,
				"precision",
				"build.theme.attack.precision",
				"standard",
				BuildLayer::Slotted,
				15u,
				BuildSlotCompatibility::SameOrWildcard,
				BuildEffectType::ActorValue,
				"weapon_speed_mult",
				3.0f,
				"",
				BuildStackRule::OnceOnly,
				"build.attack.precision.title",
				"build.attack.precision.description",
			},
			BuildOptionDef{
				"build.attack.vitals",
				BuildDiscipline::Attack,
				"precision",
				"build.theme.attack.precision",
				"special",
				BuildLayer::Slotted,
				30u,
				BuildSlotCompatibility::SameOrWildcard,
				BuildEffectType::ActorValue,
				"critical_chance",
				3.0f,
				"",
				BuildStackRule::OnceOnly,
				"build.attack.vitals.title",
				"build.attack.vitals.description",
			},
			BuildOptionDef{
				"build.defense.guard",
				BuildDiscipline::Defense,
				"guard",
				"build.theme.defense.guard",
				"signpost",
				BuildLayer::Slotted,
				5u,
				BuildSlotCompatibility::SameOrWildcard,
				BuildEffectType::ActorValue,
				"damage_resist",
				10.0f,
				"",
				BuildStackRule::OnceOnly,
				"build.defense.guard.title",
				"build.defense.guard.description",
			},
			BuildOptionDef{
				"build.defense.bastion",
				BuildDiscipline::Defense,
				"bastion",
				"build.theme.defense.bastion",
				"standard",
				BuildLayer::Slotted,
				15u,
				BuildSlotCompatibility::SameOrWildcard,
				BuildEffectType::ActorValue,
				"block_power_modifier",
				15.0f,
				"",
				BuildStackRule::OnceOnly,
				"build.defense.bastion.title",
				"build.defense.bastion.description",
			},
			BuildOptionDef{
				"build.defense.endurance",
				BuildDiscipline::Defense,
				"guard",
				"build.theme.defense.guard",
				"special",
				BuildLayer::Slotted,
				30u,
				BuildSlotCompatibility::SameOrWildcard,
				BuildEffectType::ActorValue,
				"health",
				15.0f,
				"",
				BuildStackRule::OnceOnly,
				"build.defense.endurance.title",
				"build.defense.endurance.description",
			},
			BuildOptionDef{
				"build.utility.cache",
				BuildDiscipline::Utility,
				"livelihood",
				"build.theme.utility.livelihood",
				"signpost",
				BuildLayer::Slotted,
				5u,
				BuildSlotCompatibility::SameOrWildcard,
				BuildEffectType::CarryWeight,
				"carry_weight",
				25.0f,
				"",
				BuildStackRule::OnceOnly,
				"build.utility.cache.title",
				"build.utility.cache.description",
			},
			BuildOptionDef{
				"build.utility.barter",
				BuildDiscipline::Utility,
				"livelihood",
				"build.theme.utility.livelihood",
				"standard",
				BuildLayer::Slotted,
				15u,
				BuildSlotCompatibility::SameOrWildcard,
				BuildEffectType::Economy,
				"speechcraft_modifier",
				10,
				"",
				BuildStackRule::OnceOnly,
				"build.utility.barter.title",
				"build.utility.barter.description",
			},
			BuildOptionDef{
				"build.utility.mobility",
				BuildDiscipline::Utility,
				"exploration",
				"build.theme.utility.exploration",
				"signpost",
				BuildLayer::Slotted,
				30u,
				BuildSlotCompatibility::SameOrWildcard,
				BuildEffectType::ActorValue,
				"speed_mult",
				3.0f,
				"",
				BuildStackRule::OnceOnly,
				"build.utility.mobility.title",
				"build.utility.mobility.description",
			},
		};

		constexpr std::array kBaselineMilestones{
			BuildBaselineMilestoneDef{ BuildDiscipline::Attack, 10u, BuildEffectType::ActorValue, "attack_damage_mult", 2.0f },
			BuildBaselineMilestoneDef{ BuildDiscipline::Attack, 25u, BuildEffectType::ActorValue, "attack_damage_mult", 4.0f },
			BuildBaselineMilestoneDef{ BuildDiscipline::Defense, 10u, BuildEffectType::ActorValue, "damage_resist", 5.0f },
			BuildBaselineMilestoneDef{ BuildDiscipline::Defense, 25u, BuildEffectType::ActorValue, "damage_resist", 10.0f },
			BuildBaselineMilestoneDef{ BuildDiscipline::Utility, 10u, BuildEffectType::Economy, "speechcraft_modifier", 5 },
			BuildBaselineMilestoneDef{ BuildDiscipline::Utility, 25u, BuildEffectType::CarryWeight, "carry_weight", 15.0f },
		};

		constexpr std::array kInitialSlotLayout{
			BuildSlotId::Attack1,
			BuildSlotId::Attack2,
			BuildSlotId::Defense1,
			BuildSlotId::Utility1,
			BuildSlotId::Utility2,
			BuildSlotId::Wildcard1,
		};
	}

	std::span<const BuildOptionDef> GetBuildOptionCatalog() noexcept
	{
		return kBuildOptions;
	}

	std::span<const BuildBaselineMilestoneDef> GetBuildBaselineMilestones() noexcept
	{
		return kBaselineMilestones;
	}

	std::span<const BuildSlotId> GetInitialBuildSlotLayout() noexcept
	{
		return kInitialSlotLayout;
	}
}
