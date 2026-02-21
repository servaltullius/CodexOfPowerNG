#include "CodexOfPowerNG/Registration.h"

#include "CodexOfPowerNG/L10n.h"
#include "CodexOfPowerNG/RewardCaps.h"
#include "CodexOfPowerNG/RegistrationStateStore.h"
#include "CodexOfPowerNG/Rewards.h"

#include "RegistrationInternal.h"

#include <SKSE/Logger.h>

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>

namespace CodexOfPowerNG::Registration
{
	namespace
	{
		[[nodiscard]] std::string ResolveUndoItemName(const UndoRecord& record)
		{
			auto* primary = RE::TESForm::LookupByID(record.formId);
			auto* fallback = RE::TESForm::LookupByID(record.regKey);
			auto  name = Internal::BestItemName(primary, fallback);
			if (name.empty()) {
				name = L10n::T("ui.unnamed", "(unnamed)");
			}
			return name;
		}

		[[nodiscard]] RE::TESBoundObject* ResolveUndoItemObject(const UndoRecord& record) noexcept
		{
			if (auto* item = RE::TESForm::LookupByID<RE::TESBoundObject>(record.formId)) {
				return item;
			}
			return RE::TESForm::LookupByID<RE::TESBoundObject>(record.regKey);
		}

		void RollbackFailedUndo(const UndoRecord& record) noexcept
		{
			(void)RegistrationStateStore::InsertRegistered(record.regKey, record.group);
			RegistrationStateStore::RestoreUndoRecord(record);
		}
	}

	std::vector<UndoListItem> BuildRecentUndoList(std::size_t limit)
	{
		const auto take = (std::min)(limit, kUndoHistoryLimit);
		auto records = RegistrationStateStore::SnapshotUndoRecords(take);

		std::vector<UndoListItem> out;
		out.reserve(records.size());
		for (std::size_t i = 0; i < records.size(); ++i) {
			const auto& record = records[i];
			UndoListItem item{};
			item.actionId = record.actionId;
			item.formId = record.formId;
			item.regKey = record.regKey;
			item.group = record.group;
			item.canUndo = (i == 0);
			item.hasRewardDelta = !record.rewardDeltas.empty();
			item.name = ResolveUndoItemName(record);
			out.push_back(std::move(item));
		}
		return out;
	}

	UndoResult TryUndoRegistration(std::uint64_t actionId)
	{
		UndoResult result{};
		result.actionId = actionId;

		auto recordOpt = RegistrationStateStore::PopLatestUndoRecord(actionId);
		if (!recordOpt) {
			result.message = L10n::T(
				"msg.undoOnlyLatest",
				"Codex of Power: Undo is available only for the latest registration");
			return result;
		}

		auto record = std::move(*recordOpt);
		result.regKey = record.regKey;

		if (!RegistrationStateStore::RemoveRegistered(record.regKey)) {
			RegistrationStateStore::RestoreUndoRecord(record);
			result.message = L10n::T("msg.undoMissingRegistration", "Codex of Power: Undo failed (registration missing)");
			return result;
		}

		auto* player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			RollbackFailedUndo(record);
			result.message = L10n::T("msg.undoPlayerUnavailable", "Codex of Power: Undo failed (player unavailable)");
			return result;
		}

		auto* restoreItem = ResolveUndoItemObject(record);
		if (!restoreItem) {
			RollbackFailedUndo(record);
			result.message = L10n::T("msg.undoItemMissing", "Codex of Power: Undo failed (item form missing)");
			return result;
		}

		const auto oldCount = player->GetItemCount(restoreItem);
		player->AddObjectToContainer(restoreItem, nullptr, 1, nullptr);
		const auto newCount = player->GetItemCount(restoreItem);
		if (newCount <= oldCount) {
			RollbackFailedUndo(record);
			result.message = L10n::T("msg.undoRestoreFailed", "Codex of Power: Undo failed (cannot restore item)");
			return result;
		}

		const auto rollbackApplied = Rewards::RollbackRewardDeltas(record.rewardDeltas);
		const bool hadRollbackTarget =
			std::any_of(record.rewardDeltas.begin(), record.rewardDeltas.end(), [](const RewardDelta& entry) {
				return std::abs(entry.delta) > Rewards::kRewardCapEpsilon;
			});
		if (hadRollbackTarget && rollbackApplied == 0) {
			SKSE::log::warn(
				"Undo reward rollback: no actor deltas applied (actionId={}, regKey=0x{:08X}, deltas={})",
				record.actionId,
				static_cast<std::uint32_t>(record.regKey),
				record.rewardDeltas.size());
		}
		InvalidateQuickRegisterCache();

		const auto totalRegistered = RegistrationStateStore::SnapshotRegisteredItems().size();
		result.totalRegistered = totalRegistered;
		result.success = true;
		result.message =
			L10n::T("msg.undoOkPrefix", "Undo: ") + ResolveUndoItemName(record) +
			" (" + L10n::T("msg.totalPrefix", "total ") +
			std::to_string(totalRegistered) +
			L10n::T("msg.totalSuffix", " items") + ")";
		if (hadRollbackTarget && rollbackApplied == 0) {
			result.message += L10n::T(
				"msg.undoRewardRollbackWarning",
				" [warning: reward rollback not applied]");
		}

		RE::DebugNotification(result.message.c_str());
		return result;
	}
}
