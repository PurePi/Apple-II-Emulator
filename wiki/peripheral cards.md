<!--not to html-->

In the Apple ][, peripheral cards were attached to one of 8 parallel ports on the motherboard, and they
could add functionality like floppy drive support. In this emulator, peripheral cards are in the form
of .dll files. The Programmable ROM (PROM) and expansion ROM (XROM) are to be their own files. Peripheral
cards go in the "cards" directory and the name of the card should be given in the desired slot number in
config.json. For example:

| Card Name | config.json           | Hardware file | PROM file        | XROM file        |
|-----------|-----------------------|---------------|------------------|------------------|
| testCard  | "slot 1" : "testCard" | testCard.dll  | testCardPROM.bin | testCardXROM.bin |

Each card is required to have these functions:

| Function                                  | Purpose                                                                                                                                                                                                                                                                                                                                |
|-------------------------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| startup                                   | To initialize the card's hardware, like starting any threads if ongoing functionality is needed. If the card is designed for slot 0, it needs to take an unsigned char pointer to the emulator's memory (more info under the "Slot 0" section).                                                                                        |
| void shutdown()                           | To shut down the hardware (close files, end threads, etc)                                                                                                                                                                                                                                                                              |
| void deviceSelect(unsigned char)          | Callback function to handle a reference to your card's GPIO space. The parameter is the byte referenced ($00 to $0F). This could be used to bridge software and hardware together by designating a purpose for each byte. When your card is active, your card's PROM and XROM should be the only software interacting with this space. |
| void memRef(unsigned short) (slot 0 only) | Callback function to handle reference to any memory address ever.                                                                                                                                                                                                                                                                      |

The signatures for startup:

| Slot 0                        | Slot 1-7       |
|-------------------------------|----------------|
| void startup(unsigned char *) | void startup() |

Keep in mind that startup and shutdown may be called multiple times while the application is running. These
are the only symbols used by the emulator, and your card's symbols will be independent from any other
card's symbols so no conflicts between cards will occur.

The same dll card may be put into multiple slots, but if you choose to do so, the card must do everything
within functions or have a method of managing multiple instances of any data it keeps beyond its functions.
This is because there is only one instance of a shared object in memory, therefore only one instance of its
data. It would be possible to use the card's GPIO space and dynamic memory allocation to achieve this.

Additionally, my emulator implements the device select and I/O strobe lines as flags rather than physical
wires. There is an example card implementation under the cards/testCard directory to demonstrate everything
described here.

## GPIO

Each card has 16 bytes of the general purpose I/O space from $C080 to $C0FF. Slot 1 gets $C080-C08F, slot
2 gets $C090-$C09F, etc. The lower 4 bits of the address are consistent regardless of the slot, so you may
rely on the space to serve a specialized purpose (91/80).

## Scratchpad
Each card has an 8 byte scratchpad throughout pages $04 to $07. There is no callback for references to these
locations, and the card's PROM/XROM may use these locations for storage of data (93/82).

## PROM

The PROM is put into a 256-byte space in memory. $C100-$C1FF holds slot 1's PROM, $C200-$C2FF holds
slot 2's, etc. Generally, the card does not rely on being placed in a specific slot, so this PROM
should be address-independent. All JMP instructions to other parts of PROM should be replaced with branches
or forced branches. You may require that a card be put into a specific slot to remove this limitation and
save several bytes if you're short on space. PROM software is always entered through its lowest address
($C100, $C200, etc) and typically holds driver software (91/80).

## XROM

Each card has a 2KB, absolutely-addressed expansion rom that occupies the address space $C800-$CFFF.
It's important to note that all cards' expansion ROMs share this address space, and only one may be
accessed at a time. First, the device select (devSel) flag is turned on when the card's GPIO space is
referenced. This is done by software outside of the card's control, and partially enables the card's XROM.
Second, the I/O strobe line fully enables the card's XROM, and is triggered by a reference to the card's
PROM. The devSel flag must be set in order for the I/O strobe flag to be set. Otherwise, your PROM may
accidentally access the XROM of the previously active card, or if no card has been used yet, an IRQ will
be triggered. Once the I/O strobe flag has been set, the card's PROM may then reference address $CFFF to
turn off all cards' devSel flags. Now your card, because its I/O strobe flag is still set, has sole access
to its XROM (95/84).

## Slot 0

Slot 0 has no scratchpad space, PROM, or XROM allocated to it. It does, however, have GPIO space. It is
typically only used for RAM/ROM expansion. Memory expansions were originally done by physically rerouting
where the data bus got its data from, i.e. memory on the card rather than memory on the motherboard. Because
in this emulator all memory access is done via accessing a single memory array (a pointer to which is given
through startup), memory expansion may be emulated by copying the contents of memory into an internal array
for preservation, and copying contents from elsewhere into memory. This can be controlled via a soft
switch in its GPIO space. An example of similar bank switching is in memory.c, under the
`else if(address < 0xC800)` clause, where the XROM of the previous card is swapped out for the newly
activated one.

## Recap

To use peripheral cards:
1. A program run by the emulator accesses the GPIO space of the card it wants to use. This does not call that
card's deviceSelect, but it does turn on the devSel flag.
2. The program may jump into byte 0 (Address $Cn00) of the card's PROM space. This turns on the I/O strobe
flag if the devSel flag is on, and will override the previous card's I/O select flag. Regardless, the slot's
PROM will begin executing. If no card is inserted in this slot, an IRQ will be triggered.

Steps 1 and 2 are done by software on the emulator, and it is on the programmer/user to know
if a card is in a slot. The emulator has no way of telling its software if a slot is occupied or not.

3. The card's PROM may reference $CFFF to turn off all cards' devSel flags to prevent multiple cards from
having their XROM activated at the same time.
4. If your card's I/O strobe flag is set, your card's XROM is active and your PROM may now use it. If the
card's I/O strobe flag is not set, then the previous card's XROM is active. Your PROM may now use your (or
the last) card's subroutines in XROM.

If the card is meant for slot 0, its startup function requires an `unsigned char *` argument and the  `memRef`
function, which will be called any time any memory is accessed. Slot 0 cards do not have space allocated for
scratchpad, PROM, or XROM. PROM and XROM bin files will not be searched for if a card in slot 0.
