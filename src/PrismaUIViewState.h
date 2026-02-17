#pragma once

#include "PrismaUIInternal.h"

#include <atomic>
#include <mutex>
#include <thread>

namespace CodexOfPowerNG::PrismaUIManager::Internal::State
{
	extern std::atomic<PRISMA_UI_API::IVPrismaUI1*> prismaAPI;
	extern std::atomic<PrismaView>                  view;
	extern std::atomic_bool                         domReady;
	extern std::atomic_bool                         openRequested;
	extern std::atomic_bool                         viewHidden;
	extern std::atomic_bool                         viewFocused;
	extern std::atomic_bool                         focusDelayArmed;
	extern std::atomic_int                          focusAttemptCount;
	extern std::atomic_bool                         shuttingDown;

	extern std::mutex  workerMutex;
	extern std::thread closeRetryThread;
	extern std::thread focusDelayThread;

	void JoinIfJoinable(std::thread& t) noexcept;
}
