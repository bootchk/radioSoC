
#include <cassert>

#include "mailbox.h"


/*
 * This implementation:
 * - holds only one item.
 * - is not generic on type of object held
 * - is not thread safe
 *
 * - is not static, i.e. this data members
 */


bool Mailbox::tryPut(MailContents aItem){
	if (!isFull()) {
		put(aItem);
		return true;
	}
	else {
		return false;
	}
}

void Mailbox::put(MailContents aItem){
	// FUTURE Thread safe: atomic

	assert(! isFull());	// Require

	item = aItem;
	isItem = true;
}

MailContents Mailbox::fetch(){
	assert(isMail());	// require

	// Thread safe: copy item before deleting from queue
	MailContents result = item;
	isItem = false;
	return result;
}

bool Mailbox::isMail(){  return isItem; }

// Since only one item, any is full
bool Mailbox::isFull(){  return isItem; }
