#pragma once

#include <type_traits>
#include <utility>

namespace CodexOfPowerNG::Serialization
{
	template <class Fn>
	[[nodiscard]] inline bool InvokeSaveWriter(Fn&& fn)
	{
		using Result = std::invoke_result_t<Fn>;
		if constexpr (std::is_same_v<Result, bool>) {
			return std::forward<Fn>(fn)();
		} else {
			std::forward<Fn>(fn)();
			return true;
		}
	}

	template <class WriteRegistered, class WriteBlocked, class WriteNotified, class WriteRewards, class WriteUndo>
	[[nodiscard]] inline bool ExecuteAllSaveWriters(
		WriteRegistered&& writeRegistered,
		WriteBlocked&& writeBlocked,
		WriteNotified&& writeNotified,
		WriteRewards&& writeRewards,
		WriteUndo&& writeUndo)
	{
		bool allOk = true;
		allOk = InvokeSaveWriter(std::forward<WriteRegistered>(writeRegistered)) && allOk;
		allOk = InvokeSaveWriter(std::forward<WriteBlocked>(writeBlocked)) && allOk;
		allOk = InvokeSaveWriter(std::forward<WriteNotified>(writeNotified)) && allOk;
		allOk = InvokeSaveWriter(std::forward<WriteRewards>(writeRewards)) && allOk;
		allOk = InvokeSaveWriter(std::forward<WriteUndo>(writeUndo)) && allOk;
		return allOk;
	}
}
