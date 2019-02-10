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

| Function                                  | Purpose                                                                                                                                                                                                                                         |
|-------------------------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| startup                                   | To initialize the card's hardware, like starting any threads if ongoing functionality is needed. If the card is designed for slot 0, it needs to take an unsigned char pointer to the emulator's memory (more info under the "Slot 0" section). |
| void shutdown()                           | To shut down the hardware (close files, end threads, etc)                                                                                                                                                                                       |
| void deviceSelect(int, unsigned char)     | Callback function to handle a reference to your card's GPIO space. This could be used to bridge software and hardware together by designating a purpose for each byte.                                                                          |
| void memRef(unsigned short) (slot 0 only) | Callback function to handle reference to any memory address ever.                                                                                                                                                                               |

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

There is an example card implementation under the cards/testCard directory to demonstrate everything
described here.

## GPIO

Each card has 16 bytes of the general purpose I/O space from $C080 to $C0FF. Slot 0 gets $C080-C08F, slot
1 gets $C090-$C09F, etc. The lower 4 bits of the address are consistent regardless of the slot, so you may
rely on the space to serve a specialized purpose and interact with the hardware (91/80). When this space is
referenced, the card's deviceSelect function is called. The int parameter is the byte referenced ($00 to $0F).
The unsigned char is the value at that address (useful for when PROM or XROM writes to it). When your card is
active, your card's PROM and XROM should be the only software interacting with this space, although nothing
stops outside code from referencing this space and triggering this callback.

## Scratchpad
Each card has an 8 byte scratchpad throughout pages $04 to $07. There is no callback for references to these
locations, and the card's PROM/XROM may use these locations for storage of data (93/82).

## PROM

The PROM is put into a 256-byte space in memory. $C100-$C1FF holds slot 1's PROM, $C200-$C2FF holds
slot 2's, etc. Generally, the card does not rely on being placed in a specific slot, so this PROM
should be address-independent. All JMP instructions to other parts of PROM should be replaced with branches
or forced branches. You may require that a card be put into a specific slot to remove this limitation and
save several bytes if you're short on space. PROM software should always be entered through its lowest address
($C100, $C200, etc) and typically holds driver software (91/80).

## XROM

Each card has a 2KB, absolutely-addressed expansion rom that occupies the address space $C800-$CFFF.
It's important to note that all cards' expansion ROMs share this address space, and only one may be
accessed at a time. Any reference to the card's PROM will activate that card's XROM and IOStrobe line.
Unlike the original implementation, it is not necessary for address $CFFF to disable any flags because
the emulator itself handles the XROM and decides which to use, rather than having a process of telling the
cards when they should direct references to XROM to their own on-board memory (95/84). $CFFF has no special
purpose in this emulator.

## Slot 0

Slot 0 has no scratchpad space, PROM, or XROM allocated to it. It does, however, have GPIO space. It is
typically only used for RAM/ROM expansion. Memory expansions were originally done by physically rerouting
where the data bus got its data from, i.e. memory on the card rather than memory on the motherboard. Because
in this emulator all memory access is done via accessing a single memory array (a pointer to which is given
through startup), memory expansion may be emulated by copying the contents of memory into an internal array
for preservation, and copying contents from elsewhere into memory. This can be controlled via a soft
switch in its GPIO space. An example of similar bank switching is in memory.c, under the
`else if(address < 0xC800)` block, where the XROM of the previous card is swapped out for the newly
activated one.

## Disk-II-Controller

A card named "Disk-II-Controller" additionally is required to have the function `void diskDoor(int)`
to handle opening/closing the doors of up to two floppy disk drives. Pressing F5 will call `diskDoor(0)`
(for drive A) and pressing F6 will call `diskDoor(1)` (for drive B). This is intended to let the card handle
file management and effectively let the user insert/eject floppy disks. It also must have
`char fileDirectory[FILENAME_MAX]`, used to give the card the absolute location of the emulator. My own
implementation of this card is  [here](https://github.com/PurePi/Disk-II-Controller).