#include "CodexOfPowerNG/Events.h"

#include "CodexOfPowerNG/Config.h"
#include "CodexOfPowerNG/L10n.h"
#include "CodexOfPowerNG/Registration.h"
#include "CodexOfPowerNG/State.h"

#include <RE/Skyrim.h>

#include <RE/M/Misc.h>

#include <SKSE/Logger.h>
#include <SKSE/SKSE.h>

#include <atomic>
#include <algorithm>
#include <chrono>
#include <cctype>
#include <mutex>
#include <random>
#include <string>

namespace CodexOfPowerNG::Events
{
	namespace
	{
		[[nodiscard]] std::uint64_t NowMs() noexcept
		{
			using namespace std::chrono;
			return static_cast<std::uint64_t>(
				duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
		}

		[[nodiscard]] int RandomInt(int minInclusive, int maxInclusive) noexcept
		{
			static thread_local std::mt19937 rng{ std::random_device{}() };
			std::uniform_int_distribution<int> dist(minInclusive, maxInclusive);
			return dist(rng);
		}

		std::atomic_bool g_gameReady{ false };
		std::atomic<std::uint64_t> g_ignoreUntilMs{ 0 };
		constexpr std::uint64_t kDebounceMs = 5000;
		std::atomic<std::uint64_t> g_lastNotifyMs{ 0 };
		constexpr std::uint64_t kNotifyThrottleMs = 750;
		std::atomic<std::uint64_t> g_lastCorpseProcMs{ 0 };

		[[nodiscard]] std::string_view Trim(std::string_view s) noexcept
		{
			while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) {
				s.remove_prefix(1);
			}
			while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) {
				s.remove_suffix(1);
			}
			return s;
		}

		[[nodiscard]] bool ContainsI(std::string_view hay, std::string_view needle) noexcept
		{
			if (needle.empty()) {
				return true;
			}
			if (hay.size() < needle.size()) {
				return false;
			}

			for (std::size_t i = 0; i + needle.size() <= hay.size(); ++i) {
				bool match = true;
				for (std::size_t j = 0; j < needle.size(); ++j) {
					const unsigned char a = static_cast<unsigned char>(hay[i + j]);
					const unsigned char b = static_cast<unsigned char>(needle[j]);
					if (std::tolower(a) != std::tolower(b)) {
						match = false;
						break;
					}
				}
				if (match) {
					return true;
				}
			}
			return false;
		}

		struct CorpseVfxSelection
		{
			RE::BGSArtObject*    art{ nullptr };
			RE::TESEffectShader* shader{ nullptr };
			std::string          pickedEditorID{};
		};

		std::mutex            g_corpseVfxMutex;
		bool                  g_autoVfxSearched{ false };
		CorpseVfxSelection    g_autoVfx{};
		std::string           g_overrideKey{};
		CorpseVfxSelection    g_overrideVfx{};

		[[nodiscard]] int ScoreCorpseVfxCandidate(std::string_view editorID, std::string_view model) noexcept
		{
			int score = 0;
			// Corpse-explosion themed (prefer blood/gore if available).
			if (ContainsI(editorID, "Blood")) score += 120;
			if (ContainsI(model, "blood")) score += 120;
			if (ContainsI(editorID, "Gore")) score += 90;
			if (ContainsI(model, "gore")) score += 90;
			if (ContainsI(editorID, "Splatter")) score += 70;
			if (ContainsI(editorID, "Spray")) score += 50;
			// Strong positives
			if (ContainsI(editorID, "Explosion")) score += 80;
			if (ContainsI(model, "explosion")) score += 80;
			if (ContainsI(editorID, "Fireball")) score += 50;
			if (ContainsI(editorID, "Blast")) score += 35;
			if (ContainsI(editorID, "Burst")) score += 35;
			if (ContainsI(editorID, "Impact")) score += 20;
			if (ContainsI(editorID, "Hit")) score += 10;
			// Mild positives
			if (ContainsI(editorID, "Fire")) score += 10;
			if (ContainsI(editorID, "Shock")) score += 10;
			if (ContainsI(editorID, "Frost")) score += 10;
			// Negatives: avoid traps/mines/empty placeholders
			if (ContainsI(editorID, "Trap")) score -= 60;
			if (ContainsI(editorID, "Mine")) score -= 60;
			if (ContainsI(editorID, "Empty")) score -= 80;
			return score;
		}

