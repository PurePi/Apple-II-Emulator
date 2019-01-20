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

## Other hardware
The cassette drive, game controllers, speakers, and peripheral board support are not yet implemented.