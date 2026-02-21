#include "SerializationInternal.h"

#include "CodexOfPowerNG/Constants.h"
#include "CodexOfPowerNG/Registration.h"
#include "CodexOfPowerNG/SerializationWriteFlow.h"
#include "CodexOfPowerNG/State.h"

#include <SKSE/Interfaces.h>
#include <SKSE/Logger.h>

namespace CodexOfPowerNG::Serialization::Internal
{
	namespace
	{
		[[nodiscard]] bool WriteRegisteredRecord(SKSE::SerializationInterface* a_intfc, const RuntimeState& state) noexcept
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

		[[nodiscard]] bool WriteBlockedRecord(SKSE::SerializationInterface* a_intfc, const RuntimeState& state) noexcept
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

		[[nodiscard]] bool WriteNotifiedRecord(SKSE::SerializationInterface* a_intfc, const RuntimeState& state) noexcept
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

		[[nodiscard]] bool WriteRewardsRecord(SKSE::SerializationInterface* a_intfc, const RuntimeState& state) noexcept
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

		[[nodiscard]] bool WriteUndoRecord(SKSE::SerializationInterface* a_intfc, const RuntimeState& state) noexcept
		{
			if (!a_intfc->OpenRecord(kRecordUndoHistory, kSerializationVersion)) {
				SKSE::log::error("Failed to open co-save record UNDO");
				return false;
			}

			const std::uint32_t count = static_cast<std::uint32_t>(state.undoHistory.size());
			if (!a_intfc->WriteRecordData(count) || !a_intfc->WriteRecordData(state.undoNextActionId)) {
				SKSE::log::error("Failed to write undo header");
				return false;
			}

			for (const auto& entry : state.undoHistory) {
				const std::uint32_t rewardCount = static_cast<std::uint32_t>(entry.rewardDeltas.size());
				if (!a_intfc->WriteRecordData(entry.actionId) ||
				    !a_intfc->WriteRecordData(entry.formId) ||
				    !a_intfc->WriteRecordData(entry.regKey) ||
				    !a_intfc->WriteRecordData(entry.group) ||
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
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		state.registeredItems.clear();
		state.blockedItems.clear();
		state.notifiedItems.clear();
		state.rewardTotals.clear();
		state.undoHistory.clear();
		state.undoNextActionId = 1;
		Registration::InvalidateQuickRegisterCache();
	}

	void Save(SKSE::SerializationInterface* a_intfc) noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);

		const bool allOk = ExecuteAllSaveWriters(
			[&]() { return WriteRegisteredRecord(a_intfc, state); },
			[&]() { return WriteBlockedRecord(a_intfc, state); },
			[&]() { return WriteNotifiedRecord(a_intfc, state); },
			[&]() { return WriteRewardsRecord(a_intfc, state); },
			[&]() { return WriteUndoRecord(a_intfc, state); });
		if (!allOk) {
			SKSE::log::error("Serialization save completed with writer failures; co-save may be incomplete");
		}
	}
}
