#include "CodexOfPowerNG/RegistrationTccGate.h"

#include <cassert>

int main()
{
	using CodexOfPowerNG::Registration::DecideTccGate;
	using CodexOfPowerNG::Registration::TccGateDecision;

	// Gate disabled: always allow.
	assert(DecideTccGate(false, false, false, true, false) == TccGateDecision::kAllow);
	assert(DecideTccGate(false, true, true, true, false) == TccGateDecision::kAllow);

	// Gate enabled but TCC lists missing: fail-closed.
	assert(DecideTccGate(true, false, true, true, true) == TccGateDecision::kBlockUnavailable);
	assert(DecideTccGate(true, true, false, true, true) == TccGateDecision::kBlockUnavailable);
	assert(DecideTccGate(true, false, false, false, false) == TccGateDecision::kBlockUnavailable);

	// Gate enabled, lists present, non-LOTD item: allow.
	assert(DecideTccGate(true, true, true, false, false) == TccGateDecision::kAllow);

	// Gate enabled, LOTD tracked item: displayed state decides.
	assert(DecideTccGate(true, true, true, true, false) == TccGateDecision::kBlockNotDisplayed);
	assert(DecideTccGate(true, true, true, true, true) == TccGateDecision::kAllow);

	return 0;
}
