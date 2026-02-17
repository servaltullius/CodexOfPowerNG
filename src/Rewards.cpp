#include "CodexOfPowerNG/Rewards.h"

#include "CodexOfPowerNG/Config.h"
#include "CodexOfPowerNG/L10n.h"
#include "CodexOfPowerNG/State.h"

#include "RewardsInternal.h"

#include <RE/Skyrim.h>

#include <RE/M/Misc.h>

#include <string>
#include <utility>
#include <vector>

namespace CodexOfPowerNG::Rewards
{
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
