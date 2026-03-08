#include "CodexOfPowerNG/SerializationStateStore.h"

#include "CodexOfPowerNG/SerializationStateStoreOps.h"
#include "CodexOfPowerNG/State.h"

#include <utility>

namespace CodexOfPowerNG::SerializationStateStore
{
	Snapshot SnapshotState() noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		return Ops::SnapshotState<Snapshot>(state);
	}

	void ReplaceState(Snapshot snapshot) noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		Ops::ReplaceState(state, std::move(snapshot));
	}

	void Clear() noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		Ops::Clear(state);
	}
}
