#include "CodexOfPowerNG/Constants.h"
#include "CodexOfPowerNG/Config.h"
#include "CodexOfPowerNG/Events.h"
#include "CodexOfPowerNG/L10n.h"
#include "CodexOfPowerNG/PrismaUIManager.h"
#include "CodexOfPowerNG/Registration.h"
#include "CodexOfPowerNG/Rewards.h"
#include "CodexOfPowerNG/Serialization.h"
#include "CodexOfPowerNG/TaskScheduler.h"

#include <RE/Skyrim.h>

#include <SKSE/Interfaces.h>
#include <SKSE/Logger.h>
#include <SKSE/SKSE.h>

#include <nlohmann/json.hpp>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <exception>

namespace CodexOfPowerNG
{
	namespace
	{
		inline constexpr auto kLegacySVCollectionIniPath = "Data/MCM/Settings/SVCollection.ini";
		inline constexpr auto kLegacyMCMKeybindsPath = "Data/MCM/Settings/keybinds.json";

		bool g_hasLegacySVCollectionResidue{ false };
		bool g_legacyResidueNotified{ false };

		[[nodiscard]] bool FileExists(const std::filesystem::path& path)
		{
			std::error_code ec{};
			return std::filesystem::exists(path, ec) && !ec;
		}

		[[nodiscard]] bool HasLegacySVCollectionKeybind() noexcept
		{
			std::ifstream in(std::filesystem::path(kLegacyMCMKeybindsPath), std::ios::binary);
			if (!in.is_open()) {
				return false;
			}

			try {
				nlohmann::json json;
				in >> json;

				auto keybindsIt = json.find("keybinds");
				if (keybindsIt == json.end() || !keybindsIt->is_array()) {
					return false;
				}

				for (const auto& entry : *keybindsIt) {
					if (!entry.is_object()) {
						continue;
					}

					auto modNameIt = entry.find("modName");
					if (modNameIt != entry.end() && modNameIt->is_string() && modNameIt->get<std::string>() == "SVCollection") {
						return true;
					}
				}
			} catch (const std::exception& e) {
				SKSE::log::warn(
					"Failed to parse '{}': {}",
					kLegacyMCMKeybindsPath,
					e.what());
			}

			return false;
		}

		[[nodiscard]] bool DetectLegacySVCollectionResidue() noexcept
		{
			const bool hasIni = FileExists(std::filesystem::path(kLegacySVCollectionIniPath));
			const bool hasKeybind = HasLegacySVCollectionKeybind();

			if (!hasIni && !hasKeybind) {
				return false;
			}

			SKSE::log::warn("Detected legacy Codex of Power (SVCollection) residue.");
			if (hasIni) {
				SKSE::log::warn(" - Found '{}'", kLegacySVCollectionIniPath);
			}
			if (hasKeybind) {
				SKSE::log::warn(
					" - Found 'SVCollection' keybind registration in '{}'",
					kLegacyMCMKeybindsPath);
			}

			SKSE::log::warn(
				"Legacy SVCollection MCM files can interfere with NG. "
				"Remove stale files from your active Data path (MO2: overwrite).");
			return true;
		}

		void NotifyLegacyResidueIfNeeded() noexcept
		{
			if (!g_hasLegacySVCollectionResidue || g_legacyResidueNotified) {
				return;
			}
			g_legacyResidueNotified = true;

			constexpr auto message =
				"Codex NG: Old SVCollection MCM settings detected. "
				"Delete SVCollection.ini and SVCollection keybind entry.";

			if (!QueueUITask([message]() { RE::DebugNotification(message); })) {
				RE::DebugNotification(message);
			}
		}
	}

	void SetupLogging()
	{
		auto path = SKSE::log::log_directory();
		if (!path) {
			return;
		}

		*path /= std::string(kPluginName) + ".log";

		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
		auto logger = std::make_shared<spdlog::logger>("global log", std::move(sink));

		spdlog::set_default_logger(std::move(logger));
		spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
		spdlog::set_level(spdlog::level::info);
		spdlog::flush_on(spdlog::level::info);
	}

	class InputEventSink final : public RE::BSTEventSink<RE::InputEvent*>
	{
	public:
		bool toggleKeyDown{ false };

		RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_event,
			RE::BSTEventSource<RE::InputEvent*>* /*a_eventSource*/) override
		{
			if (!a_event || !*a_event) {
				return RE::BSEventNotifyControl::kContinue;
			}

			const auto settings = GetSettings();

			for (auto* event = *a_event; event; event = event->next) {
				auto* button = event->AsButtonEvent();
				if (!button) {
					continue;
				}

				if (button->GetDevice() != RE::INPUT_DEVICE::kKeyboard) {
					continue;
				}

				if (button->GetIDCode() != settings.toggleKeyCode) {
					continue;
				}

				if (button->IsUp()) {
					toggleKeyDown = false;
					continue;
				}

				if (button->IsDown()) {
					// Only fire once per physical press.
					if (!button->IsRepeating() && !toggleKeyDown) {
						toggleKeyDown = true;
						SKSE::log::info("Hotkey pressed (toggle UI)");
						PrismaUIManager::ToggleUI();
						break;
					}
				}
			}

			return RE::BSEventNotifyControl::kContinue;
		}
	};

	InputEventSink g_inputSink;

	void RegisterInputSink()
	{
		auto* inputMgr = RE::BSInputDeviceManager::GetSingleton();
		if (!inputMgr) {
			SKSE::log::warn("BSInputDeviceManager unavailable; cannot register hotkey");
			return;
		}

		inputMgr->AddEventSink(&g_inputSink);
		const auto settings = GetSettings();
		SKSE::log::info("Registered input sink (toggle key: 0x{:02X})", settings.toggleKeyCode);
	}

	void OnSKSEMessage(SKSE::MessagingInterface::Message* message)
	{
		if (!message) {
			return;
		}

		switch (message->type) {
		case SKSE::MessagingInterface::kPostLoad:
			PrismaUIManager::OnPostLoad();
			break;
		case SKSE::MessagingInterface::kInputLoaded:
			RegisterInputSink();
			break;
		case SKSE::MessagingInterface::kPreLoadGame:
			PrismaUIManager::OnPreLoadGame();
			break;
		case SKSE::MessagingInterface::kDataLoaded:
			Registration::Warmup();
			break;
		case SKSE::MessagingInterface::kPostLoadGame:
		case SKSE::MessagingInterface::kNewGame:
			// Avoid doing UI work at the main menu. Create the view lazily on hotkey.
			PrismaUIManager::OnGameLoaded();
			Events::Install();
			Events::OnGameLoaded();
			Rewards::SyncRewardTotalsToPlayer();
			Rewards::ScheduleCarryWeightQuickResync();
			NotifyLegacyResidueIfNeeded();
			break;
		default:
			break;
		}
	}
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
	CodexOfPowerNG::SetupLogging();
	SKSE::Init(skse);

	SKSE::log::info("{} loaded", CodexOfPowerNG::kPluginName);

	CodexOfPowerNG::LoadSettingsFromDisk();
	CodexOfPowerNG::L10n::Load();
	CodexOfPowerNG::g_hasLegacySVCollectionResidue = CodexOfPowerNG::DetectLegacySVCollectionResidue();

	CodexOfPowerNG::Serialization::Install();

	auto* messaging = SKSE::GetMessagingInterface();
	if (messaging) {
		messaging->RegisterListener(CodexOfPowerNG::OnSKSEMessage);
	} else {
		SKSE::log::error("Messaging interface unavailable");
	}

	return true;
}
