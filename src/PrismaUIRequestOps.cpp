#include "PrismaUIRequestOps.h"

#include "CodexOfPowerNG/Config.h"
#include "CodexOfPowerNG/PrismaUIManager.h"
#include "CodexOfPowerNG/Registration.h"
#include "CodexOfPowerNG/RegistrationStateStore.h"
#include "CodexOfPowerNG/Rewards.h"
#include "CodexOfPowerNG/TaskScheduler.h"
#include "PrismaUIPayloads.h"

#include <SKSE/Logger.h>

#include <algorithm>
#include <atomic>
#include <charconv>
#include <chrono>
#include <cstdint>
#include <exception>
#include <mutex>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace CodexOfPowerNG::PrismaUIManager::Internal
{
	namespace
	{
		std::mutex       g_inventoryRequestMutex;
		InventoryRequest g_lastInventoryRequest{};
		std::atomic_bool g_refreshPending{ false };

		void RememberInventoryRequest(InventoryRequest req) noexcept
		{
			req.pageSize = std::clamp(req.pageSize, std::uint32_t{ 1 }, std::uint32_t{ 500 });
			std::scoped_lock lock(g_inventoryRequestMutex);
			g_lastInventoryRequest = req;
		}

		[[nodiscard]] InventoryRequest SnapshotLastInventoryRequest() noexcept
		{
			std::scoped_lock lock(g_inventoryRequestMutex);
			return g_lastInventoryRequest;
		}

		[[nodiscard]] std::optional<json> ParseJsonPayload(
			const char* argument,
			const char* contextLabel) noexcept
		{
			try {
				return json::parse(argument ? argument : "{}");
			} catch (const json::parse_error& e) {
				SKSE::log::warn("{} parse error: {}", contextLabel, e.what());
			} catch (const json::exception& e) {
				SKSE::log::warn("{} JSON exception: {}", contextLabel, e.what());
			} catch (const std::exception& e) {
				SKSE::log::warn("{} unexpected exception: {}", contextLabel, e.what());
			}
			return std::nullopt;
		}

		void RefreshUIAfterMutation() noexcept
		{
			SendStateToUI();
			QueueSendInventory(SnapshotLastInventoryRequest());
			QueueSendRegistered();
			QueueSendRewards();
			QueueSendUndoList();
		}

		void PresentRegisterResult(const Registration::RegisterResult& res) noexcept
		{
			SKSE::log::info(
				"Register item result: success={} regKey=0x{:08X} group={} total={} msg='{}'",
				res.success,
				static_cast<std::uint32_t>(res.regKey),
				res.group,
				res.totalRegistered,
				res.message);

			ShowToast(res.success ? "info" : "error", res.message);
			RefreshUIAfterMutation();
		}

		void PresentUndoResult(const Registration::UndoResult& res) noexcept
		{
			SKSE::log::info(
				"Undo register result: success={} actionId={} regKey=0x{:08X} total={} msg='{}'",
				res.success,
				res.actionId,
				static_cast<std::uint32_t>(res.regKey),
				res.totalRegistered,
				res.message);

			ShowToast(res.success ? "info" : "error", res.message);
			RefreshUIAfterMutation();
		}

		void ReportMainTaskQueueUnavailable(const char* context) noexcept
		{
			g_refreshPending.store(true, std::memory_order_release);
			SKSE::log::warn(
				"{}: main task queue unavailable; request dropped (pending refresh marked)",
				context ? context : "Request");
		}
	}

	void FlushPendingUIRefresh() noexcept
	{
		if (!g_refreshPending.exchange(false, std::memory_order_acq_rel)) {
			return;
		}

		SKSE::log::info("UI refresh: replaying pending refresh after queue recovery");
		SendStateToUI();
		QueueSendInventory(SnapshotLastInventoryRequest());
		QueueSendRegistered();
		QueueSendRewards();
		QueueSendUndoList();
	}

	InventoryRequest ParseInventoryRequest(const char* argument) noexcept
	{
		InventoryRequest out{};
		const auto payloadOpt = ParseJsonPayload(argument, "Inventory request JSON");
		if (!payloadOpt) {
			return out;
		}
		const auto& payload = *payloadOpt;

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
		} catch (const json::exception& e) {
			SKSE::log::warn("Inventory request field parse error: {}", e.what());
		} catch (const std::exception& e) {
			SKSE::log::warn("Inventory request field parse exception: {}", e.what());
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
		} catch (const json::exception& e) {
			SKSE::log::warn("formId parse error: {}", e.what());
		} catch (const std::exception& e) {
			SKSE::log::warn("formId parse exception: {}", e.what());
		}

		return std::nullopt;
	}

	std::optional<std::uint64_t> ParseActionIdFromJson(const json& j) noexcept
	{
		try {
			if (j.is_number_unsigned()) {
				return j.get<std::uint64_t>();
			}
			if (j.is_number_integer()) {
				const auto v = j.get<std::int64_t>();
				if (v < 0) {
					return std::nullopt;
				}
				return static_cast<std::uint64_t>(v);
			}
			if (j.is_string()) {
				const auto s = j.get<std::string>();
				std::uint64_t value{};
				const auto* begin = s.data();
				const auto* end = s.data() + s.size();
				auto [ptr, ec] = std::from_chars(begin, end, value, 10);
				if (ec != std::errc{} || ptr != end) {
					return std::nullopt;
				}
				return value;
			}
		} catch (const json::exception& e) {
			SKSE::log::warn("actionId parse error: {}", e.what());
		} catch (const std::exception& e) {
			SKSE::log::warn("actionId parse exception: {}", e.what());
		}

		return std::nullopt;
	}

	void QueueSendInventory(InventoryRequest req) noexcept
	{
		RememberInventoryRequest(req);

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

		ReportMainTaskQueueUnavailable("Inventory");
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

		ReportMainTaskQueueUnavailable("Registered list");
	}

	void QueueSendRewards() noexcept
	{
		auto gatherRewardState = []() {
			const auto registeredCount = RegistrationStateStore::RegisteredCount();
			auto totals = Rewards::SnapshotRewardTotals();
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

		ReportMainTaskQueueUnavailable("Rewards");
	}

	void QueueSendUndoList() noexcept
	{
		if (QueueMainTask([]() {
				auto items = Registration::BuildRecentUndoList();
				(void)QueueUITask([items = std::move(items)]() mutable {
					SendJS("copng_setUndoList", PrismaUIPayloads::BuildUndoPayload(items));
				});
			})) {
			return;
		}

		ReportMainTaskQueueUnavailable("Undo list");
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

		ReportMainTaskQueueUnavailable("Refund rewards");
		ShowToast("error", "Main thread unavailable. Try again.");
	}

	void HandleRegisterItemRequest(const char* argument) noexcept
	{
		const auto payloadOpt = ParseJsonPayload(argument, "Register item payload");
		if (!payloadOpt) {
			ShowToast("error", "Invalid JSON");
			return;
		}
		const auto& payload = *payloadOpt;

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
					PresentRegisterResult(res);
				});
			})) {
			return;
		}

		ReportMainTaskQueueUnavailable("Register item");
		ShowToast("error", "Main thread unavailable. Try again.");
	}

	void HandleUndoRegisterRequest(const char* argument) noexcept
	{
		const auto payloadOpt = ParseJsonPayload(argument, "Undo payload");
		if (!payloadOpt) {
			ShowToast("error", "Invalid JSON");
			return;
		}
		const auto& payload = *payloadOpt;

		const auto actionIdIt = payload.find("actionId");
		if (actionIdIt == payload.end()) {
			ShowToast("error", "Missing actionId");
			return;
		}

		const auto actionIdOpt = ParseActionIdFromJson(*actionIdIt);
		if (!actionIdOpt) {
			ShowToast("error", "Invalid actionId");
			return;
		}

		const auto actionId = *actionIdOpt;
		SKSE::log::info("JS requested undo register item (actionId: {})", actionId);

		if (QueueMainTask([actionId]() {
				const auto res = Registration::TryUndoRegistration(actionId);
				(void)QueueUITask([res]() {
					PresentUndoResult(res);
				});
			})) {
			return;
		}

		ReportMainTaskQueueUnavailable("Undo register item");
		ShowToast("error", "Main thread unavailable. Try again.");
	}
	}
