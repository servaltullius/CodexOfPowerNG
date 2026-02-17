#include "RewardsInternal.h"

#include "CodexOfPowerNG/Config.h"
#include "CodexOfPowerNG/L10n.h"
#include "CodexOfPowerNG/State.h"

#include <RE/Skyrim.h>

#include <RE/M/Misc.h>

#include <random>
#include <string>

namespace CodexOfPowerNG::Rewards::Internal
{
	int RandomInt(int minInclusive, int maxInclusive) noexcept
	{
		static thread_local std::mt19937 rng{ std::random_device{}() };
		std::uniform_int_distribution<int> dist(minInclusive, maxInclusive);
		return dist(rng);
	}

	float RewardMult() noexcept
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

	void GrantReward(
		RE::ActorValue av,
		float amount,
		std::string_view labelKey,
		std::string_view fallbackLabel) noexcept
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
}
