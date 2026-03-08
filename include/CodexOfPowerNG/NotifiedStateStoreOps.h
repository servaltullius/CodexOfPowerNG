#pragma once

#include <cstddef>
#include <utility>

namespace CodexOfPowerNG::NotifiedStateStore::Ops
{
	template <class Set, class Key>
	[[nodiscard]] bool ContainsAny(const Set& notifiedItems, const Key& primaryId, const Key& secondaryId) noexcept
	{
		return notifiedItems.contains(primaryId) || notifiedItems.contains(secondaryId);
	}

	template <class Set, class Key>
	void MarkPair(Set& notifiedItems, const Key& primaryId, const Key& secondaryId) noexcept
	{
		if (primaryId != 0) {
			notifiedItems.insert(primaryId);
		}
		if (secondaryId != 0) {
			notifiedItems.insert(secondaryId);
		}
	}

	template <class Set>
	void ReplaceAll(Set& notifiedItems, Set replacement) noexcept
	{
		notifiedItems = std::move(replacement);
	}

	template <class Set>
	[[nodiscard]] Set Snapshot(const Set& notifiedItems)
	{
		return notifiedItems;
	}

	template <class Set>
	[[nodiscard]] std::size_t Count(const Set& notifiedItems) noexcept
	{
		return notifiedItems.size();
	}
}
