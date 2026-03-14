#include "SerializationInternal.h"

#include "CodexOfPowerNG/Constants.h"
#include "CodexOfPowerNG/Registration.h"
#include "CodexOfPowerNG/SerializationStateStore.h"
#include "CodexOfPowerNG/SerializationWriteFlow.h"

#include <SKSE/Interfaces.h>
#include <SKSE/Logger.h>

#include <cstdint>

namespace CodexOfPowerNG::Serialization::Internal
{
	namespace
	{
		inline constexpr std::uint32_t kUndoRecordVersion = 4u;

		[[nodiscard]] bool WriteString(SKSE::SerializationInterface* a_intfc, const std::string& value) noexcept
		{
			const auto length = static_cast<std::uint32_t>(value.size());
			if (!a_intfc->WriteRecordData(length)) {
				return false;
			}
			if (length == 0) {
				return true;
			}
			return a_intfc->WriteRecordData(value.data(), length);
		}

		[[nodiscard]] bool WriteRegisteredRecord(SKSE::SerializationInterface* a_intfc,
			const SerializationStateStore::Snapshot& state) noexcept
		{
			if (!a_intfc->OpenRecord(kRecordRegisteredItems, kSerializationVersion)) {
				SKSE::log::error("Failed to open co-save record REGI");
				return false;
			}

			const std::uint32_t count = static_cast<std::uint32_t>(state.registeredItems.size());
			if (!a_intfc->WriteRecordData(count)) {
				SKSE::log::error("Failed to write registered count");
				return false;
			}

			for (const auto& [formId, group] : state.registeredItems) {
				if (!a_intfc->WriteRecordData(formId) || !a_intfc->WriteRecordData(group)) {
					SKSE::log::error("Failed to write registered entry");
					return false;
				}
			}
			return true;
		}

		[[nodiscard]] bool WriteBlockedRecord(SKSE::SerializationInterface* a_intfc,
			const SerializationStateStore::Snapshot& state) noexcept
		{
			if (!a_intfc->OpenRecord(kRecordBlockedItems, kSerializationVersion)) {
				SKSE::log::error("Failed to open co-save record BLCK");
				return false;
			}
			const std::uint32_t count = static_cast<std::uint32_t>(state.blockedItems.size());
			if (!a_intfc->WriteRecordData(count)) {
				SKSE::log::error("Failed to write blocked count");
				return false;
			}
			for (auto formId : state.blockedItems) {
				if (!a_intfc->WriteRecordData(formId)) {
					SKSE::log::error("Failed to write blocked formId");
					return false;
				}
			}
			return true;
		}

		[[nodiscard]] bool WriteNotifiedRecord(SKSE::SerializationInterface* a_intfc,
			const SerializationStateStore::Snapshot& state) noexcept
		{
			if (!a_intfc->OpenRecord(kRecordNotifiedItems, kSerializationVersion)) {
				SKSE::log::error("Failed to open co-save record NTFY");
				return false;
			}
			const std::uint32_t count = static_cast<std::uint32_t>(state.notifiedItems.size());
			if (!a_intfc->WriteRecordData(count)) {
				SKSE::log::error("Failed to write notified count");
				return false;
			}
			for (auto formId : state.notifiedItems) {
				if (!a_intfc->WriteRecordData(formId)) {
					SKSE::log::error("Failed to write notified formId");
					return false;
				}
			}
			return true;
		}

		[[nodiscard]] bool WriteBuildScoresRecord(
			SKSE::SerializationInterface*          a_intfc,
			const SerializationStateStore::Snapshot& state) noexcept
		{
			if (!a_intfc->OpenRecord(kRecordBuildScores, 2u)) {
				SKSE::log::error("Failed to open co-save record BSCR");
				return false;
			}
			return a_intfc->WriteRecordData(state.attackScore) &&
			       a_intfc->WriteRecordData(state.defenseScore) &&
			       a_intfc->WriteRecordData(state.utilityScore) &&
			       a_intfc->WriteRecordData(state.attackBuildPointsCenti) &&
			       a_intfc->WriteRecordData(state.defenseBuildPointsCenti) &&
			       a_intfc->WriteRecordData(state.utilityBuildPointsCenti);
		}

