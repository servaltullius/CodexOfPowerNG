#include "CodexOfPowerNG/BuildStateStore.h"

#include "CodexOfPowerNG/BuildOptionCatalog.h"
#include "CodexOfPowerNG/State.h"

#include <array>

namespace CodexOfPowerNG::BuildStateStore
{
	namespace
	{
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

		[[nodiscard]] std::uint32_t ScoreForDiscipline(const RuntimeState& state, Builds::BuildDiscipline discipline) noexcept
		{
			using Builds::BuildDiscipline;
			switch (discipline) {
			case BuildDiscipline::Attack:
				return state.attackScore;
			case BuildDiscipline::Defense:
				return state.defenseScore;
			case BuildDiscipline::Utility:
				return state.utilityScore;
			}

			return 0u;
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

	std::uint32_t GetAttackScore() noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		return state.attackScore;
	}

	std::uint32_t GetDefenseScore() noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		return state.defenseScore;
	}

	std::uint32_t GetUtilityScore() noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		return state.utilityScore;
	}

	void SetAttackScore(std::uint32_t score) noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		state.attackScore = score;
	}

	void SetDefenseScore(std::uint32_t score) noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		state.defenseScore = score;
	}

	void SetUtilityScore(std::uint32_t score) noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		state.utilityScore = score;
	}

	std::optional<std::string> GetActiveSlot(Builds::BuildSlotId slotId) noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);

		const auto& value = state.activeBuildSlots[Builds::ToIndex(slotId)];
		if (value.empty()) {
			return std::nullopt;
		}
		return value;
	}

	bool SetActiveSlot(Builds::BuildSlotId slotId, std::string_view optionId) noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);

		if (state.buildMigrationState == Builds::BuildMigrationState::kPendingCleanup) {
			return false;
		}

		const auto* option = FindOption(optionId);
		if (!option) {
			return false;
		}
		if (!IsSlotCompatible(*option, slotId)) {
			return false;
		}
		if (ScoreForDiscipline(state, option->discipline) < option->unlockScore) {
			return false;
		}

		state.activeBuildSlots[Builds::ToIndex(slotId)] = std::string(optionId);
		return true;
	}

	void ClearActiveSlot(Builds::BuildSlotId slotId) noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		state.activeBuildSlots[Builds::ToIndex(slotId)].clear();
	}

	void ClearActiveSlots() noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		for (auto& slot : state.activeBuildSlots) {
			slot.clear();
		}
	}

	Builds::BuildMigrationState MigrationState() noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		return state.buildMigrationState;
	}

	std::uint32_t MigrationVersion() noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		return state.buildMigrationVersion;
	}

	Builds::BuildMigrationNoticeSnapshot GetMigrationNoticeSnapshot() noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		return state.buildMigrationNotice;
	}

	void SetMigrationState(Builds::BuildMigrationState migrationState) noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		state.buildMigrationState = migrationState;
	}

	void SetMigrationVersion(std::uint32_t version) noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		state.buildMigrationVersion = version;
	}

	void SetMigrationNoticeSnapshot(Builds::BuildMigrationNoticeSnapshot snapshot) noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		state.buildMigrationNotice = snapshot;
	}
}
