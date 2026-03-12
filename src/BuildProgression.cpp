#include "CodexOfPowerNG/BuildProgression.h"

#include "CodexOfPowerNG/BuildOptionCatalog.h"
#include "CodexOfPowerNG/BuildStateStore.h"
#include "CodexOfPowerNG/Constants.h"

#include <RE/Skyrim.h>

#include <algorithm>
#include <limits>

namespace CodexOfPowerNG::BuildProgression
{
	namespace
	{
		[[nodiscard]] std::optional<Builds::BuildDiscipline> DefaultResolveLegacyDiscipline(RE::FormID formId) noexcept
		{
			const auto* form = RE::TESForm::LookupByID(formId);
			if (!form) {
				return std::nullopt;
			}
			return TryResolveDisciplineFromFormType(form->GetFormType());
		}

		[[nodiscard]] bool HasLegacyRewardTotals(const SerializationStateStore::Snapshot& snapshot) noexcept
		{
			return !snapshot.rewardTotals.empty();
		}

		[[nodiscard]] bool HasLegacyUndoRewardDeltas(const SerializationStateStore::Snapshot& snapshot) noexcept
		{
			return std::any_of(
				snapshot.undoHistory.begin(),
				snapshot.undoHistory.end(),
				[](const Registration::UndoRecord& record) { return !record.rewardDeltas.empty(); });
		}

		void ClearActiveSlots(SerializationStateStore::Snapshot& snapshot) noexcept
		{
			snapshot.activeBuildSlots = {};
		}

		void StripLegacyUndoRewardDeltas(SerializationStateStore::Snapshot& snapshot) noexcept
		{
			for (auto& record : snapshot.undoHistory) {
				record.rewardDeltas.clear();
			}
		}

		[[nodiscard]] std::string_view RemapLegacyOptionId(std::string_view optionId) noexcept
		{
			if (optionId == "build.defense.fireward" ||
			    optionId == "build.defense.frostward" ||
			    optionId == "build.defense.stormward") {
				return "build.defense.elementalWard";
			}
			if (optionId == "build.defense.antidote" ||
			    optionId == "build.defense.purity") {
				return "build.defense.purification";
			}
			return optionId;
		}

		void NormalizeActiveSlotOptionIds(SerializationStateStore::Snapshot& snapshot) noexcept
		{
			for (auto& slot : snapshot.activeBuildSlots) {
				if (slot.empty()) {
					continue;
				}
				const auto remapped = RemapLegacyOptionId(slot);
				if (remapped != slot) {
					slot = std::string(remapped);
				}
			}
		}

		void ApplyDerivedScore(
			SerializationStateStore::Snapshot& snapshot,
			Builds::BuildDiscipline           discipline) noexcept
		{
			switch (discipline) {
			case Builds::BuildDiscipline::Attack:
				++snapshot.attackScore;
				break;
			case Builds::BuildDiscipline::Defense:
				++snapshot.defenseScore;
				break;
			case Builds::BuildDiscipline::Utility:
				++snapshot.utilityScore;
				break;
			}
		}

		[[nodiscard]] std::uint32_t CurrentScore(Builds::BuildDiscipline discipline) noexcept
		{
			switch (discipline) {
			case Builds::BuildDiscipline::Attack:
				return BuildStateStore::GetAttackScore();
			case Builds::BuildDiscipline::Defense:
				return BuildStateStore::GetDefenseScore();
			case Builds::BuildDiscipline::Utility:
				return BuildStateStore::GetUtilityScore();
			}

			return 0u;
		}

		void SetScore(Builds::BuildDiscipline discipline, std::uint32_t score) noexcept
		{
			switch (discipline) {
			case Builds::BuildDiscipline::Attack:
				BuildStateStore::SetAttackScore(score);
				break;
			case Builds::BuildDiscipline::Defense:
				BuildStateStore::SetDefenseScore(score);
				break;
			case Builds::BuildDiscipline::Utility:
				BuildStateStore::SetUtilityScore(score);
				break;
			}
		}

		[[nodiscard]] const Builds::BuildOptionDef* FindOption(std::string_view optionId) noexcept
		{
			for (const auto& option : Builds::GetBuildOptionCatalog()) {
				if (option.id == optionId) {
					return &option;
				}
			}
			return nullptr;
		}

		[[nodiscard]] Builds::BuildSlotKind SlotKindForId(Builds::BuildSlotId slotId) noexcept
		{
			using namespace Builds;
			switch (slotId) {
			case BuildSlotId::Attack1:
			case BuildSlotId::Attack2:
				return BuildSlotKind::Attack;
			case BuildSlotId::Defense1:
				return BuildSlotKind::Defense;
			case BuildSlotId::Utility1:
			case BuildSlotId::Utility2:
				return BuildSlotKind::Utility;
			case BuildSlotId::Wildcard1:
				return BuildSlotKind::Wildcard;
			}

			return BuildSlotKind::Wildcard;
		}

