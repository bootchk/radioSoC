
#pragma once





/*
 * Compares voltage from an analog input pin to a reference voltage.
 */

// !!! Order is important so that we don't flip the raw values:  0 means below whether from device Comparator or VoltageComparator
enum class VCompareResult {
	BelowRef,
	AboveRef
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
