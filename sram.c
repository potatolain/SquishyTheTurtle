// This file creates a number of variables stored in sram, so they will be persisted when the console is turned off.
// Don't put any functions here!! You will literally save and run them from a very small sram bank. That's bad!

#include <gb/gb.h>
UBYTE sram_magicByte; // If this byte isn't set to a known value, clear SRAM out.
UBYTE sram_numberOfStarts[5];
UBYTE sram_numberOfEnds[5];