		[[nodiscard]] bool IsSlotCompatible(
			const Builds::BuildOptionDef& option,
			Builds::BuildSlotId          slotId) noexcept
		{
			const auto slotKind = SlotKindForId(slotId);
			if (slotKind == Builds::BuildSlotKind::Wildcard) {
				return option.slotCompatibility == Builds::BuildSlotCompatibility::WildcardOnly ||
				       option.slotCompatibility == Builds::BuildSlotCompatibility::SameOrWildcard;
			}

			const bool matchesDiscipline =
				(slotKind == Builds::BuildSlotKind::Attack && option.discipline == Builds::BuildDiscipline::Attack) ||
				(slotKind == Builds::BuildSlotKind::Defense && option.discipline == Builds::BuildDiscipline::Defense) ||
				(slotKind == Builds::BuildSlotKind::Utility && option.discipline == Builds::BuildDiscipline::Utility);
			if (!matchesDiscipline) {
				return false;
			}

			return option.slotCompatibility == Builds::BuildSlotCompatibility::SameDisciplineOnly ||
			       option.slotCompatibility == Builds::BuildSlotCompatibility::SameOrWildcard;
		}
	}

	Builds::BuildDiscipline ConvertLegacyGroupToDiscipline(std::uint32_t legacyGroup) noexcept
	{
		switch (legacyGroup) {
		case 0u:
			return Builds::BuildDiscipline::Attack;
		case 1u:
			return Builds::BuildDiscipline::Defense;
		case 2u:
		case 3u:
		case 4u:
		case 5u:
		default:
			return Builds::BuildDiscipline::Utility;
		}
	}

	std::optional<Builds::BuildDiscipline> TryResolveDisciplineFromFormType(RE::FormType formType) noexcept
	{
		switch (formType) {
		case RE::FormType::Weapon:
		case RE::FormType::Ammo:
			return Builds::BuildDiscipline::Attack;
		case RE::FormType::Armor:
			return Builds::BuildDiscipline::Defense;
		case RE::FormType::AlchemyItem:
		case RE::FormType::Ingredient:
		case RE::FormType::Book:
		case RE::FormType::Scroll:
		case RE::FormType::SoulGem:
		case RE::FormType::Misc:
			return Builds::BuildDiscipline::Utility;
		default:
			return std::nullopt;
		}
	}

	std::optional<Builds::BuildDiscipline> TryResolveLegacyDiscipline(
		RE::FormID               regKey,
		std::uint32_t            legacyGroup,
		LegacyDisciplineResolver resolver) noexcept
	{
		if (legacyGroup <= 5u) {
			return ConvertLegacyGroupToDiscipline(legacyGroup);
		}

		const auto effectiveResolver = resolver ? resolver : DefaultResolveLegacyDiscipline;
		return effectiveResolver ? effectiveResolver(regKey) : std::nullopt;
	}

	void NormalizeLoadedSnapshot(
		SerializationStateStore::Snapshot& snapshot,
		LegacyDisciplineResolver           resolver) noexcept
	{
		NormalizeActiveSlotOptionIds(snapshot);

		if (snapshot.buildMigrationVersion >= kBuildMigrationVersion) {
			if (snapshot.buildMigrationState == Builds::BuildMigrationState::kPendingCleanup) {
				ClearActiveSlots(snapshot);
			} else if (snapshot.buildMigrationState == Builds::BuildMigrationState::kComplete) {
				StripLegacyUndoRewardDeltas(snapshot);
			}
			return;
		}

		const bool hadLegacyRegistrations = !snapshot.registeredItems.empty();
		const bool hadLegacyRewards = HasLegacyRewardTotals(snapshot);
		const bool hadLegacyUndo = HasLegacyUndoRewardDeltas(snapshot);

		snapshot.attackScore = 0u;
		snapshot.defenseScore = 0u;
		snapshot.utilityScore = 0u;
		ClearActiveSlots(snapshot);

		std::uint32_t unresolvedHistoricalRegistrations = 0u;
		for (const auto& [formId, group] : snapshot.registeredItems) {
			const auto discipline = TryResolveLegacyDiscipline(formId, group, resolver);
			if (!discipline.has_value()) {
				++unresolvedHistoricalRegistrations;
				continue;
			}
			ApplyDerivedScore(snapshot, discipline.value());
		}

		snapshot.buildMigrationVersion = kBuildMigrationVersion;
		snapshot.buildMigrationNotice = {};

		const bool performedMigration = hadLegacyRegistrations || hadLegacyRewards || hadLegacyUndo;
		if (!performedMigration) {
			snapshot.buildMigrationState = Builds::BuildMigrationState::kComplete;
			return;
		}

		snapshot.buildMigrationNotice.needsNotice = true;
		snapshot.buildMigrationNotice.legacyRewardsMigrated = hadLegacyRewards;
		snapshot.buildMigrationNotice.unresolvedHistoricalRegistrations = unresolvedHistoricalRegistrations;

		if (hadLegacyRewards) {
			snapshot.buildMigrationState = Builds::BuildMigrationState::kPendingCleanup;
			return;
		}

		snapshot.buildMigrationState = Builds::BuildMigrationState::kComplete;
		StripLegacyUndoRewardDeltas(snapshot);
	}

