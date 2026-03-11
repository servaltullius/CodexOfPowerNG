#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
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
	inline constexpr std::size_t kBuildSlotCount = static_cast<std::size_t>(BuildSlotId::Wildcard1) + 1;

	[[nodiscard]] constexpr std::size_t ToIndex(BuildSlotId slotId) noexcept
	{
		return static_cast<std::size_t>(slotId);
	}

	struct BuildOptionDef
	{
		std::string_view id;
		BuildDiscipline discipline;
		BuildLayer layer;
		std::uint32_t unlockScore;
		BuildSlotCompatibility slotCompatibility;
		BuildEffectType effectType;
		std::string_view effectKey;
		BuildMagnitude magnitude;
		std::string_view exclusivityGroup;
		BuildStackRule stackRule;
		std::string_view titleKey;
		std::string_view descriptionKey;
	};

	struct BuildBaselineMilestoneDef
	{
		BuildDiscipline discipline;
		std::uint32_t threshold;
		BuildEffectType effectType;
		std::string_view effectKey;
		BuildMagnitude magnitude;
	};
}