		[[nodiscard]] bool WriteBuildSlotsRecord(
			SKSE::SerializationInterface*          a_intfc,
			const SerializationStateStore::Snapshot& state) noexcept
		{
			if (!a_intfc->OpenRecord(kRecordBuildSlots, 1u)) {
				SKSE::log::error("Failed to open co-save record BSLT");
				return false;
			}

			const auto slotCount = static_cast<std::uint32_t>(state.activeBuildSlots.size());
			if (!a_intfc->WriteRecordData(slotCount)) {
				SKSE::log::error("Failed to write build slot count");
				return false;
			}

			for (const auto& slot : state.activeBuildSlots) {
				if (!WriteString(a_intfc, slot)) {
					SKSE::log::error("Failed to write build slot entry");
					return false;
				}
			}
			return true;
		}

		[[nodiscard]] bool WriteBuildMigrationRecord(
			SKSE::SerializationInterface*          a_intfc,
			const SerializationStateStore::Snapshot& state) noexcept
		{
			if (!a_intfc->OpenRecord(kRecordBuildMigration, 1u)) {
				SKSE::log::error("Failed to open co-save record BMIG");
				return false;
			}

			const auto migrationState = static_cast<std::uint32_t>(state.buildMigrationState);
			const std::uint8_t needsNotice = state.buildMigrationNotice.needsNotice ? 1u : 0u;
			const std::uint8_t legacyRewardsMigrated = state.buildMigrationNotice.legacyRewardsMigrated ? 1u : 0u;
			return a_intfc->WriteRecordData(state.buildMigrationVersion) &&
			       a_intfc->WriteRecordData(migrationState) &&
			       a_intfc->WriteRecordData(needsNotice) &&
			       a_intfc->WriteRecordData(legacyRewardsMigrated) &&
			       a_intfc->WriteRecordData(state.buildMigrationNotice.unresolvedHistoricalRegistrations);
		}

		[[nodiscard]] bool WriteBuildAppliedEffectsRecord(
			SKSE::SerializationInterface*           a_intfc,
			const SerializationStateStore::Snapshot& state) noexcept
		{
			if (!a_intfc->OpenRecord(kRecordBuildAppliedEffects, 1u)) {
				SKSE::log::error("Failed to open co-save record BEFX");
				return false;
			}

			const std::uint32_t count = static_cast<std::uint32_t>(state.buildAppliedEffectTotals.size());
			if (!a_intfc->WriteRecordData(count)) {
				SKSE::log::error("Failed to write build applied effect count");
				return false;
			}

			for (const auto& [av, total] : state.buildAppliedEffectTotals) {
				const auto avRaw = static_cast<std::uint32_t>(av);
				if (!a_intfc->WriteRecordData(avRaw) || !a_intfc->WriteRecordData(total)) {
					SKSE::log::error("Failed to write build applied effect entry");
					return false;
				}
			}
			return true;
		}

		[[nodiscard]] bool WriteRewardsRecord(SKSE::SerializationInterface* a_intfc,
			const SerializationStateStore::Snapshot& state) noexcept
		{
			if (!a_intfc->OpenRecord(kRecordRewards, kSerializationVersion)) {
				SKSE::log::error("Failed to open co-save record RWDS");
				return false;
			}
			const std::uint32_t count = static_cast<std::uint32_t>(state.rewardTotals.size());
			if (!a_intfc->WriteRecordData(count)) {
				SKSE::log::error("Failed to write reward count");
				return false;
			}

			for (const auto& [av, total] : state.rewardTotals) {
				const auto avRaw = static_cast<std::uint32_t>(av);
				if (!a_intfc->WriteRecordData(avRaw) || !a_intfc->WriteRecordData(total)) {
					SKSE::log::error("Failed to write reward entry");
					return false;
				}
			}
			return true;
		}

