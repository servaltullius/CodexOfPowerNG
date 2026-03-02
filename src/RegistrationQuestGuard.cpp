#include "CodexOfPowerNG/RegistrationQuestGuard.h"

#include "CodexOfPowerNG/Util.h"

#include <RE/F/FormTraits.h>
#include <SKSE/Logger.h>

#include <mutex>

// BGSRefAlias.cpp.obj in CommonLibSSE.lib references TESForm::As<Actor>
// via GetActorReference(). Provide the instantiation so the linker can resolve it.
template RE::Actor* RE::TESForm::As<RE::Actor, void>() noexcept;

namespace CodexOfPowerNG::Registration::QuestGuard
{
	namespace
	{
		inline constexpr std::uint64_t kCacheTtlMs = 2000;

		std::mutex                          g_mutex;
		std::unordered_set<RE::FormID>      g_protectedForms;
		std::uint64_t                       g_builtAtMs{ 0 };

		void RebuildIfStale()
		{
			const auto now = NowMs();
			if (now >= g_builtAtMs && (now - g_builtAtMs) <= kCacheTtlMs) {
				return;
			}

			std::unordered_set<RE::FormID> fresh;

			auto* handler = RE::TESDataHandler::GetSingleton();
			if (!handler) {
				g_protectedForms.clear();
				g_builtAtMs = now;
				return;
			}

			for (auto* quest : handler->GetFormArray<RE::TESQuest>()) {
				if (!quest || !quest->IsRunning()) {
					continue;
				}

				for (auto* alias : quest->aliases) {
					if (!alias) {
						continue;
					}

					if (alias->GetVMTypeID() != RE::BGSRefAlias::VMTYPEID) {
						continue;
					}
					auto* refAlias = static_cast<RE::BGSRefAlias*>(alias);

					// Check the live reference's base object
					auto* ref = refAlias->GetReference();
					if (ref) {
						auto* base = ref->GetBaseObject();
						if (base) {
							fresh.insert(base->GetFormID());
						}
					}

					// Also check created-object aliases (fillType == kCreated)
					if (refAlias->fillType == RE::BGSBaseAlias::FILL_TYPE::kCreated) {
						auto* createdObj = refAlias->fillData.created.object;
						if (createdObj) {
							fresh.insert(createdObj->GetFormID());
						}
					}
				}
			}

			g_protectedForms = std::move(fresh);
			g_builtAtMs = now;
		}
	}

	std::unordered_set<RE::FormID> SnapshotProtectedForms()
	{
		std::scoped_lock lock(g_mutex);
		RebuildIfStale();
		return g_protectedForms;
	}

	bool IsQuestProtected(RE::FormID formId) noexcept
	{
		std::scoped_lock lock(g_mutex);
		RebuildIfStale();
		return g_protectedForms.contains(formId);
	}
}
