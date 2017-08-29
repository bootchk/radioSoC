#pragma once


// Index of words in UICR
typedef unsigned int FlagIndex;


/*
 * Custom non-volatile data.
 * This knows how to write data to flash (persists, non-volatile.)
 *
 * Data is erasable in a batch using a program such as nfjprog.
 * Erased data is all FFFF.
 * Writing can only clear bits.
 * !!! Thus these functions only work once (until you erase flash again.)
 *
 * In Nordic UICR
 *
 * UICR is distinctive flash: eraseable separately from other flash.
 * Alternative is other flash memory.
 * Note retained registers are retained through SYSTEM_OFF, but not through reset.
 */

/*
 * UICR flags that this library's exception routines write
 */
enum FaultIndex {
	// First three are Program Counters of exceptions or Line Number, not flags
	HardFaultPCIndex = 0,			// hw fault PC
	ExitFlagIndex,					// other unexpected handler called
	LineNumberIndex, 				// line no of assert
};
	//PhaseIndex,						// phase of algorithm


class CustomFlash {
	/*
	 * Offset from start of UICR to reserved partitions in bytes
	 * E.G. to address in UICR we write a string. e.g. address 0x10000840
	 *
	 * Customer UICR is 0x80 (128) bytes (32 words)
	 *
	 * 0-2 faults
	 * 3-16 flags defined by app.
	 *
	 * 17 - 21 brownout traces 2@3 words
	 * 10 words (40 bytes) string assertion filename
	 */

	static const unsigned int OffsetToString = 88;
	static const unsigned int CountWordsOfFlashString = 10;
	static const unsigned int UnwrittenValue = 0xFfFfFfFf;
public:	// Used by BrownoutRecorder
	static const unsigned int BrownoutTrace1Index = 16;
	static const unsigned int BrownoutTrace2Index = 19;

public:


	// Write entire word to zero.
	static void writeZeroAtIndex(FlagIndex);

	// Write int to word if not already written
	static void tryWriteIntAtIndex(FlagIndex, unsigned int);

	// Did we already write to a word?
	static bool isWrittenAtIndex(FlagIndex);

	// To a fixed place in flash
	static void copyStringPrefixToFlash(const char* functionName);

	/*
	 * Write three words (uint32_t), without checking if already written.
	 */
	static void writeWordsAtIndex(FlagIndex, unsigned int, unsigned int, unsigned int );

	/*
	 * Clear bit of flag word.
	 * Since flash is NOR flash, bits can be zeroed but not rewritten to one.
	 */
	static  void writeBitAtIndex(FlagIndex, unsigned int);
};
