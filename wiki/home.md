# Usage

Due to the libraries and APIs used, this emulator only works with Windows.

No command line arguments are needed for running the emulator. The directory that the emulator is in should
contain the rom folder with rom.bin and vectors.bin to work.

## Boot up and reset process
When the emulator is first started, the interrupt vector table in vectors.bin in loaded into address $FFFA.
As it is now, the reset vector points to address $FA00 and the IRQ and NMI vectors both point to $0200.
rom.bin is then loaded into address $FA00, and the CPU reset is triggered.

Once running, the F2 key will "shut down" the system, forcing the CPU to stop running,
but the emulator will remain open. The F1 key functions as the reset button, triggering the same
reset routine as when the emulator is first opened.

NOTE: While the emulator will cold start into a known state, a warm start will leave all registers and memory
in the state they were in at the moment of reset with the exception of the stack pointer, which is always
reset to $FF, and the interrupt disable flag, which is always set to 1. Because of this, the reset routine
should not make any assumptions about the initial state of registers or memory. The initial setting from
a cold start of hardware related features is detailed in the hardware page.

## Programming
I use [6502-asm](https://github.com/PurePi/6502-Assembler) to write the software for this emulator to run.
Instructions on using the program are in that repository's wiki. The rom.asm and vector.asm source code
is in the rom directory. rom.asm contains a few example routines to demonstrate interaction with the
monitor and keyboard.

## Keybindings

| Key | Function                                                                                                                                                                                                                                          |
|-----|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| F1  | Reset. This key shuts down the emulator if it's not already off, and restarts it. The contents of the registers and memory will remain as they were at the moment of shutting down. Closes any tape that's open for recording or listening.       |
| F2  | Shut down. To start it back up, press F1.                                                                                                                                                                                                         |
| F3  | Record. A file whose name is given in config.json as the value of "tape" will be opened. If a file is already open for recording, it will be closed. If a file is already open for listening, it will be closed and another opened for recording. |
| F4  | Listen. A file whose name is given in config.json as the value of "tape" will be opened. If a file is already open for listening,it will be closed. If a file is already open for recording, it will be closed and another opened for listening.  |