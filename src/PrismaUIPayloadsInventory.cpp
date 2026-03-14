#include "PrismaUIPayloads.h"

namespace CodexOfPowerNG::PrismaUIPayloads
{
	namespace
	{
		[[nodiscard]] const char* GroupToDiscipline(std::uint32_t group) noexcept
		{
			switch (group) {
			case 0u:
				return "attack";
			case 1u:
				return "defense";
			default:
				return "utility";
			}
		}

		[[nodiscard]] json BuildInventorySections(const Registration::QuickRegisterList& pageData) noexcept
		{
			json sections = json::array();
			json* currentSection = nullptr;
			std::string_view currentDiscipline{};

			for (const auto& it : pageData.items) {
				const auto discipline = std::string_view{ GroupToDiscipline(it.group) };
				if (!currentSection || currentDiscipline != discipline) {
					sections.push_back({
						{ "group", it.group },
						{ "groupName", Registration::GetDiscoveryGroupName(it.group) },
						{ "discipline", discipline },
						{ "rows", json::array() },
					});
					currentSection = &sections.back();
					currentDiscipline = discipline;
				}

				(*currentSection)["rows"].push_back({
					{ "formId", it.formId },
					{ "regKey", it.regKey },
					{ "name", it.name },
					{ "group", it.group },
					{ "groupName", Registration::GetDiscoveryGroupName(it.group) },
					{ "discipline", discipline },
					{ "totalCount", it.totalCount },
					{ "safeCount", it.safeCount },
					{ "buildPoints", Builds::FromBuildPointCenti(it.buildPointsCenti) },
					{ "buildPointsCenti", it.buildPointsCenti },
					{ "actionable", it.safeCount > 0 && it.disabledReason.empty() },
					{ "disabledReason", it.disabledReason.empty() ? json(nullptr) : json(it.disabledReason) },
				});
			}

			return sections;
		}
	}

	json BuildInventoryPayload(
		std::uint32_t page,
		std::uint32_t pageSize,
		Registration::QuickRegisterList& pageData) noexcept
	{
		json payload;
		payload["page"] = page;
		payload["pageSize"] = pageSize;
		payload["total"] = pageData.total;
		payload["hasMore"] = pageData.hasMore;

		json arr = json::array();
		for (const auto& it : pageData.items) {
			arr.push_back({
				{ "formId", it.formId },
				{ "regKey", it.regKey },
				{ "name", it.name },
				{ "group", it.group },
				{ "groupName", Registration::GetDiscoveryGroupName(it.group) },
				{ "totalCount", it.totalCount },
				{ "safeCount", it.safeCount },
				{ "buildPoints", Builds::FromBuildPointCenti(it.buildPointsCenti) },
				{ "buildPointsCenti", it.buildPointsCenti },
			});
		}
		payload["items"] = std::move(arr);
		payload["sections"] = BuildInventorySections(pageData);
		return payload;
	}

	json BuildRegisteredPayload(const std::vector<Registration::ListItem>& items) noexcept
	{
		json arr = json::array();
		for (const auto& it : items) {
			arr.push_back({
				{ "formId", it.formId },
				{ "name", it.name },
				{ "group", it.group },
				{ "groupName", Registration::GetDiscoveryGroupName(it.group) },
			});
		}
		return arr;
	}

	json BuildUndoPayload(const std::vector<Registration::UndoListItem>& items) noexcept
	{
		json arr = json::array();
		for (const auto& it : items) {
			arr.push_back({
				{ "actionId", it.actionId },
				{ "formId", it.formId },
				{ "regKey", it.regKey },
				{ "name", it.name },
				{ "group", it.group },
				{ "groupName", Registration::GetDiscoveryGroupName(it.group) },
				{ "canUndo", it.canUndo },
				{ "hasRewardDelta", it.hasRewardDelta },
			});
		}
		return arr;
	}
}
