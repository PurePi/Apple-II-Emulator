The emulator is intended to emulate the Apple ]['s hardware as closely as possible. The majority of the
information used to emulate the hardware's behavior was obtained from the original
[reference manual](http://www.apple-iigs.info/doc/fichiers/appleiiref.pdf).
Page numbers in the form `(pdf/document)` will be pointed out to find more information throughout this page.

## Display
The display is 560x384 pixels, but the resolution is still the original 280x192 "dots". The initial setting
of the monitor is text mode, display all text or graphics, display primary page, and display low-res
graphics (90/79) At the moment, only text mode is implemented, although the soft switches are also
implemented. If switching graphics modes, nothing will be displayed until the text mode switch is set.
Characters on the text pages are interpreted as ASCII rather than the codes and display modes supported by
the original Apple ][ (26/15). These features will be implemented eventually.

Each display mode has two pages to display, and soft switches to choose which gets displayed. For text and
low resolution mode, page 1 is in the range $0400-$07FF in memory, and page 2 is in the range $0800-$0BFF.
The exact addresses of each character to display is shown on page 27/16 of the reference manual.

## Keyboard
When pressing a key, the ASCII code for that key, combined with the highest bit (input flag) set, will be
placed in all memory addresses from $C000 to $C00F for the software to read. When referencing a memory
address between $C010 and $C01F, the input flag will be set to 0 (89/78) (17/6).

## Cassette Recorder
The cassette recorder is implemented as an interface for file I/O. The original cassette interface used a
toggle switch to produce a stream of "clicks" to record data onto the cassette, and a flag input to listen
to those clicks to read that data (33/22). For the sake of simplicity, my implementation treats the
input/output address as data in/data out registers rather than a flag input and toggle switch (89/78).

Before writing to or reading from the cassette, you must press F3 (record) or F4 (playback) to do so.
This opens a file whose name is given as the "tape" value in config.json for the appropriate function.
The `.bin` file extension will be appended to the file and it will be saved in the "tapes" directory.
The value at the time of pressing the button will be used as the file name, and the value can be changed
at any time, even while the emulator is running. If pressing the same key that was used to open the file,
then the file will be closed. If you press the other button, then the file will be closed and another
with the current value of "tape" will be opened for the selected function. If the name of a nonexistant
file is given when opening a file for listening, no file will be opened and nothing will happen.

Address $C060 is used for recording to the cassette. If no tape is open for recording, or one is open for
listening, this address will be ignored and nothing will happen. When a byte is written to this address,
it will be written to the file and the cursor will be advanced automatically.

Address $C020 is used for reading from the cassette. If no tape is open for listening, or one is open for
recording, this address will be ignored and nothing will happen. When reading from this address, the
byte pointed to by the cursor will be first inserted to the address and the cursor will be advanced
automatically.

## Other hardware
The game controllers, speakers, and peripheral card support are not yet implemented.