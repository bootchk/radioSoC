
#pragma once

#include "../platformTypes.h"  // WorkPayload

/*
 * Simple mailbox:
 * - holding one item
 * - listener polls
 * - not thread-safe (only one poster and listener)
 *
 * Statically configured to empty.
 * Algebra:
 * reset; isMail() == false
 * put(); isMail() == true; fetch(); isMail() == false
 *
 * Note there is no init() to empty the mailbox.
 * After an exception, it might be necessary to flush the mailbox by reading it.
 */



class Mailbox {
	WorkPayload item = 0;
	bool isItem = false;

public:
	// Does nothing if mailbox full
	void tryPut(WorkPayload item);

	// Requires mailbox not full
	void put(WorkPayload item);

	// fetch first mail in box (if queued)
	WorkPayload fetch();

	bool isMail();
};
