#include "CodexOfPowerNG/RegistrationStateStore.h"

#include "CodexOfPowerNG/State.h"

#include <atomic>

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

	QuickListSnapshot SnapshotQuickList() noexcept
	{
		return GetStore().SnapshotQuickList();
	}

	std::vector<std::pair<RE::FormID, std::uint32_t>> SnapshotRegisteredItems() noexcept
	{
		return GetStore().SnapshotRegisteredItems();
	}
}
