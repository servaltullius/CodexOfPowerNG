#include "PrismaUIPayloads.h"

namespace CodexOfPowerNG::PrismaUIPayloads
{
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
			});
		}
		payload["items"] = std::move(arr);
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