		[[nodiscard]] CorpseVfxSelection FindAutoCorpseVfx() noexcept
		{
			CorpseVfxSelection best{};
			int bestScore = -999999;

			const auto& [map, lock] = RE::TESForm::GetAllFormsByEditorID();
			[[maybe_unused]] const RE::BSReadWriteLock l{ lock };
			if (!map) {
				return best;
			}

			// Prefer ARTO (hit art) for safety; fall back to EFSH.
			for (const auto& kv : *map) {
				const auto* edidC = kv.first.c_str();
				if (!edidC || edidC[0] == '\0') {
					continue;
				}
				auto* form = kv.second;
				if (!form) {
					continue;
				}

				if (auto* art = form->As<RE::BGSArtObject>()) {
					const auto editorID = std::string_view(edidC);
					const auto modelC = art->GetModel();
					const auto model = std::string_view(modelC ? modelC : "");
					const int score = ScoreCorpseVfxCandidate(editorID, model);
					if (score > bestScore) {
						bestScore = score;
						best.art = art;
						best.shader = nullptr;
						best.pickedEditorID = std::string(editorID);
					}
				}
			}

			// If ARTO search found nothing useful, attempt EFSH by editorID only.
			if (!best.art || bestScore < 10) {
				for (const auto& kv : *map) {
					const auto* edidC = kv.first.c_str();
					if (!edidC || edidC[0] == '\0') {
						continue;
					}
					auto* form = kv.second;
					if (!form) {
						continue;
					}

					if (auto* shader = form->As<RE::TESEffectShader>()) {
						const auto editorID = std::string_view(edidC);
						const int score = ScoreCorpseVfxCandidate(editorID, "");
						if (score > bestScore) {
							bestScore = score;
							best.shader = shader;
							best.art = nullptr;
							best.pickedEditorID = std::string(editorID);
						}
					}
				}
			}

			// If we couldn't find a convincing match, fall back to a known-safe default.
			if (bestScore < 20) {
				best = {};
			}

			// Hard fallback: default absorb art object exists in all runtimes.
			if (!best.art && !best.shader) {
				if (auto* dom = RE::BGSDefaultObjectManager::GetSingleton(); dom) {
					if (auto* fallback = dom->GetObject<RE::BGSArtObject>(RE::BGSDefaultObjectManager::DefaultObject::kArtObjectAbsorbEffect)) {
						best.art = fallback;
						best.pickedEditorID = "DEFAULT:kArtObjectAbsorbEffect";
					}
				}
			}

			return best;
		}

		[[nodiscard]] CorpseVfxSelection ResolveCorpseVfx(const Settings& settings) noexcept
		{
			std::scoped_lock lock(g_corpseVfxMutex);

			const auto overrideRaw = Trim(settings.corpseExplosionVfxEditorID);
			const auto overrideKey = std::string(overrideRaw);

			if (!overrideKey.empty()) {
				if (overrideKey != g_overrideKey) {
					g_overrideKey = overrideKey;
					g_overrideVfx = {};

					if (auto* form = RE::TESForm::LookupByEditorID(overrideKey)) {
						if (auto* art = form->As<RE::BGSArtObject>()) {
							g_overrideVfx.art = art;
							g_overrideVfx.pickedEditorID = overrideKey;
						} else if (auto* shader = form->As<RE::TESEffectShader>()) {
							g_overrideVfx.shader = shader;
							g_overrideVfx.pickedEditorID = overrideKey;
						}
					}
				}
				return g_overrideVfx;
			}

			if (!g_autoVfxSearched) {
				g_autoVfxSearched = true;
				g_autoVfx = FindAutoCorpseVfx();
				SKSE::log::info("Corpse explosion VFX auto-selected: {}", g_autoVfx.pickedEditorID.empty() ? "(none)" : g_autoVfx.pickedEditorID);
			}

			return g_autoVfx;
		}

