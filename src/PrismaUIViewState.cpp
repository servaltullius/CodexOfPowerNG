#include "PrismaUIViewState.h"

namespace CodexOfPowerNG::PrismaUIManager::Internal::State
{
	std::atomic<PRISMA_UI_API::IVPrismaUI1*> prismaAPI{ nullptr };
	std::atomic<PrismaView>                  view{ 0 };
	std::atomic_bool                         domReady{ false };
	std::atomic_bool                         openRequested{ false };
	std::atomic_bool                         viewHidden{ true };
	std::atomic_bool                         viewFocused{ false };
	std::atomic_bool                         focusDelayArmed{ false };
	std::atomic_int                          focusAttemptCount{ 0 };
	std::atomic_bool                         shuttingDown{ false };

	std::mutex  workerMutex;
	std::thread closeRetryThread;
	std::thread focusDelayThread;

	void JoinIfJoinable(std::thread& t) noexcept
	{
		if (t.joinable()) {
			t.join();
		}
	}
}
