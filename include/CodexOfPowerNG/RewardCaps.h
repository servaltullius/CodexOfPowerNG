#pragma once

#include <RE/Skyrim.h>

#include <cmath>

namespace CodexOfPowerNG::Rewards
{
	inline constexpr float kRewardCapEpsilon = 0.001f;

	[[nodiscard]] inline bool TryGetRewardCap(RE::ActorValue av, float& outCap) noexcept
	{
		switch (av) {
		case RE::ActorValue::kAttackDamageMult:
			outCap = 1.0f;
			return true;
		case RE::ActorValue::kCriticalChance:
			outCap = 50.0f;
			return true;
		case RE::ActorValue::kResistMagic:
		case RE::ActorValue::kResistFire:
		case RE::ActorValue::kResistFrost:
		case RE::ActorValue::kResistShock:
		case RE::ActorValue::kPoisonResist:
		case RE::ActorValue::kResistDisease:
			outCap = 85.0f;
			return true;
		case RE::ActorValue::kAbsorbChance:
		case RE::ActorValue::kReflectDamage:
			outCap = 80.0f;
			return true;
		case RE::ActorValue::kSpeedMult:
			outCap = 0.50f;
			return true;
		default:
			return false;
		}
	}

	[[nodiscard]] inline float ClampRewardTotal(
		RE::ActorValue av,
		float total,
		float epsilon = kRewardCapEpsilon) noexcept
	{
		if (!std::isfinite(total)) {
			return 0.0f;
		}

		float cap = 0.0f;
		if (!TryGetRewardCap(av, cap)) {
			return total;
		}

		return (total > (cap + epsilon)) ? cap : total;
	}
}
