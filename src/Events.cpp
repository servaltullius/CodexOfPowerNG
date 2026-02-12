#include "CodexOfPowerNG/Events.h"

#include "CodexOfPowerNG/Config.h"
#include "CodexOfPowerNG/EventsNotifyGate.h"
#include "CodexOfPowerNG/L10n.h"
#include "CodexOfPowerNG/Registration.h"
#include "CodexOfPowerNG/State.h"
#include "CodexOfPowerNG/TaskScheduler.h"
#include "CodexOfPowerNG/Util.h"

#include <RE/Skyrim.h>

#include <RE/M/Misc.h>

#include <SKSE/Logger.h>
#include <SKSE/SKSE.h>

#include <atomic>
#include <chrono>
#include <string>

namespace CodexOfPowerNG::Events
{
	namespace
	{
		std::atomic_bool g_gameReady{ false };
		std::atomic<std::uint64_t> g_ignoreUntilMs{ 0 };
		constexpr std::uint64_t kDebounceMs = 5000;
		std::atomic<std::uint64_t> g_lastNotifyMs{ 0 };
		constexpr std::uint64_t kNotifyThrottleMs = 750;

		class ContainerChangedSink final : public RE::BSTEventSink<RE::TESContainerChangedEvent>
		{
		public:
			RE::BSEventNotifyControl ProcessEvent(const RE::TESContainerChangedEvent* event,
				RE::BSTEventSource<RE::TESContainerChangedEvent>* /*source*/) override
			{
				if (!event) {
					return RE::BSEventNotifyControl::kContinue;
				}

				const auto settings = GetSettings();
				if (!settings.enableLootNotify) {
					return RE::BSEventNotifyControl::kContinue;
				}

				if (!g_gameReady.load(std::memory_order_relaxed)) {
					return RE::BSEventNotifyControl::kContinue;
				}

				const auto nowMs = NowMs();

				auto* main = RE::Main::GetSingleton();
				if (!main || !main->gameActive) {
					g_ignoreUntilMs.store(nowMs + kDebounceMs, std::memory_order_relaxed);
					return RE::BSEventNotifyControl::kContinue;
				}

				const auto ignoreUntil = g_ignoreUntilMs.load(std::memory_order_relaxed);
				if (ignoreUntil > 0 && nowMs < ignoreUntil) {
					return RE::BSEventNotifyControl::kContinue;
				}

				// Positive count moved into new container.
				if (event->itemCount <= 0) {
					return RE::BSEventNotifyControl::kContinue;
				}

				auto* player = RE::PlayerCharacter::GetSingleton();
				if (!player) {
					return RE::BSEventNotifyControl::kContinue;
				}

				if (event->newContainer != player->GetFormID()) {
					return RE::BSEventNotifyControl::kContinue;
				}

				const auto baseId = event->baseObj;

				if (!Registration::IsDiscoverable(baseId)) {
					return RE::BSEventNotifyControl::kContinue;
				}

				if (Registration::IsRegistered(baseId)) {
					return RE::BSEventNotifyControl::kContinue;
				}

				const auto regKeyId = Registration::GetRegisterKeyId(baseId);
				if (!regKeyId) {
					return RE::BSEventNotifyControl::kContinue;
				}

				bool alreadyNotified = false;
				{
					auto& state = GetState();
					std::scoped_lock lock(state.mutex);
					alreadyNotified =
						state.notifiedItems.contains(regKeyId) ||
						state.notifiedItems.contains(baseId);
				}

				const auto lastNotify = g_lastNotifyMs.load(std::memory_order_relaxed);
				const auto decision =
					DecideLootNotify(alreadyNotified, nowMs, lastNotify, kNotifyThrottleMs);
				if (decision != LootNotifyDecision::kNotify) {
					return RE::BSEventNotifyControl::kContinue;
				}

				{
					auto& state = GetState();
					std::scoped_lock lock(state.mutex);
					if (state.notifiedItems.contains(regKeyId) || state.notifiedItems.contains(baseId)) {
						return RE::BSEventNotifyControl::kContinue;
					}
					state.notifiedItems.insert(regKeyId);
					state.notifiedItems.insert(baseId);
				}
				g_lastNotifyMs.store(nowMs, std::memory_order_relaxed);

				auto* regForm = RE::TESForm::LookupByID(regKeyId);
				const auto name = regForm && regForm->GetName() ? regForm->GetName() : "";

				const auto msg =
					L10n::T("msg.lootUnregisteredPrefix", "Unregistered: ") +
					std::string(name && name[0] != '\0' ? name : L10n::T("ui.unnamed", "(unnamed)")) +
					L10n::T("msg.lootUnregisteredSuffix", " (press hotkey to register)");

				if (!QueueUITask([msg]() { RE::DebugNotification(msg.c_str()); })) {
					RE::DebugNotification(msg.c_str());
				}
				return RE::BSEventNotifyControl::kContinue;
			}
		};

		ContainerChangedSink g_containerChangedSink;
		bool g_installed{ false };
	}

	void Install() noexcept
	{
		if (g_installed) {
			return;
		}

		auto* sources = RE::ScriptEventSourceHolder::GetSingleton();
		if (!sources) {
			SKSE::log::error("ScriptEventSourceHolder unavailable; cannot register event sinks");
			return;
		}

		sources->AddEventSink<RE::TESContainerChangedEvent>(&g_containerChangedSink);
		SKSE::log::info("Registered TESContainerChangedEvent sink");
		g_installed = true;
	}

	void OnGameLoaded() noexcept
	{
		// Save-load/new-game can cause a storm of container changed events (inventory restore).
		// Skip the first few seconds to avoid heavy work (and potential re-entrancy/deadlocks) while the game is settling.
		g_gameReady.store(true, std::memory_order_relaxed);
		g_ignoreUntilMs.store(NowMs() + kDebounceMs, std::memory_order_relaxed);
	}
}
