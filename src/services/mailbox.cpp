
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


void Mailbox::tryPut(MailContents aItem){
	if (!isMail()) put(aItem);
}

void Mailbox::put(MailContents aItem){
	// FUTURE Thread safe: atomic

	assert(! isItem);	// Require mailbox not full

	item = aItem;
	isItem = true;
}

MailContents Mailbox::fetch(){
	assert(isItem);	// require mailbox not empty

	// Thread safe: copy item before deleting from queue
	MailContents result = item;
	isItem = false;
	return result;
}

bool Mailbox::isMail(){
	return isItem;
}
