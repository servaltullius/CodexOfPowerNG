#include "CodexOfPowerNG/Rewards.h"

#include "CodexOfPowerNG/Config.h"
#include "CodexOfPowerNG/L10n.h"
#include "CodexOfPowerNG/State.h"

#include <RE/Skyrim.h>

#include <RE/M/Misc.h>

#include <random>
#include <string>
#include <utility>
#include <vector>

namespace CodexOfPowerNG::Rewards
{
	namespace
	{
		[[nodiscard]] int RandomInt(int minInclusive, int maxInclusive) noexcept
		{
			static thread_local std::mt19937 rng{ std::random_device{}() };
			std::uniform_int_distribution<int> dist(minInclusive, maxInclusive);
			return dist(rng);
		}

		[[nodiscard]] float RewardMult() noexcept
		{
			const auto settings = GetSettings();
			if (!settings.enableRewards) {
				return 0.0f;
			}

			const auto mult = static_cast<float>(settings.rewardMultiplier);
			if (mult <= 0.0f) {
				return 0.0f;
			}

			return mult;
		}

		void RecordRewardDelta(RE::ActorValue av, float delta) noexcept
		{
			auto& state = GetState();
			std::scoped_lock lock(state.mutex);
			state.rewardTotals[av] += delta;
		}

		void GrantReward(RE::ActorValue av, float amount, std::string_view labelKey, std::string_view fallbackLabel) noexcept
		{
			auto* player = RE::PlayerCharacter::GetSingleton();
			if (!player) {
				return;
			}
			auto* avOwner = player->AsActorValueOwner();
			if (!avOwner) {
				return;
			}

			const float mult = RewardMult();
			if (mult <= 0.0f) {
				return;
			}

			float applied = amount * mult;

			// ShoutRecoveryMult: lower is better (default 1.0, 0 removes cooldown)
			if (av == RE::ActorValue::kShoutRecoveryMult) {
				const float cur = avOwner->GetActorValue(av);
				const float minValue = 0.30f;
				if (cur <= minValue) {
					applied = 0.0f;
				} else if ((cur + applied) < minValue) {
					applied = minValue - cur;
				}
			}

			if (applied == 0.0f) {
				return;
			}

			avOwner->ModActorValue(av, applied);
			RecordRewardDelta(av, applied);

			const auto label = L10n::T(labelKey, fallbackLabel);
			const auto msg = L10n::T("msg.rewardPrefix", "Collection reward: ") + label;
			RE::DebugNotification(msg.c_str());
		}

