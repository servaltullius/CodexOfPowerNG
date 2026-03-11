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

	template <class... Writers>
	[[nodiscard]] inline bool ExecuteAllSaveWriters(
		Writers&&... writers)
	{
		bool allOk = true;
		((allOk = InvokeSaveWriter(std::forward<Writers>(writers)) && allOk), ...);
		return allOk;
	}
}
