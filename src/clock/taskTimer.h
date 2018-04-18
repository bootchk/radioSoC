
#pragma once

#include "../platformTypes.h"   // OSTime

typedef void (*Task)(void);

typedef unsigned int TaskTimerIndex;




/*
 * Schedules Task for future.
 * Uses Counter/CompareRegister.
 *
 * Derives from old implementation Timer (for design using Sleepers)
 */
class TaskTimer {
private:
	static void configureCompareRegisterForTimer(TaskTimerIndex index, OSTime timeout);
	static void handleExpiration();

public:
	static void schedule(Task task, OSTime duration);

	/*
	 * Called from the IRQ handler.
	 * Many events may have occurred (clock overflow, and many compare register matches)
	 */
	static void timerISR();
};
