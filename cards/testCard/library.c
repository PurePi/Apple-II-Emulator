#include <stdio.h>

/**
 * initializes testCard
 *
 * memory pointer not needed because this card isn't for slot 0
 */
void startup()
{
    // this function will be run each time the emulator boots up/resets
}

/**
 * shuts down the card
 */
void shutdown()
{
    // this function will be called each time the emulator is shut down
}

/**
 * handles a reference to this card's GPIO space
 * this function is called BEFORE the processor retrieves data if reading from it,
 * so it will read the value that this function returns,
 * and it is called AFTER the processor writes data to it, so this function receives the newly written value
 *
 * @param which which byte was accessed (0 to 15)
 * @param value byte contained at the address
 * @return byte to insert at that address
 */
unsigned char deviceSelect(int which, unsigned char value)
{
    // we want PROM to tell us which slot this card is in through byte 0 in this example
    if(which == 0)
    {
        // do something with that information here
    }
    else if(which == 4)
    {
        // and byte 4 will always give the CPU the magic number 42
        return 42;

        // there's no way to tell from here whether this location is being read from or written to
        // so you must designate each location to one or the other, and be sure to abide by it in your PROM/XROM
    }

    // we don't want to do anything special to any other bytes in this example, so leave them be
    return value;
}

// memRef not needed because this card isn't for slot 0