#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <utility>
#include <variant>

namespace CodexOfPowerNG::Builds
{
	enum class BuildDiscipline : std::uint8_t
	{
		Attack,
		Defense,
		Utility,
	};

	enum class BuildSlotKind : std::uint8_t
	{
		Attack,
		Defense,
		Utility,
		Wildcard,
	};

	enum class BuildLayer : std::uint8_t
	{
		Baseline,
		Slotted,
	};

	enum class BuildSlotId : std::uint8_t
	{
		Attack1,
		Attack2,
		Defense1,
		Utility1,
		Utility2,
		Wildcard1,
	};

	enum class BuildSlotCompatibility : std::uint8_t
	{
		SameDisciplineOnly,
		WildcardOnly,
		SameOrWildcard,
	};

	enum class BuildEffectType : std::uint8_t
	{
		ActorValue,
		CarryWeight,
		Economy,
		UtilityFlag,
	};

	enum class BuildStackRule : std::uint8_t
	{
		OnceOnly,
	};

	enum class BuildMigrationState : std::uint8_t
	{
		kNotStarted,
		kPendingCleanup,
		kComplete,
	};

	struct BuildMigrationNoticeSnapshot
	{
		bool          needsNotice{ false };
		bool          legacyRewardsMigrated{ false };
		std::uint32_t unresolvedHistoricalRegistrations{ 0 };
	};

	using BuildMagnitude = std::variant<float, std::int32_t>;
	using BuildPointCenti = std::uint32_t;
	inline constexpr std::size_t kBuildSlotCount = static_cast<std::size_t>(BuildSlotId::Wildcard1) + 1;
	inline constexpr BuildPointCenti kBuildPointScale = 100u;
	inline constexpr BuildPointCenti kBuildPointsPerTierCenti = 800u;

	[[nodiscard]] constexpr BuildPointCenti ToBuildPointCenti(std::uint32_t wholePoints) noexcept
	{
		return wholePoints * kBuildPointScale;
	}

	[[nodiscard]] constexpr float FromBuildPointCenti(BuildPointCenti points) noexcept
	{
		return static_cast<float>(points) / static_cast<float>(kBuildPointScale);
	}

	[[nodiscard]] constexpr BuildPointCenti NormalizeLegacyUnlockToBuildPointsCenti(std::uint32_t value) noexcept
	{
		return value < kBuildPointScale ? ((value / 5u) * ToBuildPointCenti(4u)) : value;
	}

	[[nodiscard]] constexpr std::size_t ToIndex(BuildSlotId slotId) noexcept
	{
		return static_cast<std::size_t>(slotId);
	}

	struct BuildOptionDef
	{
		constexpr BuildOptionDef(
			std::string_view id,
			BuildDiscipline discipline,
			std::string_view themeId,
			std::string_view themeTitleKey,
			std::string_view hierarchy,
			BuildLayer layer,
			std::uint32_t unlockValue,
			BuildSlotCompatibility slotCompatibility,
			BuildEffectType effectType,
			std::string_view effectKey,
			BuildMagnitude magnitude,
			BuildMagnitude magnitudePerTier,
			std::string_view exclusivityGroup,
			BuildStackRule stackRule,
			std::string_view titleKey,
			std::string_view descriptionKey) noexcept :
			id(id),
			discipline(discipline),
			themeId(themeId),
			themeTitleKey(themeTitleKey),
			hierarchy(hierarchy),
			layer(layer),
			unlockPointsCenti(NormalizeLegacyUnlockToBuildPointsCenti(unlockValue)),
			slotCompatibility(slotCompatibility),
			effectType(effectType),
			effectKey(effectKey),
			magnitude(std::move(magnitude)),
			magnitudePerTier(std::move(magnitudePerTier)),
			exclusivityGroup(exclusivityGroup),
			stackRule(stackRule),
			titleKey(titleKey),
			descriptionKey(descriptionKey)
		{}

		std::string_view id;
		BuildDiscipline discipline;
		std::string_view themeId;
		std::string_view themeTitleKey;
		std::string_view hierarchy;
		BuildLayer layer;
		BuildPointCenti unlockPointsCenti;
		BuildSlotCompatibility slotCompatibility;
		BuildEffectType effectType;
		std::string_view effectKey;
		BuildMagnitude magnitude;
		BuildMagnitude magnitudePerTier;
		std::string_view exclusivityGroup;
		BuildStackRule stackRule;
		std::string_view titleKey;
		std::string_view descriptionKey;
	};

	struct BuildBaselineMilestoneDef
	{
		BuildDiscipline discipline;
		BuildPointCenti threshold;
		BuildEffectType effectType;
		std::string_view effectKey;
		BuildMagnitude magnitude;
	};
}
