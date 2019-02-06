<!--not to html-->

# Quick Start

No command line arguments are needed for running the emulator. The directory containing the emulator should have:
- `config.json`
- `roms.json`
- `tapes` subdirectory containing `.bin` files to act as cassette tapes
- `rom` subdirectory containing all the roms referenced in roms.json (see Boot Up and Reset Process section)
- `cards` subdirectory containing peripheral cards (if any are used)

Currently it is configured to load some examples into memory, ready to be run. examples.asm contains sample routines
to demonstrate functionality of all the hardware components. Simply change the example jumped to on line 13 to
see what it can do. The first instruction in examples.asm is what will be run first, so you may edit it to program it.

### Some Useful Tables in the [Reference Manual](http://www.apple-iigs.info/doc/fichiers/appleiiref.pdf)
(page numbers are in the form `pdf/manual`):

- 18/7: values of keys from keyboard
- 26/15: codes for displaying characters on the screen (keyboard input values correspond with normal characters)
- 27/16: memory map of characters to display while in text mode
- 89-90/78-79: Usage and addresses of built-in I/O

# Boot Up and Reset Process

When the emulator is first started, it will read `roms.json` and load files into memory. The format of each
file in `roms.json` should be `"name" : [startAddress, endAddress]`. For example, take `"examples" : [0xF800, 0xFFF9]`
(numbers in decimal, octal, and hexadecimal are allowed here). It will read `rom/examples.bin` and place it in memory at
the address `0xF800`. The end address exists to help yourself ensure that the bin file isn't too large if you want
to load multiple files contiguously and ensure that they don't overlap. The end address is also inclusive, so the
file in the example will be allowed to occupy address `0xFFF9`, but not go beyond it. If it did, it could be
overwritten by `rom/vectors.bin`, which is loaded into `0xFFFA`. If the end address is greater than `0xFFFF`, then it will
be treated as if it were `0xFFFF`. The files are loaded in the order that they appear in `roms.json`.

Once running, the F2 key will "shut down" the system, forcing the CPU to stop running,
but the emulator will remain open. The F1 key functions as the reset button, triggering the same
reset routine as when the emulator is first opened.

NOTE: While the emulator will cold start (when first opening the emulator) into a known state, a warm start
(restarting with F1) will leave all registers and memory in the state they were in at the moment of reset
with the exception of the stack pointer, which is always reset to $FF, and the interrupt disable flag, which
is always set to 1. Because of this, the reset routine (beginning code of rom.as) should not make any
assumptions about the initial state of registers or memory. The initial setting from a cold start of hardware
related features is detailed in the hardware page where appropriate.

# Programming

I use [6502-asm](https://github.com/PurePi/6502-Assembler) to write the software for this emulator to run.
Instructions on using the program are in that repository's wiki.

## Keybindings

| Key   | Function                                                                                                                                                                                                                                            |
| ----- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F1    | Reset. This key shuts down the emulator if it's not already off, and restarts it. The contents of the registers and memory will remain as they were at the moment of shutting down. Closes any tape that's open for recording or listening.         |
| F2    | Shut down. To start back up, press F1.                                                                                                                                                                                                              |
| F3    | Record. A file whose name is given in config.json as the value of "tape" will be opened. If a file is already open for recording, it will be closed. If a file is already open for listening, it will be closed and another opened for recording.   |
| F4    | Listen. A file whose name is given in config.json as the value of "tape" will be opened. If a file is already open for listening,it will be closed. If a file is already open for recording, it will be closed and another opened for listening.    |
