#ifndef APPLE_II_EMULATOR_PERIPHERAL_H
#define APPLE_II_EMULATOR_PERIPHERAL_H

#include <stdio.h>

// a peripheral card represented by a dll and its ROMs
struct peripheralCard
{
    void *handle;
    void (*startup)();
    void (*shutdown)();
    char expansionRom[0x800];
    char deviceSelect;
    char IOSelect;
    char expansionAccess;
};

extern struct peripheralCard peripherals[8];

extern char readCards();

#endif //APPLE_II_EMULATOR_PERIPHERAL_H
