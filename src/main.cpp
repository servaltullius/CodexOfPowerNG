#include "CodexOfPowerNG/Constants.h"
#include "CodexOfPowerNG/Config.h"
#include "CodexOfPowerNG/Events.h"
#include "CodexOfPowerNG/L10n.h"
#include "CodexOfPowerNG/PrismaUIManager.h"
#include "CodexOfPowerNG/Registration.h"
#include "CodexOfPowerNG/Serialization.h"

#include <RE/Skyrim.h>

#include <SKSE/Interfaces.h>
#include <SKSE/Logger.h>
#include <SKSE/SKSE.h>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include <string>

namespace CodexOfPowerNG
{
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

	CodexOfPowerNG::Serialization::Install();

	auto* messaging = SKSE::GetMessagingInterface();
	if (messaging) {
		messaging->RegisterListener(CodexOfPowerNG::OnSKSEMessage);
	} else {
		SKSE::log::error("Messaging interface unavailable");
	}

	return true;
}
