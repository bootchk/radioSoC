
#pragma once

#include "../platformTypes.h"  // MailContents

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
	MailContents item = 0;
	bool isItem = false;

	// Requires !isFull
	void put(MailContents item);


public:
	// Does nothing if isFull
	bool tryPut(MailContents item);

	// fetch first mail in box (if queued)
	MailContents fetch();

	bool isMail();

	bool isFull();
};
