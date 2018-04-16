
#pragma once

class MCUSleep {
public:
	static void untilAnyEvent();
	static void untilInterrupt();
	static void untilInterruptSDCompatible();
};
