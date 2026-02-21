#include "SerializationInternal.h"

#include "CodexOfPowerNG/Constants.h"
#include "CodexOfPowerNG/Registration.h"
#include "CodexOfPowerNG/RewardCaps.h"
#include "CodexOfPowerNG/Rewards.h"
#include "CodexOfPowerNG/State.h"

#include <SKSE/Interfaces.h>
#include <SKSE/Logger.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace CodexOfPowerNG::Serialization::Internal
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
	}

	void Load(SKSE::SerializationInterface* a_intfc) noexcept
	{
		std::uint32_t type{};
		std::uint32_t version{};
		std::uint32_t length{};
		std::unordered_map<RE::FormID, std::uint32_t>                 loadedRegisteredItems;
		std::unordered_set<RE::FormID>                                loadedBlockedItems;
		std::unordered_set<RE::FormID>                                loadedNotifiedItems;
		std::unordered_map<RE::ActorValue, float, ActorValueHash>     loadedRewardTotals;
		std::deque<Registration::UndoRecord>                          loadedUndoHistory;
		std::uint64_t                                                 loadedUndoNextActionId{ 1 };

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

						loadedRegisteredItems.emplace(newId, 255u);
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

						loadedRegisteredItems.emplace(newId, group);
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
						loadedBlockedItems.insert(newId);
					} else {
						loadedNotifiedItems.insert(newId);
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

				for (std::uint32_t i = 0; i < count; ++i) {
					std::uint32_t avRaw{};
					float total{};
					if (a_intfc->ReadRecordData(avRaw) != sizeof(avRaw) || a_intfc->ReadRecordData(total) != sizeof(total)) {
						SKSE::log::error("Failed to read reward entry");
						return;
					}
					remaining -= entrySize;
					const auto av = static_cast<RE::ActorValue>(avRaw);
					const float clamped = Rewards::ClampRewardTotal(av, total);
					if (std::abs(clamped - total) > Rewards::kRewardCapEpsilon) {
						SKSE::log::warn(
							"Serialization load: clamped reward total for AV {} from {:.4f} to {:.4f}",
							avRaw,
							total,
							clamped);
					}
					loadedRewardTotals.insert_or_assign(av, clamped);
				}

				if (remaining > 0) {
					Skip(a_intfc, remaining);
				}
				break;
			}
			case kRecordUndoHistory: {
				std::uint32_t count{};
				std::uint64_t nextActionId{ 1 };
				const auto headerSize = static_cast<std::uint32_t>(sizeof(count) + sizeof(nextActionId));
				if (length < headerSize ||
				    a_intfc->ReadRecordData(count) != sizeof(count) ||
				    a_intfc->ReadRecordData(nextActionId) != sizeof(nextActionId)) {
					SKSE::log::error("Failed to read undo header");
					return;
				}

				auto remaining = length - headerSize;
				const auto undoHeaderSize = static_cast<std::uint32_t>(
					sizeof(std::uint64_t) + sizeof(RE::FormID) + sizeof(RE::FormID) + sizeof(std::uint32_t) + sizeof(std::uint32_t));
				const auto rewardDeltaSize = static_cast<std::uint32_t>(sizeof(std::uint32_t) + sizeof(float));

				loadedUndoHistory.clear();
				for (std::uint32_t i = 0; i < count && remaining >= undoHeaderSize; ++i) {
					Registration::UndoRecord entry{};
					std::uint32_t            rewardCount{};
					RE::FormID               oldFormId{};
					RE::FormID               oldRegKey{};
					if (a_intfc->ReadRecordData(entry.actionId) != sizeof(entry.actionId) ||
					    a_intfc->ReadRecordData(oldFormId) != sizeof(oldFormId) ||
					    a_intfc->ReadRecordData(oldRegKey) != sizeof(oldRegKey) ||
					    a_intfc->ReadRecordData(entry.group) != sizeof(entry.group) ||
					    a_intfc->ReadRecordData(rewardCount) != sizeof(rewardCount)) {
						SKSE::log::error("Failed to read undo entry header");
						return;
					}
					remaining -= undoHeaderSize;

					RE::FormID newRegKey{};
					const bool regKeyResolved = a_intfc->ResolveFormID(oldRegKey, newRegKey);
					RE::FormID newFormId{};
					const bool formIdResolved = a_intfc->ResolveFormID(oldFormId, newFormId);

					const auto maxRewardCount = rewardDeltaSize > 0 ? (remaining / rewardDeltaSize) : 0;
					if (rewardCount > maxRewardCount) {
						rewardCount = maxRewardCount;
					}

					entry.rewardDeltas.reserve(rewardCount);
					for (std::uint32_t j = 0; j < rewardCount; ++j) {
						std::uint32_t avRaw{};
						float         delta{};
						if (a_intfc->ReadRecordData(avRaw) != sizeof(avRaw) ||
						    a_intfc->ReadRecordData(delta) != sizeof(delta)) {
							SKSE::log::error("Failed to read undo reward delta");
							return;
						}
						remaining -= rewardDeltaSize;
						entry.rewardDeltas.push_back(Registration::RewardDelta{
							static_cast<RE::ActorValue>(avRaw),
							delta });
					}

					if (!regKeyResolved || !formIdResolved) {
						SKSE::log::warn(
							"Skipping undo entry {}: unresolved form(s) formId={:08X} regKey={:08X}",
							entry.actionId,
							oldFormId,
							oldRegKey);
						continue;
					}

					entry.regKey = newRegKey;
					entry.formId = newFormId;

					loadedUndoHistory.push_back(std::move(entry));
					while (loadedUndoHistory.size() > Registration::kUndoHistoryLimit) {
						loadedUndoHistory.pop_front();
					}
				}

				if (!loadedUndoHistory.empty()) {
					const auto maxActionId = loadedUndoHistory.back().actionId + 1;
					loadedUndoNextActionId = (std::max)(nextActionId, maxActionId);
				} else {
					loadedUndoNextActionId = (std::max)(nextActionId, std::uint64_t{ 1 });
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

		auto& state = GetState();
		{
			std::scoped_lock lock(state.mutex);
			state.registeredItems = std::move(loadedRegisteredItems);
			state.blockedItems = std::move(loadedBlockedItems);
			state.notifiedItems = std::move(loadedNotifiedItems);
			state.rewardTotals = std::move(loadedRewardTotals);
			state.undoHistory = std::move(loadedUndoHistory);
			state.undoNextActionId = loadedUndoNextActionId;
		}
		Registration::InvalidateQuickRegisterCache();
	}
}