		void GrantWeightedRandomReward(std::uint32_t group) noexcept
		{
			const auto settings = GetSettings();
			if (!settings.enableRewards) {
				return;
			}

			const int roll = RandomInt(1, 100);

			// group: 0=weapons, 1=armors, 2=consumables, 3=ingredients, 4=books, 5=misc
			if (group == 0) {
				// Weapons: damage-feel without touching skill levels (perk mod friendly)
				// Note: we intentionally avoid "MeleeDamage" to prevent melee-only bias.
				if (roll <= 35) {
					GrantReward(RE::ActorValue::kAttackDamageMult, 0.015f, "av.attackDamageMult", "Attack damage (physical)");
				} else if (roll <= 60) {
					GrantReward(RE::ActorValue::kCriticalChance, 1.0f, "av.critChance", "Critical chance");
				} else if (roll <= 70) {
					GrantReward(RE::ActorValue::kUnarmedDamage, 0.20f, "av.unarmedDamage", "Unarmed damage");
				} else if (roll <= 80) {
					GrantReward(RE::ActorValue::kStaminaRate, 0.10f, "av.staminaRate", "Stamina regen");
				} else if (roll <= 90) {
					GrantReward(RE::ActorValue::kStamina, 2.0f, "av.stamina", "Stamina");
				} else if (roll <= 97) {
					GrantReward(RE::ActorValue::kSpeedMult, 0.50f, "av.speedMult", "Move speed");
				} else {
					GrantReward(RE::ActorValue::kCarryWeight, 5.0f, "av.carryWeight", "Carry weight");
				}

				// Optional: allow tiny skill bumps (OFF by default)
				if (settings.allowSkillRewards) {
					const int extra = RandomInt(1, 3);
					if (extra == 1) {
						GrantReward(RE::ActorValue::kOneHanded, 0.10f, "skill.oneHanded", "Skill (One-Handed)");
					} else if (extra == 2) {
						GrantReward(RE::ActorValue::kTwoHanded, 0.10f, "skill.twoHanded", "Skill (Two-Handed)");
					} else {
						GrantReward(RE::ActorValue::kArchery, 0.10f, "skill.marksman", "Skill (Archery)");
					}
				}

				return;
			}

			if (group == 1) {
				// Armors: defenses + resist
				if (roll <= 15) {
					GrantReward(RE::ActorValue::kDamageResist, 5.0f, "av.damageResist", "Armor rating");
				} else if (roll <= 28) {
					GrantReward(RE::ActorValue::kHealth, 2.0f, "av.health", "Health");
				} else if (roll <= 36) {
					GrantReward(RE::ActorValue::kResistMagic, 0.75f, "av.magicResist", "Magic resist");
				} else if (roll <= 44) {
					GrantReward(RE::ActorValue::kResistFire, 1.0f, "av.fireResist", "Fire resist");
				} else if (roll <= 52) {
					GrantReward(RE::ActorValue::kResistFrost, 1.0f, "av.frostResist", "Frost resist");
				} else if (roll <= 60) {
					GrantReward(RE::ActorValue::kResistShock, 1.0f, "av.electricResist", "Shock resist");
				} else if (roll <= 68) {
					GrantReward(RE::ActorValue::kHealRate, 0.02f, "av.healRate", "Health regen");
				} else if (roll <= 74) {
					GrantReward(RE::ActorValue::kReflectDamage, 0.30f, "av.reflectDamage", "Reflect damage");
				} else if (roll <= 80) {
					if (settings.allowSkillRewards) {
						GrantReward(RE::ActorValue::kHeavyArmor, 0.10f, "skill.heavyArmor", "Skill (Heavy Armor)");
					} else {
						GrantReward(RE::ActorValue::kSmithingModifier, 0.50f, "av.smithingMod", "Smithing effectiveness");
					}
				} else if (roll <= 88) {
					if (settings.allowSkillRewards) {
						GrantReward(RE::ActorValue::kLightArmor, 0.10f, "skill.lightArmor", "Skill (Light Armor)");
					} else {
						GrantReward(RE::ActorValue::kCarryWeight, 5.0f, "av.carryWeight", "Carry weight");
					}
				} else if (roll <= 94) {
					if (settings.allowSkillRewards) {
						GrantReward(RE::ActorValue::kBlock, 0.10f, "skill.block", "Skill (Block)");
					} else {
						GrantReward(RE::ActorValue::kSpeedMult, 0.50f, "av.speedMult", "Move speed");
					}
				} else {
					GrantReward(RE::ActorValue::kHealth, 2.0f, "av.health", "Health");
				}

				return;
			}

			if (group == 2) {
				// Consumables: attributes + small resist
				if (roll <= 20) {
					GrantReward(RE::ActorValue::kMagicka, 2.0f, "av.magicka", "Magicka");
				} else if (roll <= 40) {
					GrantReward(RE::ActorValue::kHealth, 2.0f, "av.health", "Health");
				} else if (roll <= 60) {
					GrantReward(RE::ActorValue::kStamina, 2.0f, "av.stamina", "Stamina");
				} else if (roll <= 68) {
					GrantReward(RE::ActorValue::kMagickaRate, 0.10f, "av.magickaRate", "Magicka regen");
				} else if (roll <= 76) {
					GrantReward(RE::ActorValue::kStaminaRate, 0.10f, "av.staminaRate", "Stamina regen");
				} else if (roll <= 82) {
					GrantReward(RE::ActorValue::kHealRate, 0.02f, "av.healRate", "Health regen");
				} else if (roll <= 87) {
					GrantReward(RE::ActorValue::kResistMagic, 0.75f, "av.magicResist", "Magic resist");
				} else if (roll <= 91) {
					GrantReward(RE::ActorValue::kResistFire, 1.0f, "av.fireResist", "Fire resist");
				} else if (roll <= 95) {
					GrantReward(RE::ActorValue::kResistFrost, 1.0f, "av.frostResist", "Frost resist");
				} else if (roll <= 98) {
					GrantReward(RE::ActorValue::kResistShock, 1.0f, "av.electricResist", "Shock resist");
				} else {
					GrantReward(RE::ActorValue::kSpeedMult, 0.50f, "av.speedMult", "Move speed");
				}

				return;
			}

			if (group == 3) {
				// Ingredients: alchemy + poison/disease resist
				if (roll <= 18) {
					GrantReward(RE::ActorValue::kPoisonResist, 1.0f, "av.poisonResist", "Poison resist");
				} else if (roll <= 28) {
					GrantReward(RE::ActorValue::kResistDisease, 1.0f, "av.diseaseResist", "Disease resist");
				} else if (roll <= 40) {
					GrantReward(RE::ActorValue::kHealRate, 0.02f, "av.healRate", "Health regen");
				} else if (roll <= 52) {
					GrantReward(RE::ActorValue::kMagickaRate, 0.10f, "av.magickaRate", "Magicka regen");
				} else if (roll <= 66) {
					GrantReward(RE::ActorValue::kAlchemyModifier, 0.50f, "av.alchemyMod", "Alchemy effectiveness");
				} else if (roll <= 76) {
					if (settings.allowSkillRewards) {
						GrantReward(RE::ActorValue::kAlchemy, 0.10f, "skill.alchemy", "Skill (Alchemy)");
					} else {
						GrantReward(RE::ActorValue::kMagicka, 2.0f, "av.magicka", "Magicka");
					}
				} else if (roll <= 88) {
					GrantReward(RE::ActorValue::kHealth, 2.0f, "av.health", "Health");
				} else if (roll <= 96) {
					GrantReward(RE::ActorValue::kStaminaRate, 0.10f, "av.staminaRate", "Stamina regen");
				} else {
					GrantReward(RE::ActorValue::kStamina, 2.0f, "av.stamina", "Stamina");
				}

				return;
			}

			if (group == 4) {
				// Books: magic utility + (rare) spell power boosts.
				// NG: perk-based rewards are plugin-dependent; use AV fallback always.
				if (roll <= 4) {
					GrantReward(RE::ActorValue::kDestructionModifier, 0.50f, "av.destructionMod", "Destruction cost reduction");
				} else if (roll <= 8) {
					GrantReward(RE::ActorValue::kRestorationModifier, 0.50f, "av.restorationMod", "Restoration cost reduction");
				} else if (roll <= 12) {
					GrantReward(RE::ActorValue::kAlterationModifier, 0.50f, "av.alterationMod", "Alteration cost reduction");
				} else if (roll <= 16) {
					GrantReward(RE::ActorValue::kConjurationModifier, 0.50f, "av.conjurationMod", "Conjuration cost reduction");
				} else if (roll <= 20) {
					GrantReward(RE::ActorValue::kIllusionModifier, 0.50f, "av.illusionMod", "Illusion cost reduction");
				} else if (roll <= 34) {
					GrantReward(RE::ActorValue::kEnchantingModifier, 0.50f, "av.enchantingMod", "Enchanting effectiveness");
				} else if (roll <= 45) {
					GrantReward(RE::ActorValue::kAlterationModifier, 0.50f, "av.alterationMod", "Alteration cost reduction");
				} else if (roll <= 56) {
					GrantReward(RE::ActorValue::kDestructionModifier, 0.50f, "av.destructionMod", "Destruction cost reduction");
				} else if (roll <= 66) {
					GrantReward(RE::ActorValue::kConjurationModifier, 0.50f, "av.conjurationMod", "Conjuration cost reduction");
				} else if (roll <= 76) {
					GrantReward(RE::ActorValue::kIllusionModifier, 0.50f, "av.illusionMod", "Illusion cost reduction");
				} else if (roll <= 86) {
					GrantReward(RE::ActorValue::kRestorationModifier, 0.50f, "av.restorationMod", "Restoration cost reduction");
				} else if (roll <= 92) {
					GrantReward(RE::ActorValue::kAbsorbChance, 0.40f, "av.absorbChance", "Spell absorption chance");
				} else if (roll <= 96) {
					GrantReward(RE::ActorValue::kShoutRecoveryMult, -0.02f, "av.shoutRecoveryMult", "Shout cooldown");
				} else {
					GrantReward(RE::ActorValue::kMagickaRate, 0.10f, "av.magickaRate", "Magicka regen");
				}

				return;
			}

			// Misc: crafting/economy/stealth utility
			if (roll <= 15) {
				GrantReward(RE::ActorValue::kCarryWeight, 5.0f, "av.carryWeight", "Carry weight");
			} else if (roll <= 30) {
				GrantReward(RE::ActorValue::kSpeechcraftModifier, 0.50f, "av.speechcraftMod", "Barter effectiveness");
			} else if (roll <= 42) {
				GrantReward(RE::ActorValue::kSmithingModifier, 0.50f, "av.smithingMod", "Smithing effectiveness");
			} else if (roll <= 54) {
				GrantReward(RE::ActorValue::kLockpickingModifier, 0.50f, "av.lockpickingMod", "Lockpicking effectiveness");
			} else if (roll <= 66) {
				GrantReward(RE::ActorValue::kPickpocketModifier, 0.50f, "av.pickpocketMod", "Pickpocket effectiveness");
			} else if (roll <= 78) {
				GrantReward(RE::ActorValue::kSneakingModifier, 0.50f, "av.sneakMod", "Sneak effectiveness");
			} else if (roll <= 86) {
				GrantReward(RE::ActorValue::kSpeedMult, 0.50f, "av.speedMult", "Move speed");
			} else if (roll <= 92) {
				GrantReward(RE::ActorValue::kCriticalChance, 1.0f, "av.critChance", "Critical chance");
			} else if (roll <= 96) {
				GrantReward(RE::ActorValue::kHealRate, 0.02f, "av.healRate", "Health regen");
			} else {
				GrantReward(RE::ActorValue::kStaminaRate, 0.10f, "av.staminaRate", "Stamina regen");
			}

			// keep optional skill rewards minimal in misc
			if (settings.allowSkillRewards) {
				const int extraMisc = RandomInt(1, 6);
				if (extraMisc == 1) {
					GrantReward(RE::ActorValue::kLockpicking, 0.10f, "skill.lockpicking", "Skill (Lockpicking)");
				} else if (extraMisc == 2) {
					GrantReward(RE::ActorValue::kPickpocket, 0.10f, "skill.pickpocket", "Skill (Pickpocket)");
				}
			}
		}
	}

	void MaybeGrantRegistrationReward(std::uint32_t group, std::int32_t totalRegistered) noexcept
	{
		const auto settings = GetSettings();
		if (!settings.enableRewards) {
			return;
		}

		const auto every = settings.rewardEvery;
		if (every <= 0) {
			return;
		}

		if (totalRegistered <= 0) {
			return;
		}

		if ((totalRegistered % every) != 0) {
			return;
		}

		GrantWeightedRandomReward(group);
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

		std::vector<std::pair<RE::ActorValue, float>> totals;
		{
			auto& state = GetState();
			std::scoped_lock lock(state.mutex);
			totals.reserve(state.rewardTotals.size());
			for (const auto& [av, total] : state.rewardTotals) {
				if (total != 0.0f) {
					totals.emplace_back(av, total);
				}
			}
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
}
