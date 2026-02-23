#include "CodexOfPowerNG/RewardsSyncRuntime.h"

#include <cassert>

int main()
{
	using CodexOfPowerNG::Rewards::SyncRequestAction;
	namespace SyncRuntime = CodexOfPowerNG::Rewards::SyncRuntime;

	SyncRuntime::ResetForTesting(10);

	auto snap = SyncRuntime::Snapshot();
	assert(snap.generation == 10);
	assert(!snap.rewardSyncScheduled);
	assert(!snap.carryWeightQuickResyncScheduled);
	assert(SyncRuntime::IsCurrentGeneration(10));
	assert(!SyncRuntime::IsCurrentGeneration(11));

	assert(SyncRuntime::DecideRewardSyncRequestAction(1'000, 15'000) == SyncRequestAction::kStartNewRun);
	assert(
		SyncRuntime::DecideCarryWeightQuickResyncRequestAction(1'000, 3'000) ==
		SyncRequestAction::kStartNewRun);

	assert(SyncRuntime::TryStartRewardSync(2'000));
	snap = SyncRuntime::Snapshot();
	assert(snap.rewardSyncScheduled);
	assert(snap.rewardSyncScheduledSinceMs == 2'000);

	assert(SyncRuntime::DecideRewardSyncRequestAction(2'500, 15'000) == SyncRequestAction::kMarkRerun);
	assert(!SyncRuntime::TryStartRewardSync(2'600));
	snap = SyncRuntime::Snapshot();
	assert(snap.rewardSyncRerunRequested);
	assert(SyncRuntime::ConsumeRewardSyncRerunRequested());
	assert(!SyncRuntime::ConsumeRewardSyncRerunRequested());

	SyncRuntime::TouchRewardSyncScheduledSince(3'000);
	assert(SyncRuntime::DecideRewardSyncRequestAction(18'500, 15'000) == SyncRequestAction::kForceRestartAndStart);

	SyncRuntime::CompleteRewardSyncRun(9);
	assert(SyncRuntime::Snapshot().rewardSyncScheduled);
	SyncRuntime::CompleteRewardSyncRun(10);
	assert(!SyncRuntime::Snapshot().rewardSyncScheduled);

	assert(SyncRuntime::TryStartCarryWeightQuickResync(4'000));
	assert(!SyncRuntime::TryStartCarryWeightQuickResync(4'100));
	assert(SyncRuntime::ConsumeCarryWeightQuickResyncRerunRequested());
	assert(!SyncRuntime::ConsumeCarryWeightQuickResyncRerunRequested());
	SyncRuntime::TouchCarryWeightQuickResyncScheduledSince(4'500);
	assert(
		SyncRuntime::DecideCarryWeightQuickResyncRequestAction(7'600, 3'000) ==
		SyncRequestAction::kForceRestartAndStart);

	SyncRuntime::CompleteCarryWeightQuickResyncRun(9);
	assert(SyncRuntime::Snapshot().carryWeightQuickResyncScheduled);
	SyncRuntime::CompleteCarryWeightQuickResyncRun(10);
	assert(!SyncRuntime::Snapshot().carryWeightQuickResyncScheduled);

	assert(SyncRuntime::TryStartRewardSync(6'000));
	assert(SyncRuntime::TryStartCarryWeightQuickResync(6'100));
	const auto nextGeneration = SyncRuntime::BumpGenerationAndClearSchedulers();
	assert(nextGeneration == 11);
	snap = SyncRuntime::Snapshot();
	assert(snap.generation == 11);
	assert(!snap.rewardSyncScheduled);
	assert(!snap.rewardSyncRerunRequested);
	assert(!snap.carryWeightQuickResyncScheduled);
	assert(!snap.carryWeightQuickResyncRerunRequested);
	assert(snap.rewardSyncScheduledSinceMs == 0);
	assert(snap.carryWeightQuickResyncScheduledSinceMs == 0);

	return 0;
}
