#include "PrismaUIPayloads.h"

#include "CodexOfPowerNG/L10n.h"

#include <array>
#include <cstdio>

namespace CodexOfPowerNG::PrismaUIPayloads
{
	namespace
	{
		struct AvRewardMeta
		{
			RE::ActorValue av;
			const char*    labelKey;
			const char*    fallback;
			const char*    fmt;  // "raw", "pct", "multPct"
		};

		constexpr std::array<AvRewardMeta, 42> kAvRewardTable{ {
			{ RE::ActorValue::kHealth,              "av.health",            "Health",                     "raw"     },
			{ RE::ActorValue::kMagicka,             "av.magicka",           "Magicka",                    "raw"     },
			{ RE::ActorValue::kStamina,             "av.stamina",           "Stamina",                    "raw"     },
			{ RE::ActorValue::kHealRate,            "av.healRate",          "Health regen",               "raw"     },
			{ RE::ActorValue::kMagickaRate,         "av.magickaRate",       "Magicka regen",              "raw"     },
			{ RE::ActorValue::kStaminaRate,         "av.staminaRate",       "Stamina regen",              "raw"     },
			{ RE::ActorValue::kAttackDamageMult,    "av.attackDamageMult",  "Attack damage (physical)",   "multPct" },
			{ RE::ActorValue::kCriticalChance,      "av.critChance",        "Critical chance",            "pct"     },
			{ RE::ActorValue::kUnarmedDamage,       "av.unarmedDamage",     "Unarmed damage",             "raw"     },
			{ RE::ActorValue::kReflectDamage,       "av.reflectDamage",     "Reflect damage",             "pct"     },
			{ RE::ActorValue::kResistMagic,         "av.magicResist",       "Magic resist",               "pct"     },
			{ RE::ActorValue::kResistFire,          "av.fireResist",        "Fire resist",                "pct"     },
			{ RE::ActorValue::kResistFrost,         "av.frostResist",       "Frost resist",               "pct"     },
			{ RE::ActorValue::kResistShock,         "av.electricResist",    "Shock resist",               "pct"     },
			{ RE::ActorValue::kPoisonResist,        "av.poisonResist",      "Poison resist",              "pct"     },
			{ RE::ActorValue::kResistDisease,       "av.diseaseResist",     "Disease resist",             "pct"     },
			{ RE::ActorValue::kDamageResist,        "av.damageResist",      "Armor rating",               "raw"     },
			{ RE::ActorValue::kCarryWeight,         "av.carryWeight",       "Carry weight",               "raw"     },
			{ RE::ActorValue::kSpeedMult,           "av.speedMult",         "Move speed",                 "multPct" },
			{ RE::ActorValue::kSmithingModifier,    "av.smithingMod",       "Smithing effectiveness",     "raw"     },
			{ RE::ActorValue::kAlchemyModifier,     "av.alchemyMod",        "Alchemy effectiveness",      "raw"     },
			{ RE::ActorValue::kEnchantingModifier,  "av.enchantingMod",     "Enchanting effectiveness",   "raw"     },
			{ RE::ActorValue::kSpeechcraftModifier, "av.speechcraftMod",    "Barter effectiveness",       "raw"     },
			{ RE::ActorValue::kSneakingModifier,    "av.sneakMod",          "Sneak effectiveness",        "raw"     },
			{ RE::ActorValue::kLockpickingModifier, "av.lockpickingMod",    "Lockpicking effectiveness",  "raw"     },
			{ RE::ActorValue::kPickpocketModifier,  "av.pickpocketMod",     "Pickpocket effectiveness",   "raw"     },
			{ RE::ActorValue::kAlterationModifier,  "av.alterationMod",     "Alteration cost reduction",  "raw"     },
			{ RE::ActorValue::kConjurationModifier, "av.conjurationMod",    "Conjuration cost reduction", "raw"     },
			{ RE::ActorValue::kDestructionModifier, "av.destructionMod",    "Destruction cost reduction", "raw"     },
			{ RE::ActorValue::kIllusionModifier,    "av.illusionMod",       "Illusion cost reduction",    "raw"     },
			{ RE::ActorValue::kRestorationModifier, "av.restorationMod",    "Restoration cost reduction", "raw"     },
			{ RE::ActorValue::kAbsorbChance,        "av.absorbChance",      "Spell absorption chance",    "pct"     },
			{ RE::ActorValue::kShoutRecoveryMult,   "av.shoutRecoveryMult", "Shout cooldown",             "multPct" },
			{ RE::ActorValue::kOneHanded,           "skill.oneHanded",      "Skill (One-Handed)",         "raw"     },
			{ RE::ActorValue::kTwoHanded,           "skill.twoHanded",      "Skill (Two-Handed)",         "raw"     },
			{ RE::ActorValue::kArchery,             "skill.marksman",       "Skill (Archery)",            "raw"     },
			{ RE::ActorValue::kHeavyArmor,          "skill.heavyArmor",     "Skill (Heavy Armor)",        "raw"     },
			{ RE::ActorValue::kLightArmor,          "skill.lightArmor",     "Skill (Light Armor)",        "raw"     },
			{ RE::ActorValue::kBlock,               "skill.block",          "Skill (Block)",              "raw"     },
			{ RE::ActorValue::kAlchemy,             "skill.alchemy",        "Skill (Alchemy)",            "raw"     },
			{ RE::ActorValue::kLockpicking,         "skill.lockpicking",    "Skill (Lockpicking)",        "raw"     },
			{ RE::ActorValue::kPickpocket,          "skill.pickpocket",     "Skill (Pickpocket)",         "raw"     },
		} };

		[[nodiscard]] const AvRewardMeta* FindAvMeta(RE::ActorValue av) noexcept
		{
			for (const auto& entry : kAvRewardTable) {
				if (entry.av == av) {
					return &entry;
				}
			}
			return nullptr;
		}
	}

	std::string FormatReward(float total, std::string_view fmt) noexcept
	{
		char buf[64];
		if (fmt == "pct") {
			std::snprintf(buf, sizeof(buf), "%+.2f%%", static_cast<double>(total));
			return buf;
		}
		if (fmt == "multPct") {
			std::snprintf(buf, sizeof(buf), "%+.2f%%", static_cast<double>(total * 100.0f));
			return buf;
		}
		std::snprintf(buf, sizeof(buf), "%+.2f", static_cast<double>(total));
		return buf;
	}

	json BuildRewardTotalsArray(
		const std::vector<std::pair<RE::ActorValue, float>>& totals,
		bool useL10n) noexcept
	{
		json arr = json::array();
		for (const auto& [av, total] : totals) {
			const auto* meta = FindAvMeta(av);
			const char* fmtStr = meta ? meta->fmt : "raw";
			std::string label;
			if (meta) {
				label = useL10n ? L10n::T(meta->labelKey, meta->fallback) : meta->fallback;
			} else {
				label = "Unknown";
			}

			arr.push_back({
				{ "av", static_cast<std::uint32_t>(av) },
				{ "label", label },
				{ "total", total },
				{ "format", fmtStr },
				{ "display", FormatReward(total, fmtStr) },
			});
		}
		return arr;
	}
}