		void QueueCorpseExplosionProc(const RE::ObjectRefHandle& dyingHandle) noexcept
		{
			const auto handle = dyingHandle;
			if (!handle) {
				return;
			}

			if (auto* tasks = SKSE::GetTaskInterface(); tasks) {
				tasks->AddTask([handle]() {
					const auto settings = GetSettings();
					if (!settings.corpseExplosionEnabled) {
						return;
					}

					const auto mode = settings.corpseExplosionVfxMode;
					const bool wantArt = (mode == "auto" || mode == "art");
					const bool wantShader = (mode == "auto" || mode == "shader");
					const bool wantNone = (mode == "none");

					const auto ref = handle.get();
					if (!ref) {
						return;
					}

					auto* dying = ref.get();
					if (!dying) {
						return;
					}

					if (!wantNone) {
						const auto pick = ResolveCorpseVfx(settings);
						constexpr float kDurSeconds = 1.25f;

						bool played = false;
						if (wantArt && pick.art) {
							played = dying->ApplyArtObject(pick.art, kDurSeconds, nullptr, false, false, nullptr, false) != nullptr;
						}
						if (!played && wantShader && pick.shader) {
							played = dying->ApplyEffectShader(pick.shader, kDurSeconds, nullptr, false, false, nullptr, false) != nullptr;
						}
					}

					if (settings.corpseExplosionNotify) {
						const auto msg = L10n::T("msg.corpseExploded", "Corpse explosion!");
						RE::DebugNotification(msg.c_str());
					}
				});
			}
		}

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

				{
					auto& state = GetState();
					std::scoped_lock lock(state.mutex);
					if (state.notifiedItems.contains(regKeyId) || state.notifiedItems.contains(baseId)) {
						return RE::BSEventNotifyControl::kContinue;
					}

					state.notifiedItems.insert(regKeyId);
					state.notifiedItems.insert(baseId);
				}

				const auto lastNotify = g_lastNotifyMs.load(std::memory_order_relaxed);
				if (lastNotify > 0 && nowMs < lastNotify + kNotifyThrottleMs) {
					return RE::BSEventNotifyControl::kContinue;
				}
				g_lastNotifyMs.store(nowMs, std::memory_order_relaxed);

				auto* regForm = RE::TESForm::LookupByID(regKeyId);
				const auto name = regForm && regForm->GetName() ? regForm->GetName() : "";

				const auto msg =
					L10n::T("msg.lootUnregisteredPrefix", "Unregistered: ") +
					std::string(name && name[0] != '\0' ? name : L10n::T("ui.unnamed", "(unnamed)")) +
					L10n::T("msg.lootUnregisteredSuffix", " (press hotkey to register)");

				if (auto* tasks = SKSE::GetTaskInterface(); tasks) {
					tasks->AddUITask([msg]() { RE::DebugNotification(msg.c_str()); });
				} else {
					RE::DebugNotification(msg.c_str());
				}
				return RE::BSEventNotifyControl::kContinue;
				}
			};

		class DeathSink final : public RE::BSTEventSink<RE::TESDeathEvent>
		{
		public:
			RE::BSEventNotifyControl ProcessEvent(const RE::TESDeathEvent* event,
				RE::BSTEventSource<RE::TESDeathEvent>* /*source*/) override
			{
				if (!event || !event->dead) {
					return RE::BSEventNotifyControl::kContinue;
				}

				const auto settings = GetSettings();
				if (!settings.corpseExplosionEnabled) {
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

				auto* player = RE::PlayerCharacter::GetSingleton();
				if (!player) {
					return RE::BSEventNotifyControl::kContinue;
				}

				auto* killer = event->actorKiller.get();
				if (!killer || killer->GetFormID() != player->GetFormID()) {
					return RE::BSEventNotifyControl::kContinue;
				}

				const auto cooldownMs = settings.corpseExplosionCooldownMs > 0 ? static_cast<std::uint64_t>(settings.corpseExplosionCooldownMs) : 0;
				const auto last = g_lastCorpseProcMs.load(std::memory_order_relaxed);
				if (cooldownMs > 0 && last > 0 && nowMs < (last + cooldownMs)) {
					return RE::BSEventNotifyControl::kContinue;
				}

				const auto chancePct = settings.corpseExplosionChancePct;
				if (!(chancePct > 0.0)) {
					return RE::BSEventNotifyControl::kContinue;
				}
				const int threshold = std::clamp(static_cast<int>(chancePct * 100.0 + 0.5), 0, 10000);
				const int roll = RandomInt(1, 10000);
				if (roll > threshold) {
					return RE::BSEventNotifyControl::kContinue;
				}

				g_lastCorpseProcMs.store(nowMs, std::memory_order_relaxed);

				auto* dying = event->actorDying.get();
				if (!dying) {
					return RE::BSEventNotifyControl::kContinue;
				}

				QueueCorpseExplosionProc(dying->GetHandle());
				return RE::BSEventNotifyControl::kContinue;
			}
		};

			ContainerChangedSink g_containerChangedSink;
		DeathSink            g_deathSink;
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
		sources->AddEventSink<RE::TESDeathEvent>(&g_deathSink);
		SKSE::log::info("Registered TESDeathEvent sink");
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
