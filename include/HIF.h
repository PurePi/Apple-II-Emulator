#ifndef APPLE_II_EMULATOR_HIF_H
#define APPLE_II_EMULATOR_HIF_H

extern char running;
extern char screenFlagsChanged;
extern char screenFlags;
extern unsigned short pageBase;
extern unsigned short pageEnd;

#define gr      0b0001
#define mix     0b0010
#define pri     0b0100
#define lores   0b1000

extern int startGLFW();

#endif //APPLE_II_EMULATOR_HIF_H
