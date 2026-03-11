#include "PrismaUIRequestOps.h"

#include <iostream>
#include <string_view>

namespace
{
	using CodexOfPowerNG::PrismaUIManager::Internal::BuildMutationGuard;
	using CodexOfPowerNG::PrismaUIManager::Internal::ParseBuildActivateRequest;
	using CodexOfPowerNG::PrismaUIManager::Internal::ParseRegisterBatchRequest;
	using CodexOfPowerNG::PrismaUIManager::Internal::ValidateBuildMutationRequest;
	using CodexOfPowerNG::PrismaUIManager::Internal::json;

	bool RejectsBuildActivationWhileInCombat()
	{
		const auto request = ParseBuildActivateRequest(json{
			{ "optionId", "build.attack.ferocity" },
			{ "slotId", "attack_1" },
		});
		if (!request.has_value()) {
			return false;
		}

		return ValidateBuildMutationRequest(true, request.has_value()) ==
		       BuildMutationGuard::kCombatLocked;
	}

	bool RejectsInvalidBuildPayload()
	{
		const auto request = ParseBuildActivateRequest(json{
			{ "optionId", "" },
			{ "slotId", "not_a_slot" },
		});
		return !request.has_value() &&
		       ValidateBuildMutationRequest(false, request.has_value()) ==
			       BuildMutationGuard::kInvalidPayload;
	}

	bool BatchRequestKeepsOneRowEqualsOneRegistration()
	{
		const auto request = ParseRegisterBatchRequest(json{
			{ "formIds", json::array({ 46775u, 51234u, 61234u }) },
		});
		return request.has_value() &&
		       request->formIds.size() == 3u &&
		       request->formIds[0] == 46775u &&
		       request->formIds[1] == 51234u &&
		       request->formIds[2] == 61234u;
	}
}

int main()
{
	const auto expect = [](bool condition, std::string_view message) {
		if (!condition) {
			std::cerr << "build_request_guards: " << message << '\n';
			return false;
		}
		return true;
	};

	if (!expect(RejectsBuildActivationWhileInCombat(), "build activation must be combat-locked")) {
		return 1;
	}
	if (!expect(RejectsInvalidBuildPayload(), "invalid build payload must be rejected")) {
		return 1;
	}
	if (!expect(
			BatchRequestKeepsOneRowEqualsOneRegistration(),
			"batch request must preserve one row per registration payload entry")) {
		return 1;
	}

	return 0;
}
