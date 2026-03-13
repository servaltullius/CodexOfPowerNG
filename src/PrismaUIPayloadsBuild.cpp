#include "PrismaUIPayloads.h"

#include "CodexOfPowerNG/BuildOptionCatalog.h"
#include "CodexOfPowerNG/BuildStateStore.h"

#include <algorithm>
#include <array>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace CodexOfPowerNG::PrismaUIPayloads
{
	namespace
	{
		struct ThemeDef
		{
			std::string_view id;
			std::string_view titleKey;
		};

		constexpr std::array kAttackThemes{
			ThemeDef{ "devastation", "build.theme.attack.devastation" },
			ThemeDef{ "precision", "build.theme.attack.precision" },
			ThemeDef{ "fury", "build.theme.attack.fury" },
		};

		constexpr std::array kDefenseThemes{
			ThemeDef{ "guard", "build.theme.defense.guard" },
			ThemeDef{ "bastion", "build.theme.defense.bastion" },
			ThemeDef{ "resistance", "build.theme.defense.resistance" },
		};

		constexpr std::array kUtilityThemes{
			ThemeDef{ "livelihood", "build.theme.utility.livelihood" },
			ThemeDef{ "exploration", "build.theme.utility.exploration" },
			ThemeDef{ "trickery", "build.theme.utility.trickery" },
		};

		constexpr std::array kHierarchyOrder{
			std::string_view{ "signpost" },
			std::string_view{ "standard" },
			std::string_view{ "special" },
		};

		[[nodiscard]] std::span<const ThemeDef> ThemeCatalog(Builds::BuildDiscipline discipline) noexcept
		{
			switch (discipline) {
			case Builds::BuildDiscipline::Attack:
				return kAttackThemes;
			case Builds::BuildDiscipline::Defense:
				return kDefenseThemes;
			case Builds::BuildDiscipline::Utility:
				return kUtilityThemes;
			}
			return {};
		}

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

		[[nodiscard]] bool SlotMatchesDiscipline(
			Builds::BuildDiscipline discipline,
			Builds::BuildSlotId slotId) noexcept
		{
			switch (discipline) {
			case Builds::BuildDiscipline::Attack:
				return slotId == Builds::BuildSlotId::Attack1 || slotId == Builds::BuildSlotId::Attack2;
			case Builds::BuildDiscipline::Defense:
				return slotId == Builds::BuildSlotId::Defense1;
			case Builds::BuildDiscipline::Utility:
				return slotId == Builds::BuildSlotId::Utility1 || slotId == Builds::BuildSlotId::Utility2;
			}
			return false;
		}

		[[nodiscard]] bool IsSlotCompatible(
			const Builds::BuildOptionDef& option,
			Builds::BuildSlotId slotId) noexcept
		{
			switch (option.slotCompatibility) {
			case Builds::BuildSlotCompatibility::SameDisciplineOnly:
				return SlotMatchesDiscipline(option.discipline, slotId);
			case Builds::BuildSlotCompatibility::WildcardOnly:
				return slotId == Builds::BuildSlotId::Wildcard1;
			case Builds::BuildSlotCompatibility::SameOrWildcard:
				return SlotMatchesDiscipline(option.discipline, slotId) ||
				       slotId == Builds::BuildSlotId::Wildcard1;
			}
			return false;
		}

		[[nodiscard]] std::optional<Builds::BuildSlotId> FindActiveSlotForOption(std::string_view optionId) noexcept
		{
			for (const auto slotId : Builds::GetInitialBuildSlotLayout()) {
				const auto activeOption = BuildStateStore::GetActiveSlot(slotId);
				if (activeOption.has_value() && activeOption.value() == optionId) {
					return slotId;
				}
			}
			return std::nullopt;
		}

		[[nodiscard]] std::uint32_t CountOptionsForTheme(
			Builds::BuildDiscipline discipline,
			std::string_view themeId) noexcept
		{
			std::uint32_t count = 0;
			for (const auto& option : Builds::GetBuildOptionCatalog()) {
				if (option.discipline == discipline && option.themeId == themeId) {
					++count;
				}
			}
			return count;
		}

		[[nodiscard]] int HierarchyRank(std::string_view hierarchy) noexcept
		{
			for (std::size_t index = 0; index < kHierarchyOrder.size(); ++index) {
				if (kHierarchyOrder[index] == hierarchy) {
					return static_cast<int>(index);
				}
			}
			return static_cast<int>(kHierarchyOrder.size());
		}

		[[nodiscard]] const char* OptionStateToJs(
			bool unlocked,
			bool active) noexcept
		{
			if (active) {
				return "active";
			}
			if (unlocked) {
				return "unlocked";
			}
			return "locked";
		}

		[[nodiscard]] json CompatibleSlotIdsJson(const Builds::BuildOptionDef& option) noexcept
		{
			json slots = json::array();
			for (const auto slotId : Builds::GetInitialBuildSlotLayout()) {
				if (IsSlotCompatible(option, slotId)) {
					slots.push_back(SlotIdToJs(slotId));
				}
			}
			return slots;
		}

		[[nodiscard]] json BuildCatalogRowPayload(
			const Builds::BuildOptionDef& option,
			std::uint32_t activeScore) noexcept
		{
			const bool unlocked = activeScore >= option.unlockScore;
			const auto activeSlotId = FindActiveSlotForOption(option.id);
			const auto currentTier = Builds::GetBuildScalingTier(activeScore);
			const auto nextTierScore = Builds::GetNextBuildScalingScore(activeScore);
			const auto scoreToNextTier = Builds::GetScoreToNextBuildScalingTier(activeScore);
			const auto currentMagnitude = Builds::GetScaledBuildMagnitude(option, activeScore);
			const auto nextMagnitude = Builds::GetNextTierBuildMagnitude(option, activeScore);
			return {
				{ "id", option.id },
				{ "optionId", option.id },
				{ "discipline", DisciplineToJs(option.discipline) },
				{ "themeId", option.themeId },
				{ "themeTitleKey", option.themeTitleKey },
				{ "hierarchy", option.hierarchy },
				{ "unlockScore", option.unlockScore },
				{ "unlocked", unlocked },
				{ "state", OptionStateToJs(unlocked, activeSlotId.has_value()) },
				{ "slotCompatibility", SlotCompatibilityToJs(option.slotCompatibility) },
				{ "compatibleSlots", CompatibleSlotIdsJson(option) },
				{ "activeSlotId", activeSlotId.has_value() ? json(SlotIdToJs(activeSlotId.value())) : json(nullptr) },
				{ "titleKey", option.titleKey },
				{ "descriptionKey", option.descriptionKey },
				{ "effectType", EffectTypeToJs(option.effectType) },
				{ "effectKey", option.effectKey },
				{ "magnitude", MagnitudeToJson(currentMagnitude) },
				{ "baseMagnitude", MagnitudeToJson(option.magnitude) },
				{ "magnitudePerTier", MagnitudeToJson(option.magnitudePerTier) },
				{ "currentMagnitude", MagnitudeToJson(currentMagnitude) },
				{ "nextMagnitude", MagnitudeToJson(nextMagnitude) },
				{ "currentTier", currentTier },
				{ "nextTierScore", nextTierScore },
				{ "scoreToNextTier", scoreToNextTier },
				{ "exclusivityGroup", option.exclusivityGroup },
			};
		}

		[[nodiscard]] std::vector<const Builds::BuildOptionDef*> CollectThemeRows(
			Builds::BuildDiscipline discipline,
			std::string_view themeId) noexcept
		{
			std::vector<const Builds::BuildOptionDef*> rows;
			for (const auto& option : Builds::GetBuildOptionCatalog()) {
				if (option.discipline == discipline && option.themeId == themeId) {
					rows.push_back(&option);
				}
			}

			std::sort(
				rows.begin(),
				rows.end(),
				[](const Builds::BuildOptionDef* lhs, const Builds::BuildOptionDef* rhs) {
					const auto lhsRank = HierarchyRank(lhs->hierarchy);
					const auto rhsRank = HierarchyRank(rhs->hierarchy);
					if (lhsRank != rhsRank) {
						return lhsRank < rhsRank;
					}
					if (lhs->unlockScore != rhs->unlockScore) {
						return lhs->unlockScore < rhs->unlockScore;
					}
					return lhs->id < rhs->id;
				});

			return rows;
		}

		[[nodiscard]] std::string_view SelectDefaultThemeId(Builds::BuildDiscipline discipline) noexcept
		{
			const auto themeDefs = ThemeCatalog(discipline);
			for (const auto& theme : themeDefs) {
				if (CountOptionsForTheme(discipline, theme.id) > 0u) {
					return theme.id;
				}
			}
			if (!themeDefs.empty()) {
				return themeDefs.front().id;
			}
			return {};
		}

		[[nodiscard]] const Builds::BuildOptionDef* SelectDefaultOption(
			Builds::BuildDiscipline discipline,
			std::string_view themeId) noexcept
		{
			const auto activeScore = DisciplineScore(discipline);
			const auto rows = CollectThemeRows(discipline, themeId);
			for (const auto* option : rows) {
				if (FindActiveSlotForOption(option->id).has_value()) {
					return option;
				}
			}
			for (const auto* option : rows) {
				if (activeScore >= option->unlockScore) {
					return option;
				}
			}
			return rows.empty() ? nullptr : rows.front();
		}
	}

	json BuildBuildPayload() noexcept
	{
		json payload;
		payload["disciplines"] = {
			{ "attack", {
				{ "score", BuildStateStore::GetAttackScore() },
				{ "currentTier", Builds::GetBuildScalingTier(BuildStateStore::GetAttackScore()) },
				{ "nextTierScore", Builds::GetNextBuildScalingScore(BuildStateStore::GetAttackScore()) },
				{ "scoreToNextTier", Builds::GetScoreToNextBuildScalingTier(BuildStateStore::GetAttackScore()) },
			} },
			{ "defense", {
				{ "score", BuildStateStore::GetDefenseScore() },
				{ "currentTier", Builds::GetBuildScalingTier(BuildStateStore::GetDefenseScore()) },
				{ "nextTierScore", Builds::GetNextBuildScalingScore(BuildStateStore::GetDefenseScore()) },
				{ "scoreToNextTier", Builds::GetScoreToNextBuildScalingTier(BuildStateStore::GetDefenseScore()) },
			} },
			{ "utility", {
				{ "score", BuildStateStore::GetUtilityScore() },
				{ "currentTier", Builds::GetBuildScalingTier(BuildStateStore::GetUtilityScore()) },
				{ "nextTierScore", Builds::GetNextBuildScalingScore(BuildStateStore::GetUtilityScore()) },
				{ "scoreToNextTier", Builds::GetScoreToNextBuildScalingTier(BuildStateStore::GetUtilityScore()) },
			} },
		};

		payload["hierarchyOrder"] = json::array();
		for (const auto hierarchy : kHierarchyOrder) {
			payload["hierarchyOrder"].push_back(hierarchy);
		}

		json options = json::array();
		for (const auto& option : Builds::GetBuildOptionCatalog()) {
			const auto activeScore = DisciplineScore(option.discipline);
			const auto activeSlotId = FindActiveSlotForOption(option.id);
			options.push_back({
				{ "id", option.id },
				{ "discipline", DisciplineToJs(option.discipline) },
				{ "themeId", option.themeId },
				{ "themeTitleKey", option.themeTitleKey },
				{ "hierarchy", option.hierarchy },
				{ "layer", BuildLayerToJs(option.layer) },
				{ "unlockScore", option.unlockScore },
				{ "unlocked", activeScore >= option.unlockScore },
				{ "state", OptionStateToJs(activeScore >= option.unlockScore, activeSlotId.has_value()) },
				{ "slotCompatibility", SlotCompatibilityToJs(option.slotCompatibility) },
				{ "compatibleSlots", CompatibleSlotIdsJson(option) },
				{ "activeSlotId", activeSlotId.has_value() ? json(SlotIdToJs(activeSlotId.value())) : json(nullptr) },
				{ "effectType", EffectTypeToJs(option.effectType) },
				{ "effectKey", option.effectKey },
				{ "magnitude", MagnitudeToJson(Builds::GetScaledBuildMagnitude(option, activeScore)) },
				{ "baseMagnitude", MagnitudeToJson(option.magnitude) },
				{ "magnitudePerTier", MagnitudeToJson(option.magnitudePerTier) },
				{ "currentMagnitude", MagnitudeToJson(Builds::GetScaledBuildMagnitude(option, activeScore)) },
				{ "nextMagnitude", MagnitudeToJson(Builds::GetNextTierBuildMagnitude(option, activeScore)) },
				{ "currentTier", Builds::GetBuildScalingTier(activeScore) },
				{ "nextTierScore", Builds::GetNextBuildScalingScore(activeScore) },
				{ "scoreToNextTier", Builds::GetScoreToNextBuildScalingTier(activeScore) },
				{ "exclusivityGroup", option.exclusivityGroup },
				{ "titleKey", option.titleKey },
				{ "descriptionKey", option.descriptionKey },
			});
		}
		payload["options"] = std::move(options);

		json themeMap = {
			{ "attack", json::array() },
			{ "defense", json::array() },
			{ "utility", json::array() },
		};
		json groupedCatalog = json::object();

		for (const auto discipline : {
			     Builds::BuildDiscipline::Attack,
			     Builds::BuildDiscipline::Defense,
			     Builds::BuildDiscipline::Utility }) {
			const auto disciplineJs = DisciplineToJs(discipline);
			json groupedThemes = json::array();

			for (const auto& theme : ThemeCatalog(discipline)) {
				const auto rows = CollectThemeRows(discipline, theme.id);
				json rowPayload = json::array();
				for (const auto* option : rows) {
					rowPayload.push_back(BuildCatalogRowPayload(*option, DisciplineScore(discipline)));
				}

				themeMap[disciplineJs].push_back({
					{ "id", theme.id },
					{ "titleKey", theme.titleKey },
					{ "optionCount", static_cast<std::uint32_t>(rows.size()) },
				});

				groupedThemes.push_back({
					{ "id", theme.id },
					{ "titleKey", theme.titleKey },
					{ "optionCount", static_cast<std::uint32_t>(rows.size()) },
					{ "rows", std::move(rowPayload) },
				});
			}

			groupedCatalog[disciplineJs] = {
				{ "discipline", disciplineJs },
				{ "themes", std::move(groupedThemes) },
			};
		}

		payload["themeMap"] = std::move(themeMap);
		payload["groupedCatalog"] = std::move(groupedCatalog);

		constexpr auto defaultDiscipline = Builds::BuildDiscipline::Attack;
		const auto selectedThemeId = SelectDefaultThemeId(defaultDiscipline);
		const auto* selectedOption = SelectDefaultOption(defaultDiscipline, selectedThemeId);
		payload["selectedDiscipline"] = DisciplineToJs(defaultDiscipline);
		payload["selectedTheme"] = selectedThemeId;
		payload["selectedOptionId"] = selectedOption != nullptr ? json(selectedOption->id) : json("");

		json selectedThemeRows = json::array();
		for (const auto* option : CollectThemeRows(defaultDiscipline, selectedThemeId)) {
			selectedThemeRows.push_back(BuildCatalogRowPayload(*option, DisciplineScore(defaultDiscipline)));
		}
		payload["selectedThemeRows"] = std::move(selectedThemeRows);
		payload["selectedOptionDetail"] = selectedOption != nullptr
			? BuildCatalogRowPayload(*selectedOption, DisciplineScore(defaultDiscipline))
			: json(nullptr);

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
