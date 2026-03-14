#include "PrismaUIInternal.h"

#include "CodexOfPowerNG/Config.h"
#include "CodexOfPowerNG/BuildStateStore.h"
#include "CodexOfPowerNG/L10n.h"
#include "CodexOfPowerNG/Registration.h"
#include "CodexOfPowerNG/RegistrationStateStore.h"
#include "CodexOfPowerNG/Rewards.h"

namespace CodexOfPowerNG::PrismaUIManager::Internal
{
	json BuildRuntimeStatePayload()
	{
		const auto registeredCount = RegistrationStateStore::RegisteredCount();
		const auto rewardCount = Rewards::SnapshotTrackedRewardCount();

		// Avoid querying PrismaUI view state on-demand here; Focus/Show timing can
		// conflict with internal view task processing and appear as a stall.
		const bool focused = IsViewFocused();
		const bool hidden = IsViewHidden();

		const auto settings = GetSettings();

		json j;
		j["ui"] = {
			{ "ready", IsDomReady() },
			{ "focused", focused },
			{ "hidden", hidden },
		};
		j["registeredCount"] = registeredCount;
		j["rewardCount"] = rewardCount;
		const auto migrationNotice = BuildStateStore::GetMigrationNoticeSnapshot();
		const auto attackScore = BuildStateStore::GetAttackScore();
		const auto defenseScore = BuildStateStore::GetDefenseScore();
		const auto utilityScore = BuildStateStore::GetUtilityScore();
		const auto attackBuildPointsCenti = BuildStateStore::GetAttackBuildPointsCenti();
		const auto defenseBuildPointsCenti = BuildStateStore::GetDefenseBuildPointsCenti();
		const auto utilityBuildPointsCenti = BuildStateStore::GetUtilityBuildPointsCenti();
		j["buildSummary"] = {
			{ "attackScore", attackScore },
			{ "defenseScore", defenseScore },
			{ "utilityScore", utilityScore },
			{ "totalScore", attackScore + defenseScore + utilityScore },
			{ "attackBuildPoints", Builds::FromBuildPointCenti(attackBuildPointsCenti) },
			{ "defenseBuildPoints", Builds::FromBuildPointCenti(defenseBuildPointsCenti) },
			{ "utilityBuildPoints", Builds::FromBuildPointCenti(utilityBuildPointsCenti) },
			{ "totalBuildPoints", Builds::FromBuildPointCenti(
				  attackBuildPointsCenti + defenseBuildPointsCenti + utilityBuildPointsCenti) },
		};
		j["buildMigration"] = {
			{ "state", static_cast<std::uint32_t>(BuildStateStore::MigrationState()) },
			{ "version", BuildStateStore::MigrationVersion() },
			{ "needsNotice", migrationNotice.needsNotice },
			{ "legacyRewardsMigrated", migrationNotice.legacyRewardsMigrated },
			{ "unresolvedHistoricalRegistrations", migrationNotice.unresolvedHistoricalRegistrations },
		};
		j["language"] = L10n::ActiveLanguage();
		j["toggleKeyCode"] = settings.toggleKeyCode;
		const bool tccListsAvailable = Registration::IsTccDisplayedListsAvailable();
		j["lotdGate"] = {
			{ "requireTccDisplayed", settings.requireTccDisplayed },
			{ "tccListsAvailable", tccListsAvailable },
			{ "blocking", settings.requireTccDisplayed && !tccListsAvailable },
		};
		return j;
	}
}
