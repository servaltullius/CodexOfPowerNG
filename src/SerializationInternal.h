#pragma once

namespace SKSE
{
	class SerializationInterface;
}

namespace CodexOfPowerNG::Serialization::Internal
{
	void Revert(SKSE::SerializationInterface* a_intfc) noexcept;
	void Save(SKSE::SerializationInterface* a_intfc) noexcept;
	void Load(SKSE::SerializationInterface* a_intfc) noexcept;
}
