#pragma once

#include <RE/Skyrim.h>

#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

namespace CodexOfPowerNG::RewardStateStore
{
	struct RewardTotalTransition
	{
		bool  existedBefore{ false };
		float previousTotal{ 0.0f };
		float nextTotal{ 0.0f };
	};

	struct RewardCapAdjustment
	{
		RE::ActorValue av;
		float          before;
		float          after;
	};

	[[nodiscard]] RewardTotalTransition AdjustClamped(RE::ActorValue av, float delta) noexcept;
	[[nodiscard]] std::optional<float>  Get(RE::ActorValue av) noexcept;
	[[nodiscard]] std::optional<float>  Take(RE::ActorValue av) noexcept;
	void                                Set(RE::ActorValue av, float total) noexcept;
	void                                Clear() noexcept;
	[[nodiscard]] std::size_t           Count() noexcept;
	[[nodiscard]] std::vector<std::pair<RE::ActorValue, float>> Snapshot() noexcept;
	[[nodiscard]] std::vector<RewardCapAdjustment>              ClampAll() noexcept;
}
