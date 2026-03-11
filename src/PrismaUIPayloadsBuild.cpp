#include "PrismaUIPayloads.h"

#include "CodexOfPowerNG/BuildOptionCatalog.h"
#include "CodexOfPowerNG/BuildProgression.h"
#include "CodexOfPowerNG/BuildStateStore.h"

namespace CodexOfPowerNG::PrismaUIPayloads
{
	namespace
	{
		[[nodiscard]] const char* DisciplineToJs(Builds::BuildDiscipline discipline) noexcept
		{
			switch (discipline) {
			case Builds::BuildDiscipline::Attack:
				return "attack";
			case Builds::BuildDiscipline::Defense:
				return "defense";
			case Builds::BuildDiscipline::Utility:
				return "utility";
			}
			return "utility";
		}

		[[nodiscard]] std::uint32_t DisciplineScore(Builds::BuildDiscipline discipline) noexcept
		{
			switch (discipline) {
			case Builds::BuildDiscipline::Attack:
				return BuildStateStore::GetAttackScore();
			case Builds::BuildDiscipline::Defense:
				return BuildStateStore::GetDefenseScore();
			case Builds::BuildDiscipline::Utility:
				return BuildStateStore::GetUtilityScore();
			}
			return 0u;
		}

		[[nodiscard]] const char* SlotKindToJs(Builds::BuildSlotId slotId) noexcept
		{
			switch (slotId) {
			case Builds::BuildSlotId::Attack1:
			case Builds::BuildSlotId::Attack2:
				return "attack";
			case Builds::BuildSlotId::Defense1:
				return "defense";
			case Builds::BuildSlotId::Utility1:
			case Builds::BuildSlotId::Utility2:
				return "utility";
			case Builds::BuildSlotId::Wildcard1:
				return "wildcard";
			}
			return "wildcard";
		}

		[[nodiscard]] const char* SlotIdToJs(Builds::BuildSlotId slotId) noexcept
		{
			switch (slotId) {
			case Builds::BuildSlotId::Attack1:
				return "attack_1";
			case Builds::BuildSlotId::Attack2:
				return "attack_2";
			case Builds::BuildSlotId::Defense1:
				return "defense_1";
			case Builds::BuildSlotId::Utility1:
				return "utility_1";
			case Builds::BuildSlotId::Utility2:
				return "utility_2";
			case Builds::BuildSlotId::Wildcard1:
				return "wildcard_1";
			}
			return "wildcard_1";
		}

		[[nodiscard]] const char* SlotCompatibilityToJs(Builds::BuildSlotCompatibility compatibility) noexcept
		{
			switch (compatibility) {
			case Builds::BuildSlotCompatibility::SameDisciplineOnly:
				return "same_discipline_only";
			case Builds::BuildSlotCompatibility::WildcardOnly:
				return "wildcard_only";
			case Builds::BuildSlotCompatibility::SameOrWildcard:
				return "same_or_wildcard";
			}
			return "same_or_wildcard";
		}

		[[nodiscard]] const char* EffectTypeToJs(Builds::BuildEffectType effectType) noexcept
		{
			switch (effectType) {
			case Builds::BuildEffectType::ActorValue:
				return "actor_value";
			case Builds::BuildEffectType::CarryWeight:
				return "carry_weight";
			case Builds::BuildEffectType::Economy:
				return "economy";
			case Builds::BuildEffectType::UtilityFlag:
				return "utility_flag";
			}
			return "utility_flag";
		}

		[[nodiscard]] const char* BuildLayerToJs(Builds::BuildLayer layer) noexcept
		{
			switch (layer) {
			case Builds::BuildLayer::Baseline:
				return "baseline";
			case Builds::BuildLayer::Slotted:
				return "slotted";
			}
			return "slotted";
		}

		[[nodiscard]] json MagnitudeToJson(const Builds::BuildMagnitude& magnitude) noexcept
		{
			if (std::holds_alternative<float>(magnitude)) {
				return json(std::get<float>(magnitude));
			}
			return json(std::get<std::int32_t>(magnitude));
		}
	}

	json BuildBuildPayload() noexcept
	{
		json payload;
		payload["disciplines"] = {
			{ "attack", {
				{ "score", BuildStateStore::GetAttackScore() },
				{ "unlockedBaselineCount", BuildProgression::CountUnlockedBaselineMilestones(
					Builds::BuildDiscipline::Attack,
					BuildStateStore::GetAttackScore()) },
			} },
			{ "defense", {
				{ "score", BuildStateStore::GetDefenseScore() },
				{ "unlockedBaselineCount", BuildProgression::CountUnlockedBaselineMilestones(
					Builds::BuildDiscipline::Defense,
					BuildStateStore::GetDefenseScore()) },
			} },
			{ "utility", {
				{ "score", BuildStateStore::GetUtilityScore() },
				{ "unlockedBaselineCount", BuildProgression::CountUnlockedBaselineMilestones(
					Builds::BuildDiscipline::Utility,
					BuildStateStore::GetUtilityScore()) },
			} },
		};

		json options = json::array();
		for (const auto& option : Builds::GetBuildOptionCatalog()) {
			const auto activeScore = DisciplineScore(option.discipline);
			options.push_back({
				{ "id", option.id },
				{ "discipline", DisciplineToJs(option.discipline) },
				{ "layer", BuildLayerToJs(option.layer) },
				{ "unlockScore", option.unlockScore },
				{ "unlocked", activeScore >= option.unlockScore },
				{ "slotCompatibility", SlotCompatibilityToJs(option.slotCompatibility) },
				{ "effectType", EffectTypeToJs(option.effectType) },
				{ "effectKey", option.effectKey },
				{ "magnitude", MagnitudeToJson(option.magnitude) },
				{ "exclusivityGroup", option.exclusivityGroup },
				{ "titleKey", option.titleKey },
				{ "descriptionKey", option.descriptionKey },
			});
		}
		payload["options"] = std::move(options);

		json activeSlots = json::array();
		for (const auto slotId : Builds::GetInitialBuildSlotLayout()) {
			const auto activeOption = BuildStateStore::GetActiveSlot(slotId);
			activeSlots.push_back({
				{ "slotId", SlotIdToJs(slotId) },
				{ "slotKind", SlotKindToJs(slotId) },
				{ "optionId", activeOption.has_value() ? json(activeOption.value()) : json(nullptr) },
				{ "occupied", activeOption.has_value() },
			});
		}
		payload["activeSlots"] = std::move(activeSlots);

		const auto migrationNotice = BuildStateStore::GetMigrationNoticeSnapshot();
		payload["migrationNotice"] = {
			{ "needsNotice", migrationNotice.needsNotice },
			{ "legacyRewardsMigrated", migrationNotice.legacyRewardsMigrated },
			{ "unresolvedHistoricalRegistrations", migrationNotice.unresolvedHistoricalRegistrations },
		};
		return payload;
	}
}
