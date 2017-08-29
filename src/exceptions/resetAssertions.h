
/*
 * Functions that assert() that certain kinds of reset have not happened.
 *
 * Used in debugging.
 */

// Raises assertion if any reset other than POR or BOR have occurred since POR or call to clearResetReason
void assertNoResetsOccurred();

/*
 * Call this early, just after POR.
 * While POR will clear the register RESETREAS, debugger may set the DIF bit just before jumping to start address.
 * At least call this before you call assertNoResetsOccurred(), else the DIF may assert.
 */
void clearResetReason();


