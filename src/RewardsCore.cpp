#include "RewardsInternal.h"

#include "CodexOfPowerNG/Config.h"
#include "CodexOfPowerNG/L10n.h"
#include "CodexOfPowerNG/RewardCaps.h"
#include "CodexOfPowerNG/Rewards.h"
#include "CodexOfPowerNG/State.h"

#include <RE/Skyrim.h>

#include <RE/M/Misc.h>

#include <SKSE/Logger.h>

#include <cmath>
#include <random>
#include <string>

namespace CodexOfPowerNG::Rewards::Internal
{
	namespace
	{
		thread_local std::vector<Registration::RewardDelta>* g_rewardCaptureOut{ nullptr };
	}

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

	float RecordRewardDelta(RE::ActorValue av, float delta) noexcept
	{
		if (std::abs(delta) <= kRewardCapEpsilon) {
			return 0.0f;
		}

		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		auto& total = state.rewardTotals[av];
		const float clampedTotal = ClampRewardTotal(av, total + delta);
		const float appliedDelta = clampedTotal - total;
		total = clampedTotal;
		return (std::abs(appliedDelta) <= kRewardCapEpsilon) ? 0.0f : appliedDelta;
	}

	void BeginRewardDeltaCapture(std::vector<Registration::RewardDelta>& outDeltas) noexcept
	{
		outDeltas.clear();
		g_rewardCaptureOut = &outDeltas;
	}

	void EndRewardDeltaCapture() noexcept
	{
		g_rewardCaptureOut = nullptr;
	}

	void CaptureAppliedRewardDelta(RE::ActorValue av, float delta) noexcept
	{
		if (!g_rewardCaptureOut || std::abs(delta) <= kRewardCapEpsilon) {
			return;
		}

		for (auto& entry : *g_rewardCaptureOut) {
			if (entry.av != av) {
				continue;
			}
			entry.delta += delta;
			return;
		}

		g_rewardCaptureOut->push_back(Registration::RewardDelta{ av, delta });
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

		if (std::abs(applied) <= kRewardCapEpsilon) {
			return;
		}

		const float requested = applied;
		applied = RecordRewardDelta(av, applied);
		if (std::abs(applied) <= kRewardCapEpsilon) {
			float cap = 0.0f;
			if (requested > 0.0f && TryGetRewardCap(av, cap)) {
				SKSE::log::info(
					"Reward grant skipped by cap: AV {} request {:.4f} (cap {:.2f})",
					static_cast<std::uint32_t>(av),
					requested,
					cap);
			}
			return;
		}

		avOwner->ModActorValue(av, applied);
		CaptureAppliedRewardDelta(av, applied);
		if (av == RE::ActorValue::kCarryWeight) {
			// Carry weight desync reports are high-impact in gameplay feel.
			// Schedule carry-only quick resync so missed application
			// (load/order side effects) is recovered without full sync cost.
			Rewards::ScheduleCarryWeightQuickResync();
		}
		if (std::abs(applied - requested) > kRewardCapEpsilon) {
			float cap = 0.0f;
			if (TryGetRewardCap(av, cap)) {
				SKSE::log::info(
					"Reward cap applied: AV {} request {:.4f} -> applied {:.4f} (cap {:.2f})",
					static_cast<std::uint32_t>(av),
					requested,
					applied,
					cap);
			}
		}

		const auto label = L10n::T(labelKey, fallbackLabel);
		const auto msg = L10n::T("msg.rewardPrefix", "Collection reward: ") + label;
		RE::DebugNotification(msg.c_str());
	}
}
