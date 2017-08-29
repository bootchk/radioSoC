/*
 * Interrupt handlers.
 *
 * This is just the outer layer: it understands that one handler takes events/interrupts from many devices.
 * One handler may farm out or multiplex to several next layers.
 * For any interrupt, all events are handled, for all of the several next layers.
 *
 * These override the default handlers.
 * To insure that the linker drags all these symbols in and overrides weak symbols:
 * #include these files into some other .cpp file.
 * (Alternatively, you could use whole-archive flag etc.)
 *
 * These are not "called" per se, but are referenced in the default interrupt vector table.
 */
 
 Also see radio IRQ, which does not multiplex, only gets events from radio.