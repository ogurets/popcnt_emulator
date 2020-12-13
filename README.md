# popcnt emulator
**Cyberpunk 2077 update**: see below!

Pintool library for running Quantum Break on pre-SSE4.2 CPUs

It's very raw and dirty, you'll probably have a very hard time building it and will probably have the game crashed eventually (when it gets to the non-patched popcnt). I'm "working" on it ("testing" it, you know ;) ).

Compared to the widespread SDE solution it doesn't kill performance (0% performance hit!), as the Pin tool is having being detached after initial loading.

There doesn't seem to be any actual popcnt calls, so everything I had to do so far is patching cpuid to enable popcnt feature flag!

## Why were SDE solution bad?
Because you don't have to emulate the entire Nehalem architecture for just one single popcnt instruction!

And don't have to keep Pin tracer running, which also hogs CPU quite hard.

## Usage:
Suppose you've downloaded your Pin toolkit version 2.14-71313-msvc11-windows into <pre>c:\pin-2.14-71313-msvc11-windows</pre>

Then running the game will look like:

<pre>c:\pin-2.14-71313-msvc11-windows\pin.exe -follow-execv -t64 instruction_hook64.dll -t instruction_hook32.dll -- QuantumBreak.exe</pre>

Note that you'll probably have to build both x86 and x64 versions of the tool and name them "instruction_hook64.dll" and "instruction_hook32.dll" respectively.

If you wish to contact me (to ask for binaries, help or simply chat) please use the Issues tab.

## Cyberpunk 2077
**Good news**: it works. Not a 0% performance hit this time due to the different approach, but still quite playable.

**Bad news**: _very_ version-bound stuff! Requires GOG version 1.03, probably won't work on other versions (but may though). You could get past the main menu with this, but crash later.

**Not so bad news**: I will describe the way you could fix it to work with your version of the game, it is relatively easy.

### Usage:
Build only `popcnt_hotpatch` project! x64 Release mode. Adjust target paths to your preferred ones.

It builds into `icudt.dll`. Don't change that name.

Replace your game's `icudt.dll` library with this one (make a backup of the original!).

Just start the game as usual!

### For the tech-savvy fellows, here's how it was done:
This time popcnt's are everywhere! They don't just pop up at the game loading stage and not used later, which rules out all PIN-based solutions (too slow).
I've tried JIT, Trace mode and Probe. Probe could do the trick, but it refuses to hotwire code blocks less than 7 bytes long (popcnts are 4-5 bytes long).
Manual patching is out of the picture too, as there are about 120 popcnts in the game! And they are used often.

So, I did a different thing - built a DLL, which wraps a small ICU DLL packaged with the game (so the game could statically import it without patching).
This DLL is installing a global exception handler, which intercepts "Illegal instruction" exceptions and emulates popcnt in the exception handler.
Afterwards it is putting the result to the correct destination and jumps over popcnt.

This solution is versatile and could be made even more optimized and version-independent, but still imposes some overhead.
Also because the game starts throwing a lot of exceptions, attaching a debugger to it would really hit the brakes. But that's not a big issue since you probably wouldn't want to.

### Adding new fixups to the tool
If your game crashed with unhandled popcnt exception, the `hotfix.log` file will appear at the game folder with the instruction address.
Disassemble this spot and add another `HOTFIX` line to the code at the bottom (there is a comment), using other lines as an example.

Use `ctx` to access and modify registers and direct pointer access for memory (since we are inside the game's process, we don't have to `ReadProcessMemory` or something like that).

### A note about the infamous AVX crash after Prologue
This tool doesn't solve this crash because emulating the whole AVX feels a lot more tedious than just one simple instruction.

But there are fixes for it already and they work. Basically they're just NOP'ing out AVX instructions, you could do it yourself.
