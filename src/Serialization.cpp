#include "CodexOfPowerNG/Serialization.h"

#include "CodexOfPowerNG/Constants.h"
#include "SerializationInternal.h"

#include <RE/Skyrim.h>

#include <SKSE/Interfaces.h>
#include <SKSE/Logger.h>
#include <SKSE/SKSE.h>

namespace CodexOfPowerNG::Serialization
{
	void Install() noexcept
	{
		auto* serialization = SKSE::GetSerializationInterface();
		if (!serialization) {
			SKSE::log::error("Serialization interface unavailable");
			return;
		}

		serialization->SetUniqueID(kSerializationUniqueId);
		serialization->SetRevertCallback(Internal::Revert);
		serialization->SetSaveCallback(Internal::Save);
		serialization->SetLoadCallback(Internal::Load);
	}
}
