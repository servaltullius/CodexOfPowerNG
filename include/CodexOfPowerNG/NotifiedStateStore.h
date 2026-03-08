#pragma once

#include <RE/Skyrim.h>

#include <cstddef>
#include <unordered_set>

namespace CodexOfPowerNG::NotifiedStateStore
{
	[[nodiscard]] bool ContainsAny(RE::FormID primaryId, RE::FormID secondaryId) noexcept;
	void               MarkPair(RE::FormID primaryId, RE::FormID secondaryId) noexcept;
	void               Clear() noexcept;
	void               ReplaceAll(std::unordered_set<RE::FormID> notifiedItems) noexcept;
	[[nodiscard]] std::unordered_set<RE::FormID> Snapshot() noexcept;
	[[nodiscard]] std::size_t                    Count() noexcept;
}
