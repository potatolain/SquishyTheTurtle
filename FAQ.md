# FAQ
## Why?
While chatting in the #ludumdare room, a number of questions came up, and in addition anyone looking at source is likely to have a few more. I figured I'd write down some of these details before I forget.

## Why GBDK?
GBDK leaves some things to be desired - the generated code is sometimes inefficient, and on occasion even wrong. Its output is a bit unclear at times. It also has its share of quirks.

All that said, it does a great job of protecting me from a system I don't know that well, and its quirks. I can manipulate graphics without worrying about vblank timing or anything like that; gbdk does it for me. 

It also lets me write code a lot faster. I'm pretty crafty with assembly (at least 6502) but being able to write in terms of a for loop, etc is huge with a short time limit.

The short answer to this question: GBDK allows for rapid development in a reasonably high-level language while still allowing low-enough-level access to get things done.

## What is helper_1.c/.h? Why is it?

That file is a secondary bank to do a lot of calculations that will not fit into the primary code bank. It tries to offload any calculations that do not need access to other ROM banks.

Ideally, the code would have been logically split into a a few more meaningful files/banks, but time was of the essence. Keep in mind this was written inside of two days. I did something that worked.

The more interesting question is why it is necessary. The GameBoy can address two ROM banks with fairly limited space, (16kb a pop) but one of those banks is switchable, and can be replaced with one of up to 32 other 16kb banks. 

Obviously, the goal is to keep the main non-switchable bank as light as possible. I tried to limit it to only operations that needed information in one of the non-fixed banks.

Unfortunately, I didn't really succeed in keeping the fixed bank code small. The reason so many arbitrary things are in helper_1.c is that I did something horribly inefficient in the main bank (I'm honestly not sure what) and it is nearly completely full. As I added new things to that bank, I continually had to pull other things out to make new code fit.

The short answer to this question: An abombination resulting from extremely inefficient ROM usage by my code/the assembler.

## Why are your tiles/maps in assembly code? And what's with the *awful* makefile transformation?

There are a few reasons for this. For one, I seem to have stumbled on some bug with GBDK where it will use up extra resources for const arrays like the ones I'm using. GBTD and GBMB both support outputting to gbdk-compatible .c/.h files, but the result is so inefficient that I cannot use a tile map the size I want to. (100x100) I didn't spend forever researching this problem; I found a solution that worked and moved on.

As for the awful makefile transformation, none of the assembly formats supported by GBTD/GBMB actually line up with the assembler used by GBDK. Technically, I could make gbdk use rgbds as its assembler and use those files being output, but reworking that for this reason (and creating new bugs by doing so) seemed less painful than using the ugly sed line. It isn't pretty, but it's a fairly simple text subsitution for the few things the exported files do. 

