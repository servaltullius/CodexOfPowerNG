#pragma once

#include <RE/Skyrim.h>

#include <unordered_set>

namespace CodexOfPowerNG::Registration::QuestGuard
{
	/// Returns a snapshot of base FormIDs referenced by running quest RefAliases (TTL-cached).
	[[nodiscard]] std::unordered_set<RE::FormID> SnapshotProtectedForms();

	/// Single-FormID convenience check against the cached set.
	[[nodiscard]] bool IsQuestProtected(RE::FormID formId) noexcept;
}
