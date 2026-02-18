#include "PrismaUIRequestOps.h"

#include "CodexOfPowerNG/Config.h"
#include "CodexOfPowerNG/PrismaUIManager.h"
#include "CodexOfPowerNG/Registration.h"
#include "CodexOfPowerNG/Rewards.h"
#include "CodexOfPowerNG/State.h"
#include "CodexOfPowerNG/TaskScheduler.h"
#include "PrismaUIPayloads.h"

#include <SKSE/Logger.h>

#include <algorithm>
#include <charconv>
#include <chrono>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace CodexOfPowerNG::PrismaUIManager::Internal
{
	InventoryRequest ParseInventoryRequest(const char* argument) noexcept
	{
		InventoryRequest out{};
		json payload;
		try {
			payload = json::parse(argument ? argument : "{}");
		} catch (...) {
			return out;
		}

		try {
			if (auto it = payload.find("page"); it != payload.end() && it->is_number_integer()) {
				const auto v = it->get<std::int64_t>();
				out.page = v > 0 ? static_cast<std::uint32_t>(v) : 0;
			}
			if (auto it = payload.find("pageSize"); it != payload.end() && it->is_number_integer()) {
				const auto v = it->get<std::int64_t>();
				const auto clamped = std::clamp<std::int64_t>(v, 1, 500);
				out.pageSize = static_cast<std::uint32_t>(clamped);
			}
		} catch (...) {
		}

		return out;
	}

	std::optional<RE::FormID> ParseFormIDFromJson(const json& j) noexcept
	{
		try {
			if (j.is_number_unsigned()) {
				return static_cast<RE::FormID>(j.get<std::uint32_t>());
			}
			if (j.is_number_integer()) {
				const auto v = j.get<std::int64_t>();
				if (v < 0 || v > 0xFFFFFFFFll) {
					return std::nullopt;
				}
				return static_cast<RE::FormID>(static_cast<std::uint32_t>(v));
			}
			if (j.is_string()) {
				const auto s = j.get<std::string>();
				std::size_t idx = 0;
				int         base = 10;
				if (s.rfind("0x", 0) == 0 || s.rfind("0X", 0) == 0) {
					base = 16;
					idx = 2;
				}
				std::uint32_t value{};
				const auto*   begin = s.data() + idx;
				const auto*   end = s.data() + s.size();
				auto [ptr, ec] = std::from_chars(begin, end, value, base);
				if (ec != std::errc{} || ptr != end) {
					return std::nullopt;
				}
				return static_cast<RE::FormID>(value);
			}
		} catch (...) {
		}

		return std::nullopt;
	}

	void QueueSendInventory(InventoryRequest req) noexcept
	{
		if (QueueMainTask([req]() {
				const auto tBuild0 = std::chrono::steady_clock::now();
				const auto offset = static_cast<std::size_t>(req.page) * static_cast<std::size_t>(req.pageSize);
				auto page = Registration::BuildQuickRegisterList(offset, req.pageSize);
				const auto tBuild1 = std::chrono::steady_clock::now();
				const auto buildMs =
					static_cast<std::uint32_t>(
						std::chrono::duration_cast<std::chrono::milliseconds>(tBuild1 - tBuild0).count());
				SKSE::log::info(
					"Inventory: built page {} ({} items, hasMore={}, total {}) in {}ms",
					req.page, page.items.size(), page.hasMore, page.total, buildMs);
				const auto itemCount = page.items.size();
				(void)QueueUITask([page = std::move(page), req, itemCount, buildMs]() mutable {
					const auto tJson0 = std::chrono::steady_clock::now();
					auto payload = PrismaUIPayloads::BuildInventoryPayload(req.page, req.pageSize, page);
					const auto tJson1 = std::chrono::steady_clock::now();
					const auto jsonMs =
						static_cast<std::uint32_t>(
							std::chrono::duration_cast<std::chrono::milliseconds>(tJson1 - tJson0).count());

					const auto tSend0 = std::chrono::steady_clock::now();
					SendJS("copng_setInventory", payload);
					const auto tSend1 = std::chrono::steady_clock::now();
					const auto sendMs =
						static_cast<std::uint32_t>(
							std::chrono::duration_cast<std::chrono::milliseconds>(tSend1 - tSend0).count());

					SKSE::log::info(
						"Inventory: json {}ms + send {}ms for {} items (build {}ms)",
						jsonMs, sendMs, itemCount, buildMs);
				});
			})) {
			return;
		}

		// Fallback (no task interface): synchronous
		const auto offset = static_cast<std::size_t>(req.page) * static_cast<std::size_t>(req.pageSize);
		auto page = Registration::BuildQuickRegisterList(offset, req.pageSize);
		SendJS("copng_setInventory", PrismaUIPayloads::BuildInventoryPayload(req.page, req.pageSize, page));
	}

	void QueueSendRegistered() noexcept
	{
		if (QueueMainTask([]() {
				auto items = Registration::BuildRegisteredList();
				(void)QueueUITask([items = std::move(items)]() mutable {
					SendJS("copng_setRegistered", PrismaUIPayloads::BuildRegisteredPayload(items));
				});
			})) {
			return;
		}

		auto items = Registration::BuildRegisteredList();
		SendJS("copng_setRegistered", PrismaUIPayloads::BuildRegisteredPayload(items));
	}

	void QueueSendRewards() noexcept
	{
		auto gatherRewardState = []() {
			std::size_t registeredCount = 0;
			std::vector<std::pair<RE::ActorValue, float>> totals;
			{
				auto& state = GetState();
				std::scoped_lock lock(state.mutex);
				registeredCount = state.registeredItems.size();
				totals.reserve(state.rewardTotals.size());
				for (const auto& [av, total] : state.rewardTotals) {
					if (total != 0.0f) {
						totals.emplace_back(av, total);
					}
				}
			}
			return std::make_pair(registeredCount, std::move(totals));
		};

		auto buildRewardsJson = [](std::size_t registeredCount,
		                           std::vector<std::pair<RE::ActorValue, float>>& totals,
		                           bool useL10n) {
			const auto settings = GetSettings();
			const auto every = settings.rewardEvery > 0 ? settings.rewardEvery : 0;
			const auto rolls = (every > 0) ?
				                   static_cast<std::int32_t>(registeredCount / static_cast<std::size_t>(every)) :
				                   0;

			json j;
			j["registeredCount"] = registeredCount;
			j["rewardEvery"] = every;
			j["rewardMultiplier"] = settings.rewardMultiplier;
			j["rolls"] = rolls;
			j["totals"] = PrismaUIPayloads::BuildRewardTotalsArray(totals, useL10n);
			return j;
		};

		if (QueueMainTask([gatherRewardState, buildRewardsJson]() {
				auto [registeredCount, totals] = gatherRewardState();
				(void)QueueUITask([registeredCount, totals = std::move(totals), buildRewardsJson]() mutable {
					SendJS("copng_setRewards", buildRewardsJson(registeredCount, totals, true));
				});
			})) {
			return;
		}

		// Fallback (no task interface): minimal synchronous snapshot
		auto [registeredCount, totals] = gatherRewardState();
		SendJS("copng_setRewards", buildRewardsJson(registeredCount, totals, false));
	}

	void HandleRefundRewardsRequest() noexcept
	{
		if (QueueMainTask([]() {
				const auto cleared = Rewards::RefundRewards();
				(void)QueueUITask([cleared]() {
					ShowToast("info", "Rewards refunded (" + std::to_string(cleared) + ")");
					SendStateToUI();
					QueueSendRewards();
				});
			})) {
			return;
		}

		const auto cleared = Rewards::RefundRewards();
		ShowToast("info", "Rewards refunded (" + std::to_string(cleared) + ")");
		SendStateToUI();
		QueueSendRewards();
	}

	void HandleRegisterItemRequest(const char* argument) noexcept
	{
		json payload;
		try {
			payload = json::parse(argument ? argument : "{}");
		} catch (...) {
			ShowToast("error", "Invalid JSON");
			return;
		}

		auto formIdIt = payload.find("formId");
		if (formIdIt == payload.end()) {
			ShowToast("error", "Missing formId");
			return;
		}

		auto formIdOpt = ParseFormIDFromJson(*formIdIt);
		if (!formIdOpt) {
			ShowToast("error", "Invalid formId");
			return;
		}

		const auto formId = *formIdOpt;
		SKSE::log::info("JS requested register item (formId: 0x{:08X})", static_cast<std::uint32_t>(formId));

		if (QueueMainTask([formId]() {
				const auto res = Registration::TryRegisterItem(formId);
				(void)QueueUITask([res]() {
					SKSE::log::info(
						"Register item result: success={} regKey=0x{:08X} group={} total={} msg='{}'",
						res.success,
						static_cast<std::uint32_t>(res.regKey),
						res.group,
						res.totalRegistered,
						res.message);
					ShowToast(res.success ? "info" : "error", res.message);
					SendStateToUI();
					QueueSendInventory(InventoryRequest{});
					QueueSendRegistered();
					QueueSendRewards();
				});
			})) {
			return;
		}

		const auto res = Registration::TryRegisterItem(formId);
		SKSE::log::info(
			"Register item result (sync): success={} regKey=0x{:08X} group={} total={} msg='{}'",
			res.success,
			static_cast<std::uint32_t>(res.regKey),
			res.group,
			res.totalRegistered,
			res.message);
		ShowToast(res.success ? "info" : "error", res.message);
		SendStateToUI();
		QueueSendInventory(InventoryRequest{});
		QueueSendRegistered();
		QueueSendRewards();
	}
}
