#include "CodexOfPowerNG/RegistrationFormId.h"

#include <cassert>
#include <cstdint>

int main()
{
	using CodexOfPowerNG::Registration::FormId::ParseFormIDText;
	using CodexOfPowerNG::Registration::FormId::ToLocalFormID;

	// Parse boundaries
	assert(!ParseFormIDText("").has_value());
	assert(!ParseFormIDText("0x").has_value());
	assert(!ParseFormIDText("12x").has_value());
	assert(!ParseFormIDText("0x100000000").has_value());  // uint32 overflow

	assert(ParseFormIDText("123").value() == 123u);
	assert(ParseFormIDText("0x1A").value() == 0x1Au);
	assert(ParseFormIDText("0Xfe000abc").value() == 0xFE000ABCu);

	// Local-form conversion boundaries
	assert(ToLocalFormID(0u) == 0u);
	assert(ToLocalFormID(0x01000ABCu) == 0x00000ABCu);
	assert(ToLocalFormID(0x12345678u) == 0x00345678u);
	assert(ToLocalFormID(0xFE000ABCu) == 0x00000ABCu);
	assert(ToLocalFormID(0xFEFFF123u) == 0x00000123u);

	return 0;
}
