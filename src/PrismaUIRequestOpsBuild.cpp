#include "PrismaUIRequestOps.h"

#include <array>
#include <charconv>
#include <cstdint>
#include <optional>
#include <string_view>

namespace CodexOfPowerNG::PrismaUIManager::Internal
{
	namespace
	{
		struct SlotNameEntry
		{
			std::string_view     raw;
			Builds::BuildSlotId slotId;
		};

		constexpr std::array kSlotNames{
			SlotNameEntry{ "attack_1", Builds::BuildSlotId::Attack1 },
			SlotNameEntry{ "attack_2", Builds::BuildSlotId::Attack2 },
			SlotNameEntry{ "defense_1", Builds::BuildSlotId::Defense1 },
			SlotNameEntry{ "utility_1", Builds::BuildSlotId::Utility1 },
			SlotNameEntry{ "utility_2", Builds::BuildSlotId::Utility2 },
			SlotNameEntry{ "wildcard_1", Builds::BuildSlotId::Wildcard1 },
		};

		[[nodiscard]] std::optional<RE::FormID> ParseBatchFormId(const json& value) noexcept
		{
			try {
				if (value.is_number_unsigned()) {
					return static_cast<RE::FormID>(value.get<std::uint32_t>());
				}
				if (value.is_number_integer()) {
					const auto parsed = value.get<std::int64_t>();
					if (parsed < 0 || parsed > 0xFFFFFFFFll) {
						return std::nullopt;
					}
					return static_cast<RE::FormID>(static_cast<std::uint32_t>(parsed));
				}
				if (value.is_string()) {
					const auto raw = value.get<std::string>();
					if (raw.empty()) {
						return std::nullopt;
					}

					auto* begin = raw.data();
					auto* end = raw.data() + raw.size();
					int   base = 10;
					if (raw.rfind("0x", 0) == 0 || raw.rfind("0X", 0) == 0) {
						begin += 2;
						base = 16;
					}

					std::uint32_t parsed{};
					const auto [ptr, ec] = std::from_chars(begin, end, parsed, base);
					if (ec != std::errc{} || ptr != end) {
						return std::nullopt;
					}
					return static_cast<RE::FormID>(parsed);
				}
			} catch (const json::exception&) {
				return std::nullopt;
			}

			return std::nullopt;
		}
	}

	std::optional<Builds::BuildSlotId> ParseBuildSlotId(std::string_view raw) noexcept
	{
		for (const auto& entry : kSlotNames) {
			if (entry.raw == raw) {
				return entry.slotId;
			}
		}
		return std::nullopt;
	}

	std::optional<BuildActivationRequest> ParseBuildActivateRequest(const json& payload) noexcept
	{
		if (!payload.is_object()) {
			return std::nullopt;
		}

		try {
			const auto optionIdIt = payload.find("optionId");
			const auto slotIdIt = payload.find("slotId");
			if (optionIdIt == payload.end() || slotIdIt == payload.end() ||
			    !optionIdIt->is_string() || !slotIdIt->is_string()) {
				return std::nullopt;
			}

			auto optionId = optionIdIt->get<std::string>();
			if (optionId.empty()) {
				return std::nullopt;
			}

			auto slotId = ParseBuildSlotId(slotIdIt->get<std::string>());
			if (!slotId.has_value()) {
				return std::nullopt;
			}

			return BuildActivationRequest{
				.optionId = std::move(optionId),
				.slotId = slotId.value(),
			};
		} catch (const json::exception&) {
			return std::nullopt;
		}
	}

	std::optional<BuildDeactivationRequest> ParseBuildDeactivateRequest(const json& payload) noexcept
	{
		if (!payload.is_object()) {
			return std::nullopt;
		}

		try {
			const auto slotIdIt = payload.find("slotId");
			if (slotIdIt == payload.end() || !slotIdIt->is_string()) {
				return std::nullopt;
			}

			auto slotId = ParseBuildSlotId(slotIdIt->get<std::string>());
			if (!slotId.has_value()) {
				return std::nullopt;
			}

			return BuildDeactivationRequest{
				.slotId = slotId.value(),
			};
		} catch (const json::exception&) {
			return std::nullopt;
		}
	}

	std::optional<BuildSwapRequest> ParseBuildSwapRequest(const json& payload) noexcept
	{
		if (!payload.is_object()) {
			return std::nullopt;
		}

		try {
			const auto optionIdIt = payload.find("optionId");
			const auto fromSlotIdIt = payload.find("fromSlotId");
			const auto toSlotIdIt = payload.find("toSlotId");
			if (optionIdIt == payload.end() || fromSlotIdIt == payload.end() || toSlotIdIt == payload.end() ||
			    !optionIdIt->is_string() || !fromSlotIdIt->is_string() || !toSlotIdIt->is_string()) {
				return std::nullopt;
			}

			auto optionId = optionIdIt->get<std::string>();
			if (optionId.empty()) {
				return std::nullopt;
			}

			auto fromSlotId = ParseBuildSlotId(fromSlotIdIt->get<std::string>());
			auto toSlotId = ParseBuildSlotId(toSlotIdIt->get<std::string>());
			if (!fromSlotId.has_value() || !toSlotId.has_value()) {
				return std::nullopt;
			}

			return BuildSwapRequest{
				.optionId = std::move(optionId),
				.fromSlotId = fromSlotId.value(),
				.toSlotId = toSlotId.value(),
			};
		} catch (const json::exception&) {
			return std::nullopt;
		}
	}

	std::optional<RegisterBatchRequest> ParseRegisterBatchRequest(const json& payload) noexcept
	{
		if (!payload.is_object()) {
			return std::nullopt;
		}

		const auto formIdsIt = payload.find("formIds");
		if (formIdsIt == payload.end() || !formIdsIt->is_array()) {
			return std::nullopt;
		}

		RegisterBatchRequest request{};
		request.formIds.reserve(formIdsIt->size());
		for (const auto& rawValue : *formIdsIt) {
			const auto formId = ParseBatchFormId(rawValue);
			if (!formId.has_value() || formId.value() == 0u) {
				return std::nullopt;
			}
			request.formIds.push_back(formId.value());
		}

		if (request.formIds.empty()) {
			return std::nullopt;
		}
		return request;
	}

	BuildMutationGuard ValidateBuildMutationRequest(bool inCombat, bool payloadValid) noexcept
	{
		if (!payloadValid) {
			return BuildMutationGuard::kInvalidPayload;
		}
		if (inCombat) {
			return BuildMutationGuard::kCombatLocked;
		}
		return BuildMutationGuard::kAllowed;
	}
}
