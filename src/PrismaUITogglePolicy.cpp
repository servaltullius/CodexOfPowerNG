#include "PrismaUIInternal.h"

#include <RE/Skyrim.h>

#include <SKSE/Logger.h>

#include <atomic>
#include <cstdint>

namespace CodexOfPowerNG::PrismaUIManager::Internal
{
	namespace
	{
		std::atomic_bool g_toggleAllowed{ false };
		std::atomic<std::uint64_t> g_toggleAllowedAtMs{ 0 };
		std::atomic<std::uint64_t> g_lastToggleMs{ 0 };
	}

	void ResetTogglePolicyForPreLoad() noexcept
	{
		g_toggleAllowed.store(false, std::memory_order_relaxed);
		g_toggleAllowedAtMs.store(0, std::memory_order_relaxed);
		g_lastToggleMs.store(0, std::memory_order_relaxed);
	}

	void ArmTogglePolicyAfterGameLoaded(std::uint64_t nowMs, std::uint64_t graceMs) noexcept
	{
		g_toggleAllowed.store(true, std::memory_order_relaxed);
		g_toggleAllowedAtMs.store(nowMs + graceMs, std::memory_order_relaxed);
	}

	bool CanToggleNow(std::uint64_t nowMs, std::uint64_t debounceMs) noexcept
	{
		if (!g_toggleAllowed.load(std::memory_order_relaxed)) {
			SKSE::log::info("ToggleUI: ignored (not allowed yet)");
			return false;
		}

		const auto allowAt = g_toggleAllowedAtMs.load(std::memory_order_relaxed);
		if (allowAt > 0 && nowMs < allowAt) {
			SKSE::log::info("ToggleUI: ignored (post-load grace period)");
			return false;
		}

		if (auto* ui = RE::UI::GetSingleton(); ui) {
			if (ui->IsMenuOpen(RE::MainMenu::MENU_NAME) || ui->IsMenuOpen(RE::LoadingMenu::MENU_NAME)) {
				SKSE::log::info("ToggleUI: ignored (application menu open)");
				return false;
			}
		}

		if (auto* main = RE::Main::GetSingleton(); !main || !main->gameActive) {
			SKSE::log::info("ToggleUI: ignored (game not active)");
			return false;
		}

		const auto last = g_lastToggleMs.load(std::memory_order_relaxed);
		if (last > 0 && nowMs < last + debounceMs) {
			SKSE::log::info("ToggleUI: debounced");
			return false;
		}

		g_lastToggleMs.store(nowMs, std::memory_order_relaxed);
		return true;
	}
}
