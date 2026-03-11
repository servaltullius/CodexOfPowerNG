#pragma once

#include "CodexOfPowerNG/SerializationStateStore.h"

#include <optional>

namespace CodexOfPowerNG::BuildProgression
{
	inline constexpr std::uint32_t kBuildMigrationVersion = 1;

	using LegacyDisciplineResolver = std::optional<Builds::BuildDiscipline> (*)(RE::FormID formId) noexcept;
	using LegacyCleanupFn = bool (*)(const SerializationStateStore::Snapshot& snapshot) noexcept;

	struct RollbackContributionResult
	{
		bool        scoreChanged{ false };
		std::size_t deactivatedSlots{ 0u };
	};

	[[nodiscard]] Builds::BuildDiscipline ConvertLegacyGroupToDiscipline(std::uint32_t legacyGroup) noexcept;
	[[nodiscard]] std::optional<Builds::BuildDiscipline> TryResolveDisciplineFromFormType(RE::FormType formType) noexcept;
	[[nodiscard]] std::optional<Builds::BuildDiscipline> TryResolveLegacyDiscipline(
		RE::FormID               regKey,
		std::uint32_t            legacyGroup,
		LegacyDisciplineResolver resolver = nullptr) noexcept;

	void NormalizeLoadedSnapshot(
		SerializationStateStore::Snapshot& snapshot,
		LegacyDisciplineResolver           resolver = nullptr) noexcept;

	[[nodiscard]] bool TryFinalizePendingMigration(
		SerializationStateStore::Snapshot& snapshot,
		LegacyCleanupFn                    cleanup) noexcept;

	[[nodiscard]] std::optional<Registration::BuildScoreContribution> MakeRegistrationContribution(
		std::uint32_t group) noexcept;
	[[nodiscard]] bool ApplyRegistrationContribution(
		const Registration::BuildScoreContribution& contribution) noexcept;
	[[nodiscard]] RollbackContributionResult RollbackRegistrationContributionDetailed(
		const Registration::BuildScoreContribution& contribution) noexcept;
	[[nodiscard]] std::size_t RollbackRegistrationContribution(
		const Registration::BuildScoreContribution& contribution) noexcept;
	[[nodiscard]] std::size_t CountUnlockedBaselineMilestones(
		Builds::BuildDiscipline discipline,
		std::uint32_t           score) noexcept;
	[[nodiscard]] bool BaselineMilestonesChanged(
		Builds::BuildDiscipline discipline,
		std::uint32_t           previousScore,
		std::uint32_t           nextScore) noexcept;

	[[nodiscard]] std::optional<Builds::BuildMigrationNoticeSnapshot> ConsumeMigrationNotice(
		SerializationStateStore::Snapshot& snapshot) noexcept;
}
