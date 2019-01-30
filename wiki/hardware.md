<!--not to html-->

The emulator is intended to emulate the Apple ]['s hardware as closely as possible. The majority of the
information used to emulate the hardware's behavior was obtained from the original
[reference manual](http://www.apple-iigs.info/doc/fichiers/appleiiref.pdf).
Page numbers in the form `(pdf/document)` will be pointed out to find more information throughout this page.

## Display

The display is 560x384 pixels, but the resolution is still the original 280x192 "dots". The initial setting
of the monitor is text mode, display all text or graphics, display primary page, and display hi-res
graphics. There are 8 different soft switches to control these settings (90/79). Each display mode has two
pages to display, and soft switches to choose which gets displayed. For text and low resolution mode, page 1
is in the range $0400-$07FF in memory, and page 2 is in the range $0800-$0BFF. For high resolution mode, the
pages are located at $2000-$3FFF and $4000-$5FFF.

In text mode, the values at certain locations on the page produce a character on the screen. The exact
address and location of each character to display is shown on page 27/16 of the reference manual. To
understand the layout more easily, imagine that the screen is split into 3 horizontal sections, and the
page is dealing 24 40-character rows to them. After dealing to the third section each time, the last 8 bytes
of each 128 bytes are discarded, then it continues to give a 40-character row to the first section. In the
end, each section gets 8 rows.

In low resolution mode, the same pages are interpreted to produce colored blocks at a resolution 40 blocks
wide and 48 tall. Each byte represents two blocks, where the lower 4 bits are the upper block and vice versa.
The colors and memory map are given on page 28/17.

In high resolution mode, the bytes on its pages are interpreted as 3 colored 1x2 dot blocks each. Bits 1:0
determine the leftmost block's color, bits 3:2 the middle block's, and bits 5:4 the rightmost block's. Bit 6
is ignored. Bit 7 determines the palette from which to select the color. Each row is represented by 40 bytes
(30/19).

| Palette | Block | Color  |
|---------|-------|--------|
| 0 or 1  | %00   | black  |
| 0 or 1  | %11   | white  |
| 0       | %01   | green  |
| 0       | %10   | violet |
| 1       | %01   | orange |
| 1       | %10   | blue   |

While in either graphics mode, mixed graphics and text may be selected. This will interpret and display the
last 4 rows of the text page. If displaying page 2 in high resolution, page 2 of text mode will be interpreted.
The address of these last 4 rows are the same as on page 27/16.

## Keyboard

When pressing a key, the ASCII code for that key, combined with the highest bit (input flag) set, will be
placed in all memory addresses from $C000 to $C00F for the software to read. This nunmber may be placed
directy on the screen when in text mode to produce the character. Removing the highest bit will produce a
flashing character, and further subtracting $40 will produce an inverse character. Holding shift while
pressing a letter key makes no difference because all leters oln the original Apple ][ were uppercase.
Holding control while pressing a letter will produce a different, but still printable, number. The N and P
keys are unique because they produced the ^ and @ characters when holding shift, and holding ctrl+shift
while pressing them produced a unique code. In my implementation, holding shift with these numbers makes no
difference because @ and ^ are located on different keys, but holding ctrl and ctrl+shift produces the
original codes. The table on page 18/7 shows the exact values produced by the keys. When referencing a
memory address between $C010 and $C01F, the input flag will be set to 0 (89/78) (17/6).

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

Address $C020-$C02F is used for recording to the cassette. If no tape is open for recording, or one is open for
listening, this address will be ignored and nothing will happen. When a byte is written to this address,
it will be written to the file and the cursor will be advanced automatically.

Address $C060 is used for reading from the cassette. If no tape is open for listening, or one is open for
recording, this address will be ignored and nothing will happen. When reading from this address, the
byte pointed to by the cursor will be first inserted to the address and the cursor will be advanced
automatically.

## Peripheral Cards

Peripheral cards are described in the "Peripheral Cards" page

## Other hardware

The game controllers, speakers, pushbutton inputs and annunciator outputs are not implemented. Memory
addresses normally allocated to them and the utility strobe (90/79) are useful for general purpose RAM
as they have no special purpose in this emulator.
