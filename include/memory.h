#ifndef APPLE_II_EMULATOR_MEMORY_H
#define APPLE_II_EMULATOR_MEMORY_H

#include <windows.h>

extern unsigned char memory[0x10000];
extern HANDLE memMutex;
extern HANDLE screenMutex;
extern HANDLE runningMutex;

extern unsigned char readByte(unsigned short address);
extern void writeByte(unsigned short address, unsigned char byte);

#endif //APPLE_II_EMULATOR_MEMORY_H
