#include "SerializationInternal.h"

#include "CodexOfPowerNG/Constants.h"
#include "CodexOfPowerNG/RewardCaps.h"
#include "CodexOfPowerNG/Rewards.h"
#include "CodexOfPowerNG/State.h"

#include <SKSE/Interfaces.h>
#include <SKSE/Logger.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

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
					const auto av = static_cast<RE::ActorValue>(avRaw);
					const float clamped = Rewards::ClampRewardTotal(av, total);
					if (std::abs(clamped - total) > Rewards::kRewardCapEpsilon) {
						SKSE::log::warn(
							"Serialization load: clamped reward total for AV {} from {:.4f} to {:.4f}",
							avRaw,
							total,
							clamped);
					}
					state.rewardTotals.insert_or_assign(av, clamped);
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

		Rewards::SyncRewardTotalsToPlayer();
	}
}
