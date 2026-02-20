#pragma once

#include <RE/Skyrim.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace CodexOfPowerNG::Registration
{
	inline constexpr std::size_t kUndoHistoryLimit = 10;

	struct RewardDelta
	{
		RE::ActorValue av{ RE::ActorValue::kNone };
		float          delta{ 0.0f };
	};

	struct UndoRecord
	{
		std::uint64_t            actionId{ 0 };
		RE::FormID               formId{ 0 };
		RE::FormID               regKey{ 0 };
		std::uint32_t            group{ 255 };
		std::vector<RewardDelta> rewardDeltas;
	};
}
