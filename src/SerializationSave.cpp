#include "SerializationInternal.h"

#include "CodexOfPowerNG/Constants.h"
#include "CodexOfPowerNG/SerializationWriteFlow.h"
#include "CodexOfPowerNG/State.h"

#include <SKSE/Interfaces.h>
#include <SKSE/Logger.h>

namespace CodexOfPowerNG::Serialization::Internal
{
	namespace
	{
		void WriteRegisteredRecord(SKSE::SerializationInterface* a_intfc, const RuntimeState& state) noexcept
		{
			if (!a_intfc->OpenRecord(kRecordRegisteredItems, kSerializationVersion)) {
				SKSE::log::error("Failed to open co-save record REGI");
				return;
			}

			const std::uint32_t count = static_cast<std::uint32_t>(state.registeredItems.size());
			if (!a_intfc->WriteRecordData(count)) {
				SKSE::log::error("Failed to write registered count");
				return;
			}

			for (const auto& [formId, group] : state.registeredItems) {
				if (!a_intfc->WriteRecordData(formId) || !a_intfc->WriteRecordData(group)) {
					SKSE::log::error("Failed to write registered entry");
					return;
				}
			}
		}

		void WriteBlockedRecord(SKSE::SerializationInterface* a_intfc, const RuntimeState& state) noexcept
		{
			if (!a_intfc->OpenRecord(kRecordBlockedItems, kSerializationVersion)) {
				SKSE::log::error("Failed to open co-save record BLCK");
				return;
			}
			const std::uint32_t count = static_cast<std::uint32_t>(state.blockedItems.size());
			if (!a_intfc->WriteRecordData(count)) {
				SKSE::log::error("Failed to write blocked count");
				return;
			}
			for (auto formId : state.blockedItems) {
				if (!a_intfc->WriteRecordData(formId)) {
					SKSE::log::error("Failed to write blocked formId");
					return;
				}
			}
		}

		void WriteNotifiedRecord(SKSE::SerializationInterface* a_intfc, const RuntimeState& state) noexcept
		{
			if (!a_intfc->OpenRecord(kRecordNotifiedItems, kSerializationVersion)) {
				SKSE::log::error("Failed to open co-save record NTFY");
				return;
			}
			const std::uint32_t count = static_cast<std::uint32_t>(state.notifiedItems.size());
			if (!a_intfc->WriteRecordData(count)) {
				SKSE::log::error("Failed to write notified count");
				return;
			}
			for (auto formId : state.notifiedItems) {
				if (!a_intfc->WriteRecordData(formId)) {
					SKSE::log::error("Failed to write notified formId");
					return;
				}
			}
		}

		void WriteRewardsRecord(SKSE::SerializationInterface* a_intfc, const RuntimeState& state) noexcept
		{
			if (!a_intfc->OpenRecord(kRecordRewards, kSerializationVersion)) {
				SKSE::log::error("Failed to open co-save record RWDS");
				return;
			}
			const std::uint32_t count = static_cast<std::uint32_t>(state.rewardTotals.size());
			if (!a_intfc->WriteRecordData(count)) {
				SKSE::log::error("Failed to write reward count");
				return;
			}

			for (const auto& [av, total] : state.rewardTotals) {
				const auto avRaw = static_cast<std::uint32_t>(av);
				if (!a_intfc->WriteRecordData(avRaw) || !a_intfc->WriteRecordData(total)) {
					SKSE::log::error("Failed to write reward entry");
					return;
				}
			}
		}

		void WriteUndoRecord(SKSE::SerializationInterface* a_intfc, const RuntimeState& state) noexcept
		{
			if (!a_intfc->OpenRecord(kRecordUndoHistory, kSerializationVersion)) {
				SKSE::log::error("Failed to open co-save record UNDO");
				return;
			}

			const std::uint32_t count = static_cast<std::uint32_t>(state.undoHistory.size());
			if (!a_intfc->WriteRecordData(count) || !a_intfc->WriteRecordData(state.undoNextActionId)) {
				SKSE::log::error("Failed to write undo header");
				return;
			}

			for (const auto& entry : state.undoHistory) {
				const std::uint32_t rewardCount = static_cast<std::uint32_t>(entry.rewardDeltas.size());
				if (!a_intfc->WriteRecordData(entry.actionId) ||
				    !a_intfc->WriteRecordData(entry.formId) ||
				    !a_intfc->WriteRecordData(entry.regKey) ||
				    !a_intfc->WriteRecordData(entry.group) ||
				    !a_intfc->WriteRecordData(rewardCount)) {
					SKSE::log::error("Failed to write undo entry header");
					return;
				}

				for (const auto& delta : entry.rewardDeltas) {
					const auto avRaw = static_cast<std::uint32_t>(delta.av);
					if (!a_intfc->WriteRecordData(avRaw) || !a_intfc->WriteRecordData(delta.delta)) {
						SKSE::log::error("Failed to write undo reward delta");
						return;
					}
				}
			}
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
	}

	void Save(SKSE::SerializationInterface* a_intfc) noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);

		ExecuteAllSaveWriters(
			[&]() { WriteRegisteredRecord(a_intfc, state); },
			[&]() { WriteBlockedRecord(a_intfc, state); },
			[&]() { WriteNotifiedRecord(a_intfc, state); },
			[&]() { WriteRewardsRecord(a_intfc, state); },
			[&]() { WriteUndoRecord(a_intfc, state); });
	}
}
