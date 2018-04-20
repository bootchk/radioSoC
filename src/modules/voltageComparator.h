
#pragma once





/*
 * Compares voltage from an analog input pin to a reference voltage.
 */

enum class VCompareResult {
	AboveRef,
	BelowRef
};



class VoltageComparator {
public:
	/*
	 * Init, read, and shutdown.
	 *
	 * Blocks for typically 50uSec
	 */
	static VCompareResult compareToLowThreshold();
	static VCompareResult compareToHighThreshold();
};
