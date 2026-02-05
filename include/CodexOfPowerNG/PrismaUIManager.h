#pragma once

namespace CodexOfPowerNG::PrismaUIManager
{
	void OnPostLoad() noexcept;
	void OnDataLoaded() noexcept;
	void OnPreLoadGame() noexcept;
	void OnGameLoaded() noexcept;

	void ToggleUI() noexcept;
	void SendStateToUI() noexcept;
}
