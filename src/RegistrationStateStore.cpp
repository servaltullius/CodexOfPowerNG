#include "CodexOfPowerNG/RegistrationStateStore.h"

#include "CodexOfPowerNG/State.h"

#include <algorithm>
#include <atomic>
#include <utility>

namespace CodexOfPowerNG::RegistrationStateStore
{
	namespace
	{
		class RuntimeRegistrationStateStore final : public IRegistrationStateStore
		{
		public:
			bool IsBlocked(RE::FormID formId) noexcept override
			{
				auto& state = GetState();
				std::scoped_lock lock(state.mutex);
				return state.blockedItems.contains(formId);
			}

			bool IsRegistered(RE::FormID formId) noexcept override
			{
				auto& state = GetState();
				std::scoped_lock lock(state.mutex);
				return state.registeredItems.contains(formId);
			}

			bool IsRegisteredEither(RE::FormID regKeyId, RE::FormID legacyId) noexcept override
			{
				auto& state = GetState();
				std::scoped_lock lock(state.mutex);
				if (state.registeredItems.contains(regKeyId)) {
					return true;
				}

				// Backward compat: older data may have stored the non-template variant as the key.
				if (legacyId != regKeyId) {
					return state.registeredItems.contains(legacyId);
				}
				return false;
			}

			void BlockPair(RE::FormID regKeyId, RE::FormID itemId) noexcept override
			{
				auto& state = GetState();
				std::scoped_lock lock(state.mutex);

				if (regKeyId != 0) {
					state.blockedItems.insert(regKeyId);
				}
				if (itemId != 0 && itemId != regKeyId) {
					state.blockedItems.insert(itemId);
				}
			}

			std::size_t InsertRegistered(RE::FormID regKeyId, std::uint32_t group) noexcept override
			{
				auto& state = GetState();
				std::scoped_lock lock(state.mutex);
				state.registeredItems.emplace(regKeyId, group);
				return state.registeredItems.size();
			}

			bool RemoveRegistered(RE::FormID regKeyId) noexcept override
			{
				auto& state = GetState();
				std::scoped_lock lock(state.mutex);
				return state.registeredItems.erase(regKeyId) > 0;
			}

			std::uint64_t PushUndoRecord(Registration::UndoRecord record) noexcept override
			{
				auto& state = GetState();
				std::scoped_lock lock(state.mutex);

				record.actionId = state.undoNextActionId++;
				state.undoHistory.push_back(std::move(record));
				while (state.undoHistory.size() > Registration::kUndoHistoryLimit) {
					state.undoHistory.pop_front();
				}

				return state.undoHistory.empty() ? 0 : state.undoHistory.back().actionId;
			}

			std::optional<Registration::UndoRecord> PopLatestUndoRecord(std::uint64_t actionId) noexcept override
			{
				auto& state = GetState();
				std::scoped_lock lock(state.mutex);
				if (state.undoHistory.empty()) {
					return std::nullopt;
				}

				const auto& latest = state.undoHistory.back();
				if (latest.actionId != actionId) {
					return std::nullopt;
				}

				auto out = std::move(state.undoHistory.back());
				state.undoHistory.pop_back();
				return out;
			}

			void RestoreUndoRecord(Registration::UndoRecord record) noexcept override
			{
				auto& state = GetState();
				std::scoped_lock lock(state.mutex);
				state.undoNextActionId = (std::max)(state.undoNextActionId, record.actionId + 1);
				state.undoHistory.push_back(std::move(record));
				while (state.undoHistory.size() > Registration::kUndoHistoryLimit) {
					state.undoHistory.pop_front();
				}
			}

			std::vector<Registration::UndoRecord> SnapshotUndoRecords(std::size_t limit) noexcept override
			{
				std::vector<Registration::UndoRecord> out;
				auto& state = GetState();
				std::scoped_lock lock(state.mutex);

				if (state.undoHistory.empty()) {
					return out;
				}

				const auto take = (std::min)(limit, state.undoHistory.size());
				out.reserve(take);

				for (auto it = state.undoHistory.rbegin(); it != state.undoHistory.rend() && out.size() < take; ++it) {
					out.push_back(*it);
				}
				return out;
			}

			QuickListSnapshot SnapshotQuickList() noexcept override
			{
				QuickListSnapshot snapshot{};
				auto&             state = GetState();
				std::scoped_lock  lock(state.mutex);

				snapshot.blockedItems = state.blockedItems;
				snapshot.registeredKeys.reserve(state.registeredItems.size() * 2);
				for (const auto& [id, _] : state.registeredItems) {
					snapshot.registeredKeys.insert(id);
				}
				return snapshot;
			}

			std::vector<std::pair<RE::FormID, std::uint32_t>> SnapshotRegisteredItems() noexcept override
			{
				std::vector<std::pair<RE::FormID, std::uint32_t>> out;
				auto&                                              state = GetState();
				std::scoped_lock                                   lock(state.mutex);

				out.reserve(state.registeredItems.size());
				for (const auto& [id, group] : state.registeredItems) {
					out.emplace_back(id, group);
				}
				return out;
			}
		};

		RuntimeRegistrationStateStore           g_runtimeStore;
		std::atomic<IRegistrationStateStore*> g_overrideStore{ nullptr };
	}

	IRegistrationStateStore& GetStore() noexcept
	{
		if (auto* overrideStore = g_overrideStore.load(std::memory_order_acquire)) {
			return *overrideStore;
		}
		return g_runtimeStore;
	}

	void SetStoreForTesting(IRegistrationStateStore* store) noexcept
	{
		g_overrideStore.store(store, std::memory_order_release);
	}

	bool IsBlocked(RE::FormID formId) noexcept
	{
		return GetStore().IsBlocked(formId);
	}

	bool IsRegistered(RE::FormID formId) noexcept
	{
		return GetStore().IsRegistered(formId);
	}

	bool IsRegisteredEither(RE::FormID regKeyId, RE::FormID legacyId) noexcept
	{
		return GetStore().IsRegisteredEither(regKeyId, legacyId);
	}

	void BlockPair(RE::FormID regKeyId, RE::FormID itemId) noexcept
	{
		GetStore().BlockPair(regKeyId, itemId);
	}

	std::size_t InsertRegistered(RE::FormID regKeyId, std::uint32_t group) noexcept
	{
		return GetStore().InsertRegistered(regKeyId, group);
	}

	bool RemoveRegistered(RE::FormID regKeyId) noexcept
	{
		return GetStore().RemoveRegistered(regKeyId);
	}

	std::uint64_t PushUndoRecord(Registration::UndoRecord record) noexcept
	{
		return GetStore().PushUndoRecord(std::move(record));
	}

	std::optional<Registration::UndoRecord> PopLatestUndoRecord(std::uint64_t actionId) noexcept
	{
		return GetStore().PopLatestUndoRecord(actionId);
	}

	void RestoreUndoRecord(Registration::UndoRecord record) noexcept
	{
		GetStore().RestoreUndoRecord(std::move(record));
	}

	std::vector<Registration::UndoRecord> SnapshotUndoRecords(std::size_t limit) noexcept
	{
		return GetStore().SnapshotUndoRecords(limit);
	}

	QuickListSnapshot SnapshotQuickList() noexcept
	{
		return GetStore().SnapshotQuickList();
	}

	std::vector<std::pair<RE::FormID, std::uint32_t>> SnapshotRegisteredItems() noexcept
	{
		return GetStore().SnapshotRegisteredItems();
	}
}
