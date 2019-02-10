#ifndef APPLE_II_EMULATOR_PERIPHERAL_H
#define APPLE_II_EMULATOR_PERIPHERAL_H

#include <stdio.h>

// a peripheral card represented by a dll and its ROMs
struct peripheralCard
{
    void *handle;
    void *startup; // cast to appropriate function depending on slot in mountCards
    void (*shutdown)();
    void (*memRef)(unsigned short address);
    unsigned char (*deviceSelect)(int which, unsigned char value);
    char expansionRom[0x800];
    char IOStrobe;
};

extern struct peripheralCard peripherals[8];

extern char mountCards();
extern void unmountCards();
extern void (*diskDoor)(int);

#endif //APPLE_II_EMULATOR_PERIPHERAL_H
