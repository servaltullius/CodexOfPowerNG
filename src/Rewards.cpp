#include "CodexOfPowerNG/Rewards.h"

#include "CodexOfPowerNG/Config.h"
#include "CodexOfPowerNG/L10n.h"
#include "CodexOfPowerNG/RewardsResync.h"
#include "CodexOfPowerNG/State.h"
#include "CodexOfPowerNG/TaskScheduler.h"

#include "RewardsInternal.h"

#include <RE/Skyrim.h>

#include <RE/M/Misc.h>

#include <SKSE/Logger.h>

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace CodexOfPowerNG::Rewards
{
	namespace
	{
		[[nodiscard]] std::vector<std::pair<RE::ActorValue, float>> SnapshotRewardTotals() noexcept
		{
			std::vector<std::pair<RE::ActorValue, float>> totals;
			auto& state = GetState();
			std::scoped_lock lock(state.mutex);
			totals.reserve(state.rewardTotals.size());
			for (const auto& [av, total] : state.rewardTotals) {
				if (total != 0.0f) {
					totals.emplace_back(av, total);
				}
			}
			return totals;
		}

		[[nodiscard]] std::size_t ApplyRewardSyncPass() noexcept
		{
			auto* player = RE::PlayerCharacter::GetSingleton();
			if (!player) {
				return 0;
			}
			auto* avOwner = player->AsActorValueOwner();
			if (!avOwner) {
				return 0;
			}

			const auto totals = SnapshotRewardTotals();
			if (totals.empty()) {
				return 0;
			}

			std::size_t corrected = 0;
			for (const auto& [av, total] : totals) {
				const float base = avOwner->GetBaseActorValue(av);
				const float cur = avOwner->GetActorValue(av);

				float delta = ComputeRewardSyncDelta(base, cur, total);
				if (delta == 0.0f) {
					continue;
				}

				// Keep shout cooldown in sane range even if load-time state diverges.
				if (av == RE::ActorValue::kShoutRecoveryMult) {
					const float minValue = 0.30f;
					if (cur <= minValue) {
						continue;
					}
					if ((cur + delta) < minValue) {
						delta = minValue - cur;
					}
				}

				if (delta == 0.0f) {
					continue;
				}

				avOwner->ModActorValue(av, delta);
				++corrected;
				SKSE::log::info(
					"Reward sync: AV {} adjusted by {:.4f} (base {:.4f}, current {:.4f}, expectedTotal {:.4f})",
					static_cast<std::uint32_t>(av),
					delta,
					base,
					cur,
					total);
			}

			if (corrected > 0) {
				SKSE::log::info("Reward sync: corrected {} actor value entries", corrected);
			}

			return corrected;
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

		Internal::GrantWeightedRandomReward(group);
	}

	void SyncRewardTotalsToPlayer() noexcept
	{
		auto syncTask = []() noexcept {
			(void)ApplyRewardSyncPass();
			// Some actor value stacks settle one frame later after load.
			(void)QueueMainTask([]() { (void)ApplyRewardSyncPass(); });
		};

		if (QueueMainTask(syncTask)) {
			return;
		}

		syncTask();
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

		auto totals = SnapshotRewardTotals();
		{
			auto& state = GetState();
			std::scoped_lock lock(state.mutex);
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
