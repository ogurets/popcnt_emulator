# popcnt emulator
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
