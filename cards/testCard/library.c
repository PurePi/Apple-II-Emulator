#include <stdio.h>

/**
 * initializes testCard
 *
 * memory pointer not needed because this card isn't for slot 0
 */
void startup()
{
    printf("Hello from testCard!\n");
}

/**
 * shuts down the card
 */
void shutdown()
{
    printf("shutting down testCard\n");
}

/**
 * handles a reference to this card's GPIO space
 * this function is called BEFORE the processor retrieves data if reading from it,
 * so it will read the value that this function returns,
 * and is called AFTER writing data to it, so this function receives the newly written value
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
        printf("This card is in slot %d\n", value);
    }
    else if(which == 4)
    {
        // and byte 4 will always give the CPU the magic number 42
        return 42;
    }

    // we don't want to do anything special to any other bytes in this example, so leave them be
    return value;
}

// memRef not needed because this card isn't for slot 0