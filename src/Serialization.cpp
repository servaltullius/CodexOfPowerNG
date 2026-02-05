#include "CodexOfPowerNG/Serialization.h"

#include "CodexOfPowerNG/Constants.h"
#include "CodexOfPowerNG/State.h"

#include <SKSE/Interfaces.h>
#include <SKSE/Logger.h>
#include <SKSE/SKSE.h>

#include <algorithm>
#include <array>
#include <cstddef>

namespace CodexOfPowerNG::Serialization
{
	namespace
	{
		void Skip(SKSE::SerializationInterface* a_intfc, std::uint32_t length) noexcept
		{
			std::array<std::byte, 256> buffer{};
			while (length > 0) {
				const auto chunkSize = (std::min)(length, static_cast<std::uint32_t>(buffer.size()));
				const auto bytesRead = a_intfc->ReadRecordData(buffer.data(), chunkSize);
				if (bytesRead == 0) {
					break;
				}
				length -= bytesRead;
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
		}

		void Save(SKSE::SerializationInterface* a_intfc) noexcept
		{
			auto& state = GetState();
			std::scoped_lock lock(state.mutex);

			// REGI: registered items (regKey -> group)
			if (!a_intfc->OpenRecord(kRecordRegisteredItems, kSerializationVersion)) {
				SKSE::log::error("Failed to open co-save record REGI");
				return;
			}

			{
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

			// BLCK: blocked items
			if (!a_intfc->OpenRecord(kRecordBlockedItems, kSerializationVersion)) {
				SKSE::log::error("Failed to open co-save record BLCK");
				return;
			}
			{
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

			// NTFY: notified items
			if (!a_intfc->OpenRecord(kRecordNotifiedItems, kSerializationVersion)) {
				SKSE::log::error("Failed to open co-save record NTFY");
				return;
			}
			{
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

			// RWDS: reward totals
			if (!a_intfc->OpenRecord(kRecordRewards, kSerializationVersion)) {
				SKSE::log::error("Failed to open co-save record RWDS");
				return;
			}
			{
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
		}

		void Load(SKSE::SerializationInterface* a_intfc) noexcept
		{
			auto& state = GetState();
			{
				std::scoped_lock lock(state.mutex);
				state.registeredItems.clear();
				state.blockedItems.clear();
				state.notifiedItems.clear();
				state.rewardTotals.clear();
			}

			std::uint32_t type{};
			std::uint32_t version{};
			std::uint32_t length{};

			while (a_intfc->GetNextRecordInfo(type, version, length)) {
				switch (type) {
				case kRecordRegisteredItems: {
					// v1: [count][FormID...]
					// v2: [count][FormID, group(uint32)]...
					std::uint32_t count{};
					if (length < sizeof(count) || a_intfc->ReadRecordData(count) != sizeof(count)) {
						SKSE::log::error("Failed to read registered count");
						return;
					}

					auto remaining = length - static_cast<std::uint32_t>(sizeof(count));

					std::scoped_lock lock(state.mutex);
					if (version == 1) {
						const auto maxCount = remaining / static_cast<std::uint32_t>(sizeof(RE::FormID));
						if (count > maxCount) {
							count = maxCount;
						}

						for (std::uint32_t i = 0; i < count; ++i) {
							RE::FormID oldId{};
							if (a_intfc->ReadRecordData(oldId) != sizeof(oldId)) {
								SKSE::log::error("Failed to read registered formId");
								return;
							}
							remaining -= static_cast<std::uint32_t>(sizeof(oldId));

							RE::FormID newId{};
							if (!a_intfc->ResolveFormID(oldId, newId)) {
								continue;
							}

							state.registeredItems.emplace(newId, 255u);
						}
					} else if (version == 2) {
						const auto entrySize = static_cast<std::uint32_t>(sizeof(RE::FormID) + sizeof(std::uint32_t));
						const auto maxCount = remaining / entrySize;
						if (count > maxCount) {
							count = maxCount;
						}

						for (std::uint32_t i = 0; i < count; ++i) {
							RE::FormID oldId{};
							std::uint32_t group{};
							if (a_intfc->ReadRecordData(oldId) != sizeof(oldId) || a_intfc->ReadRecordData(group) != sizeof(group)) {
								SKSE::log::error("Failed to read registered entry");
								return;
							}
							remaining -= entrySize;

							RE::FormID newId{};
							if (!a_intfc->ResolveFormID(oldId, newId)) {
								continue;
							}

							state.registeredItems.emplace(newId, group);
						}
					} else {
						SKSE::log::warn("Unsupported REGI version {}", version);
					}

					if (remaining > 0) {
						Skip(a_intfc, remaining);
					}
					break;
				}
				case kRecordBlockedItems:
				case kRecordNotifiedItems: {
					std::uint32_t count{};
					if (length < sizeof(count) || a_intfc->ReadRecordData(count) != sizeof(count)) {
						SKSE::log::error("Failed to read list count");
						return;
					}

					auto remaining = length - static_cast<std::uint32_t>(sizeof(count));
					const auto maxCount = remaining / static_cast<std::uint32_t>(sizeof(RE::FormID));
					if (count > maxCount) {
						count = maxCount;
					}

					std::scoped_lock lock(state.mutex);
					for (std::uint32_t i = 0; i < count; ++i) {
						RE::FormID oldId{};
						if (a_intfc->ReadRecordData(oldId) != sizeof(oldId)) {
							SKSE::log::error("Failed to read list formId");
							return;
						}
						remaining -= static_cast<std::uint32_t>(sizeof(oldId));

						RE::FormID newId{};
						if (!a_intfc->ResolveFormID(oldId, newId)) {
							continue;
						}

						if (type == kRecordBlockedItems) {
							state.blockedItems.insert(newId);
						} else {
							state.notifiedItems.insert(newId);
						}
					}

					if (remaining > 0) {
						Skip(a_intfc, remaining);
					}
					break;
				}
				case kRecordRewards: {
					std::uint32_t count{};
					if (length < sizeof(count) || a_intfc->ReadRecordData(count) != sizeof(count)) {
						SKSE::log::error("Failed to read reward count");
						return;
					}

					auto remaining = length - static_cast<std::uint32_t>(sizeof(count));
					const auto entrySize = static_cast<std::uint32_t>(sizeof(std::uint32_t) + sizeof(float));
					const auto maxCount = remaining / entrySize;
					if (count > maxCount) {
						count = maxCount;
					}

					std::scoped_lock lock(state.mutex);
					for (std::uint32_t i = 0; i < count; ++i) {
						std::uint32_t avRaw{};
						float total{};
						if (a_intfc->ReadRecordData(avRaw) != sizeof(avRaw) || a_intfc->ReadRecordData(total) != sizeof(total)) {
							SKSE::log::error("Failed to read reward entry");
							return;
						}
						remaining -= entrySize;
						state.rewardTotals.emplace(static_cast<RE::ActorValue>(avRaw), total);
					}

					if (remaining > 0) {
						Skip(a_intfc, remaining);
					}
					break;
				}
				default:
					SKSE::log::warn("Skipping unknown record type {:08X}", type);
					Skip(a_intfc, length);
					break;
				}
			}
		}
	}

	void Install() noexcept
	{
		auto* serialization = SKSE::GetSerializationInterface();
		if (!serialization) {
			SKSE::log::error("Serialization interface unavailable");
			return;
		}

		serialization->SetUniqueID(kSerializationUniqueId);
		serialization->SetRevertCallback(Revert);
		serialization->SetSaveCallback(Save);
		serialization->SetLoadCallback(Load);
	}
}