		[[nodiscard]] bool WriteUndoRecord(SKSE::SerializationInterface* a_intfc,
			const SerializationStateStore::Snapshot& state) noexcept
		{
			if (!a_intfc->OpenRecord(kRecordUndoHistory, kUndoRecordVersion)) {
				SKSE::log::error("Failed to open co-save record UNDO");
				return false;
			}

			const std::uint32_t count = static_cast<std::uint32_t>(state.undoHistory.size());
			if (!a_intfc->WriteRecordData(count) || !a_intfc->WriteRecordData(state.undoNextActionId)) {
				SKSE::log::error("Failed to write undo header");
				return false;
			}

			for (const auto& entry : state.undoHistory) {
				const std::uint32_t hasBuildContribution = entry.buildContribution.has_value() ? 1u : 0u;
				const std::uint32_t disciplineRaw = entry.buildContribution.has_value()
					? static_cast<std::uint32_t>(entry.buildContribution->discipline)
					: 0u;
				const std::int32_t recordDelta = entry.buildContribution.has_value()
					? entry.buildContribution->recordDelta
					: 0;
				const std::int32_t pointsDeltaCenti = entry.buildContribution.has_value()
					? entry.buildContribution->pointsDeltaCenti
					: 0;
				const std::uint32_t rewardCount = static_cast<std::uint32_t>(entry.rewardDeltas.size());
				if (!a_intfc->WriteRecordData(entry.actionId) ||
				    !a_intfc->WriteRecordData(entry.formId) ||
				    !a_intfc->WriteRecordData(entry.regKey) ||
				    !a_intfc->WriteRecordData(entry.group) ||
				    !a_intfc->WriteRecordData(hasBuildContribution) ||
				    !a_intfc->WriteRecordData(disciplineRaw) ||
				    !a_intfc->WriteRecordData(recordDelta) ||
				    !a_intfc->WriteRecordData(pointsDeltaCenti) ||
				    !a_intfc->WriteRecordData(rewardCount)) {
					SKSE::log::error("Failed to write undo entry header");
					return false;
				}

				for (const auto& delta : entry.rewardDeltas) {
					const auto avRaw = static_cast<std::uint32_t>(delta.av);
					if (!a_intfc->WriteRecordData(avRaw) || !a_intfc->WriteRecordData(delta.delta)) {
						SKSE::log::error("Failed to write undo reward delta");
						return false;
					}
				}
			}
			return true;
		}
	}

	void Revert(SKSE::SerializationInterface* /*a_intfc*/) noexcept
	{
		SerializationStateStore::Clear();
		Registration::InvalidateQuickRegisterCache();
	}

	void Save(SKSE::SerializationInterface* a_intfc) noexcept
	{
		const auto state = SerializationStateStore::SnapshotState();

		const bool allOk = ExecuteAllSaveWriters(
			[&]() { return WriteRegisteredRecord(a_intfc, state); },
			[&]() { return WriteBlockedRecord(a_intfc, state); },
			[&]() { return WriteNotifiedRecord(a_intfc, state); },
			[&]() { return WriteBuildScoresRecord(a_intfc, state); },
			[&]() { return WriteBuildSlotsRecord(a_intfc, state); },
			[&]() { return WriteBuildMigrationRecord(a_intfc, state); },
			[&]() { return WriteBuildAppliedEffectsRecord(a_intfc, state); },
			[&]() { return WriteRewardsRecord(a_intfc, state); },
			[&]() { return WriteUndoRecord(a_intfc, state); });
		if (!allOk) {
			SKSE::log::error("Serialization save completed with writer failures; co-save may be incomplete");
		}
	}
}
