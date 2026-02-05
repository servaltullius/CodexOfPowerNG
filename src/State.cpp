#include "CodexOfPowerNG/State.h"

namespace CodexOfPowerNG
{
	RuntimeState& GetState() noexcept
	{
		static RuntimeState state{};
		return state;
	}
}

