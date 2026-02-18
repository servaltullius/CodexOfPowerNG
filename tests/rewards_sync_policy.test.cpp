#include "CodexOfPowerNG/RewardsSyncPolicy.h"

#include <cassert>

int main()
{
	using CodexOfPowerNG::Rewards::DecideSyncRequestAction;
	using CodexOfPowerNG::Rewards::NextMissingStreak;
	using CodexOfPowerNG::Rewards::ShouldApplyAfterStreak;
	using CodexOfPowerNG::Rewards::SyncRequestAction;

	// Request action decisions.
	assert(DecideSyncRequestAction(false, 0, 1000, 15000) == SyncRequestAction::kStartNewRun);
	assert(DecideSyncRequestAction(true, 1000, 2000, 15000) == SyncRequestAction::kMarkRerun);
	assert(DecideSyncRequestAction(true, 1000, 17001, 15000) == SyncRequestAction::kForceRestartAndStart);

	// Missing streak progression.
	assert(NextMissingStreak(0.0f, 3) == 0);
	assert(NextMissingStreak(0.5f, 0) == 1);
	assert(NextMissingStreak(-0.5f, 1) == 2);

	// Apply threshold gating.
	assert(!ShouldApplyAfterStreak(0.5f, 2, 3));
	assert(ShouldApplyAfterStreak(0.5f, 3, 3));
	assert(ShouldApplyAfterStreak(-0.5f, 3, 3));
	assert(!ShouldApplyAfterStreak(0.0f, 5, 3));

	return 0;
}