	bool TryFinalizePendingMigration(
		SerializationStateStore::Snapshot& snapshot,
		LegacyCleanupFn                    cleanup) noexcept
	{
		if (snapshot.buildMigrationVersion < kBuildMigrationVersion) {
			return false;
		}
		if (snapshot.buildMigrationState != Builds::BuildMigrationState::kPendingCleanup) {
			if (snapshot.buildMigrationState == Builds::BuildMigrationState::kComplete) {
				StripLegacyUndoRewardDeltas(snapshot);
			}
			return false;
		}

		ClearActiveSlots(snapshot);

		const bool cleaned =
			snapshot.rewardTotals.empty() ||
			(cleanup && cleanup(snapshot));
		if (!cleaned) {
			return false;
		}

		snapshot.rewardTotals.clear();
		snapshot.buildMigrationVersion = kBuildMigrationVersion;
		snapshot.buildMigrationState = Builds::BuildMigrationState::kComplete;
		StripLegacyUndoRewardDeltas(snapshot);
		return true;
	}

	std::optional<Registration::BuildScoreContribution> MakeRegistrationContribution(std::uint32_t group) noexcept
	{
		if (group > 5u) {
			return std::nullopt;
		}

		return Registration::BuildScoreContribution{
			.discipline = ConvertLegacyGroupToDiscipline(group),
			.scoreDelta = 1,
		};
	}

	bool ApplyRegistrationContribution(const Registration::BuildScoreContribution& contribution) noexcept
	{
		if (contribution.scoreDelta <= 0) {
			return false;
		}

		const auto current = CurrentScore(contribution.discipline);
		const auto delta = static_cast<std::uint32_t>(contribution.scoreDelta);
		const auto next = current > (std::numeric_limits<std::uint32_t>::max() - delta)
			? std::numeric_limits<std::uint32_t>::max()
			: (current + delta);
		SetScore(contribution.discipline, next);
		return true;
	}

	RollbackContributionResult RollbackRegistrationContributionDetailed(
		const Registration::BuildScoreContribution& contribution) noexcept
	{
		RollbackContributionResult result{};
		if (contribution.scoreDelta <= 0) {
			return result;
		}

		const auto current = CurrentScore(contribution.discipline);
		const auto delta = static_cast<std::uint32_t>(contribution.scoreDelta);
		const auto next = current > delta ? (current - delta) : 0u;
		result.scoreChanged = next != current;
		SetScore(contribution.discipline, next);

		for (const auto slotId : Builds::GetInitialBuildSlotLayout()) {
			const auto activeOption = BuildStateStore::GetActiveSlot(slotId);
			if (!activeOption.has_value()) {
				continue;
			}

			const auto* option = FindOption(activeOption.value());
			if (!option || !IsSlotCompatible(*option, slotId) || CurrentScore(option->discipline) < option->unlockScore) {
				BuildStateStore::ClearActiveSlot(slotId);
				++result.deactivatedSlots;
			}
		}
		return result;
	}

	std::size_t RollbackRegistrationContribution(const Registration::BuildScoreContribution& contribution) noexcept
	{
		return RollbackRegistrationContributionDetailed(contribution).deactivatedSlots;
	}

	std::size_t CountUnlockedBaselineMilestones(
		Builds::BuildDiscipline discipline,
		std::uint32_t           score) noexcept
	{
		std::size_t count = 0u;
		for (const auto& milestone : Builds::GetBuildBaselineMilestones()) {
			if (milestone.discipline == discipline && score >= milestone.threshold) {
				++count;
			}
		}
		return count;
	}

	bool BaselineMilestonesChanged(
		Builds::BuildDiscipline discipline,
		std::uint32_t           previousScore,
		std::uint32_t           nextScore) noexcept
	{
		return CountUnlockedBaselineMilestones(discipline, previousScore) !=
		       CountUnlockedBaselineMilestones(discipline, nextScore);
	}

	std::optional<Builds::BuildMigrationNoticeSnapshot> ConsumeMigrationNotice(
		SerializationStateStore::Snapshot& snapshot) noexcept
	{
		if (!snapshot.buildMigrationNotice.needsNotice) {
			return std::nullopt;
		}

		auto notice = snapshot.buildMigrationNotice;
		snapshot.buildMigrationNotice.needsNotice = false;
		return notice;
	}
}
