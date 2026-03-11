#include "SerializationInternal.h"

#include "CodexOfPowerNG/BuildProgression.h"
#include "CodexOfPowerNG/Constants.h"
#include "CodexOfPowerNG/Registration.h"
#include "CodexOfPowerNG/RewardCaps.h"
#include "CodexOfPowerNG/Rewards.h"
#include "CodexOfPowerNG/SerializationStateStore.h"

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
		inline constexpr std::uint32_t kMaxSerializedRewardEntries = 256;
		inline constexpr std::uint32_t kMaxSerializedUndoRewardDeltas = 64;
		inline constexpr std::uint32_t kMaxSerializedBuildSlotStringLength = 128;

		[[nodiscard]] bool IsSupportedRewardActorValue(std::uint32_t avRaw) noexcept
		{
			switch (static_cast<RE::ActorValue>(avRaw)) {
			case RE::ActorValue::kOneHanded:
			case RE::ActorValue::kTwoHanded:
			case RE::ActorValue::kArchery:
			case RE::ActorValue::kCriticalChance:
			case RE::ActorValue::kUnarmedDamage:
			case RE::ActorValue::kStaminaRate:
			case RE::ActorValue::kStamina:
			case RE::ActorValue::kSpeedMult:
			case RE::ActorValue::kCarryWeight:
			case RE::ActorValue::kDamageResist:
			case RE::ActorValue::kHealth:
			case RE::ActorValue::kResistMagic:
			case RE::ActorValue::kResistFire:
			case RE::ActorValue::kResistFrost:
			case RE::ActorValue::kResistShock:
			case RE::ActorValue::kHealRate:
			case RE::ActorValue::kReflectDamage:
			case RE::ActorValue::kHeavyArmor:
			case RE::ActorValue::kSmithingModifier:
			case RE::ActorValue::kLightArmor:
			case RE::ActorValue::kBlock:
			case RE::ActorValue::kMagicka:
			case RE::ActorValue::kMagickaRate:
			case RE::ActorValue::kPoisonResist:
			case RE::ActorValue::kResistDisease:
			case RE::ActorValue::kAlchemyModifier:
			case RE::ActorValue::kAlchemy:
			case RE::ActorValue::kDestructionModifier:
			case RE::ActorValue::kRestorationModifier:
			case RE::ActorValue::kAlterationModifier:
			case RE::ActorValue::kConjurationModifier:
			case RE::ActorValue::kIllusionModifier:
			case RE::ActorValue::kEnchantingModifier:
			case RE::ActorValue::kAbsorbChance:
			case RE::ActorValue::kShoutRecoveryMult:
			case RE::ActorValue::kSpeechcraftModifier:
			case RE::ActorValue::kLockpickingModifier:
			case RE::ActorValue::kPickpocketModifier:
			case RE::ActorValue::kSneakingModifier:
			case RE::ActorValue::kLockpicking:
			case RE::ActorValue::kPickpocket:
			case RE::ActorValue::kAttackDamageMult:
				return true;
			default:
				return false;
			}
		}

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

		[[nodiscard]] bool ReadString(
			SKSE::SerializationInterface* a_intfc,
			std::uint32_t&                remaining,
			std::string&                  out) noexcept
		{
			std::uint32_t length{};
			if (remaining < sizeof(length) || a_intfc->ReadRecordData(length) != sizeof(length)) {
				return false;
			}
			remaining -= static_cast<std::uint32_t>(sizeof(length));

			if (length > remaining) {
				return false;
			}

			std::string value(length, '\0');
			if (length > 0 && a_intfc->ReadRecordData(value.data(), length) != length) {
				return false;
			}
			remaining -= length;

			if (length > kMaxSerializedBuildSlotStringLength) {
				value.resize(kMaxSerializedBuildSlotStringLength);
			}

			out = std::move(value);
			return true;
		}
	}

	void Load(SKSE::SerializationInterface* a_intfc) noexcept
	{
		std::uint32_t type{};
		std::uint32_t version{};
		std::uint32_t length{};
		SerializationStateStore::Snapshot loadedState{};

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

					loadedState.registeredItems.emplace(newId, kGroupUnknown);
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

						loadedState.registeredItems.emplace(newId, group);
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
						loadedState.blockedItems.insert(newId);
					} else {
						loadedState.notifiedItems.insert(newId);
					}
				}

				if (remaining > 0) {
					Skip(a_intfc, remaining);
				}
				break;
			}
			case kRecordBuildScores: {
				if (version != 1u) {
					SKSE::log::warn("Unsupported BSCR version {}", version);
					Skip(a_intfc, length);
					break;
				}
				if (length < sizeof(loadedState.attackScore) + sizeof(loadedState.defenseScore) + sizeof(loadedState.utilityScore) ||
				    a_intfc->ReadRecordData(loadedState.attackScore) != sizeof(loadedState.attackScore) ||
				    a_intfc->ReadRecordData(loadedState.defenseScore) != sizeof(loadedState.defenseScore) ||
				    a_intfc->ReadRecordData(loadedState.utilityScore) != sizeof(loadedState.utilityScore)) {
					SKSE::log::error("Failed to read build scores");
					return;
				}

				const auto bytesRead = static_cast<std::uint32_t>(
					sizeof(loadedState.attackScore) +
					sizeof(loadedState.defenseScore) +
					sizeof(loadedState.utilityScore));
				if (length > bytesRead) {
					Skip(a_intfc, length - bytesRead);
				}
				break;
			}
			case kRecordBuildSlots: {
				if (version != 1u) {
					SKSE::log::warn("Unsupported BSLT version {}", version);
					Skip(a_intfc, length);
					break;
				}

				std::uint32_t slotCount{};
				if (length < sizeof(slotCount) || a_intfc->ReadRecordData(slotCount) != sizeof(slotCount)) {
					SKSE::log::error("Failed to read build slot count");
					return;
				}

				auto remaining = length - static_cast<std::uint32_t>(sizeof(slotCount));
				loadedState.activeBuildSlots = {};
				const auto maxSlots = static_cast<std::uint32_t>(loadedState.activeBuildSlots.size());
				for (std::uint32_t i = 0; i < slotCount && remaining > 0; ++i) {
					std::string slotValue;
					if (!ReadString(a_intfc, remaining, slotValue)) {
						SKSE::log::error("Failed to read build slot entry");
						return;
					}
					if (i < maxSlots) {
						loadedState.activeBuildSlots[i] = std::move(slotValue);
					}
				}

				if (remaining > 0) {
					Skip(a_intfc, remaining);
				}
				break;
			}
			case kRecordBuildMigration: {
				if (version != 1u) {
					SKSE::log::warn("Unsupported BMIG version {}", version);
					Skip(a_intfc, length);
					break;
				}

				const auto minimumLength = static_cast<std::uint32_t>(
					sizeof(loadedState.buildMigrationVersion) +
					sizeof(std::uint32_t) +
					sizeof(std::uint8_t) +
					sizeof(std::uint8_t) +
					sizeof(loadedState.buildMigrationNotice.unresolvedHistoricalRegistrations));
				if (length < minimumLength) {
					SKSE::log::error("Failed to read build migration record");
					return;
				}

				std::uint32_t migrationStateRaw{};
				std::uint8_t  needsNotice{};
				std::uint8_t  legacyRewardsMigrated{};
				if (a_intfc->ReadRecordData(loadedState.buildMigrationVersion) != sizeof(loadedState.buildMigrationVersion) ||
				    a_intfc->ReadRecordData(migrationStateRaw) != sizeof(migrationStateRaw) ||
				    a_intfc->ReadRecordData(needsNotice) != sizeof(needsNotice) ||
				    a_intfc->ReadRecordData(legacyRewardsMigrated) != sizeof(legacyRewardsMigrated) ||
				    a_intfc->ReadRecordData(loadedState.buildMigrationNotice.unresolvedHistoricalRegistrations) !=
					    sizeof(loadedState.buildMigrationNotice.unresolvedHistoricalRegistrations)) {
					SKSE::log::error("Failed to read build migration payload");
					return;
				}

				loadedState.buildMigrationState = static_cast<Builds::BuildMigrationState>(migrationStateRaw);
				loadedState.buildMigrationNotice.needsNotice = needsNotice != 0u;
				loadedState.buildMigrationNotice.legacyRewardsMigrated = legacyRewardsMigrated != 0u;

				if (length > minimumLength) {
					Skip(a_intfc, length - minimumLength);
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
				const auto readableCount = (std::min)(count, maxCount);
				const auto readCount = (std::min)(readableCount, kMaxSerializedRewardEntries);
				if (readableCount > readCount) {
					SKSE::log::warn(
						"Serialization load: reward record truncated from {} to {} entries",
						readableCount,
						readCount);
				}

				for (std::uint32_t i = 0; i < readCount; ++i) {
					std::uint32_t avRaw{};
					float total{};
					if (a_intfc->ReadRecordData(avRaw) != sizeof(avRaw) || a_intfc->ReadRecordData(total) != sizeof(total)) {
						SKSE::log::error("Failed to read reward entry");
						return;
					}
					remaining -= entrySize;
					if (!IsSupportedRewardActorValue(avRaw)) {
						SKSE::log::warn("Serialization load: skipping unsupported reward actor value {}", avRaw);
						continue;
					}
					const auto av = static_cast<RE::ActorValue>(avRaw);
					const float clamped = Rewards::ClampRewardTotal(av, total);
					if (std::abs(clamped - total) > Rewards::kRewardCapEpsilon) {
						SKSE::log::warn(
							"Serialization load: clamped reward total for AV {} from {:.4f} to {:.4f}",
							avRaw,
							total,
							clamped);
					}
					loadedState.rewardTotals.insert_or_assign(av, clamped);
				}

				const auto skippedEntries = readableCount - readCount;
				if (skippedEntries > 0) {
					const auto skipBytes = skippedEntries * entrySize;
					Skip(a_intfc, skipBytes);
					remaining -= skipBytes;
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
				const auto undoHeaderBaseSize = static_cast<std::uint32_t>(
					sizeof(std::uint64_t) +
					sizeof(RE::FormID) +
					sizeof(RE::FormID) +
					sizeof(std::uint32_t));
				const auto undoBuildContributionSize = static_cast<std::uint32_t>(
					sizeof(std::uint32_t) +
					sizeof(std::uint32_t) +
					sizeof(std::int32_t));
				const auto undoHeaderSize = undoHeaderBaseSize +
					(version >= 3u ? undoBuildContributionSize : 0u) +
					static_cast<std::uint32_t>(sizeof(std::uint32_t));
				const auto rewardDeltaSize = static_cast<std::uint32_t>(sizeof(std::uint32_t) + sizeof(float));

				loadedState.undoHistory.clear();
				for (std::uint32_t i = 0; i < count && remaining >= undoHeaderSize; ++i) {
					Registration::UndoRecord entry{};
					std::uint32_t            rewardCount{};
					std::uint32_t            hasBuildContribution{};
					std::uint32_t            buildDisciplineRaw{};
					std::int32_t             buildScoreDelta{};
					RE::FormID               oldFormId{};
					RE::FormID               oldRegKey{};
					if (a_intfc->ReadRecordData(entry.actionId) != sizeof(entry.actionId) ||
					    a_intfc->ReadRecordData(oldFormId) != sizeof(oldFormId) ||
					    a_intfc->ReadRecordData(oldRegKey) != sizeof(oldRegKey) ||
					    a_intfc->ReadRecordData(entry.group) != sizeof(entry.group)) {
						SKSE::log::error("Failed to read undo entry header");
						return;
					}
					remaining -= undoHeaderBaseSize;

					if (version >= 3u) {
						if (a_intfc->ReadRecordData(hasBuildContribution) != sizeof(hasBuildContribution) ||
						    a_intfc->ReadRecordData(buildDisciplineRaw) != sizeof(buildDisciplineRaw) ||
						    a_intfc->ReadRecordData(buildScoreDelta) != sizeof(buildScoreDelta)) {
							SKSE::log::error("Failed to read undo build contribution payload");
							return;
						}
						remaining -= undoBuildContributionSize;
						if (hasBuildContribution != 0u) {
							entry.buildContribution = Registration::BuildScoreContribution{
								.discipline = static_cast<Builds::BuildDiscipline>(buildDisciplineRaw),
								.scoreDelta = buildScoreDelta,
							};
						}
					}

					if (a_intfc->ReadRecordData(rewardCount) != sizeof(rewardCount)) {
						SKSE::log::error("Failed to read undo reward count");
						return;
					}
					remaining -= static_cast<std::uint32_t>(sizeof(rewardCount));

					RE::FormID newRegKey{};
					const bool regKeyResolved = a_intfc->ResolveFormID(oldRegKey, newRegKey);
					RE::FormID newFormId{};
					const bool formIdResolved = a_intfc->ResolveFormID(oldFormId, newFormId);

					const auto availableRewardCount = rewardDeltaSize > 0 ? (remaining / rewardDeltaSize) : 0;
					const auto readableRewardCount = (std::min)(rewardCount, availableRewardCount);
					if (readableRewardCount > kMaxSerializedUndoRewardDeltas) {
						SKSE::log::warn(
							"Serialization load: truncating undo reward deltas for action {} from {} to {}",
							entry.actionId,
							readableRewardCount,
							kMaxSerializedUndoRewardDeltas);
					}

					const auto storedRewardCount = (std::min)(readableRewardCount, kMaxSerializedUndoRewardDeltas);
					entry.rewardDeltas.reserve(storedRewardCount);
					for (std::uint32_t j = 0; j < readableRewardCount; ++j) {
						std::uint32_t avRaw{};
						float         delta{};
						if (a_intfc->ReadRecordData(avRaw) != sizeof(avRaw) ||
						    a_intfc->ReadRecordData(delta) != sizeof(delta)) {
							SKSE::log::error("Failed to read undo reward delta");
							return;
						}
						remaining -= rewardDeltaSize;

						if (j >= kMaxSerializedUndoRewardDeltas) {
							continue;
						}
						if (!IsSupportedRewardActorValue(avRaw)) {
							SKSE::log::warn(
								"Serialization load: dropping unsupported undo reward actor value {} in action {}",
								avRaw,
								entry.actionId);
							continue;
						}
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

					loadedState.undoHistory.push_back(std::move(entry));
					while (loadedState.undoHistory.size() > Registration::kUndoHistoryLimit) {
						loadedState.undoHistory.pop_front();
					}
				}

				if (!loadedState.undoHistory.empty()) {
					const auto maxActionId = loadedState.undoHistory.back().actionId + 1;
					loadedState.undoNextActionId = (std::max)(nextActionId, maxActionId);
				} else {
					loadedState.undoNextActionId = (std::max)(nextActionId, std::uint64_t{ 1 });
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

		BuildProgression::NormalizeLoadedSnapshot(loadedState);
		SerializationStateStore::ReplaceState(std::move(loadedState));
		Registration::InvalidateQuickRegisterCache();
	}
}
