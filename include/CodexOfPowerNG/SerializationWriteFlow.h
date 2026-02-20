#pragma once

#include <utility>

namespace CodexOfPowerNG::Serialization
{
	template <class WriteRegistered, class WriteBlocked, class WriteNotified, class WriteRewards, class WriteUndo>
	inline void ExecuteAllSaveWriters(
		WriteRegistered&& writeRegistered,
		WriteBlocked&& writeBlocked,
		WriteNotified&& writeNotified,
		WriteRewards&& writeRewards,
		WriteUndo&& writeUndo)
	{
		std::forward<WriteRegistered>(writeRegistered)();
		std::forward<WriteBlocked>(writeBlocked)();
		std::forward<WriteNotified>(writeNotified)();
		std::forward<WriteRewards>(writeRewards)();
		std::forward<WriteUndo>(writeUndo)();
	}
}
