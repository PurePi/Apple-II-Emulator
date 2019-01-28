#include "HIF.h"
#include "memory.h"
#include <GLFW/glfw3.h>
#include <mem.h>
#include <stdio.h>
#include <process.h>
#include <ctype.h>
#include "cpu.h"
#include "cassette.h"
#include "peripheral.h"

char running = 0;
char screenFlagsChanged = 0;
char screenFlags = pri | lores; // TODO change this back to hires when done testing lores

unsigned short pageBase = 0x0400;
unsigned short pageEnd = 0x0BFF;

static GLFWwindow *window;

// bitmaps to draw on the screen
static char lopage1[7680] = { 0 };
static char lopage2[7680] = { 0 };
static char hipage1[7680] = { 0 };
static char hipage2[7680] = { 0 };

// to keep the chars in the current page while in text mode, flashing characters may be turned to whitespace
static unsigned char characters[0x400] = { 0 };
// to preserve all chars on the screen in order to put flashing characters back onto the screen
static unsigned char saveChars[0x400] = { 0 };
// to keep display mode of each char
static unsigned char charFlags[0x400] = { 0 };
// for current state of flashing
static char showing = 1;

#define FLASHING 0b01
#define INVERSE 0b10

// to blink every second, gets decremented each frame
static int flashTimer = 30;

static void (*fillBuffer)();

/**
 * inserts a single character into the video memory
 * @param page 1 = page 1, 2 = page 2
 * @param startAt upper-left pixel coordinate for this char's bitmap
 * @param character the character to draw
 * @param flag the flags for that char
 */
static void insertChar(int page, int startAt, char character, unsigned char flag)
{
    unsigned char r0 = 0, r1 = 0, r2 = 0, r3 = 0, r4 = 0,
            r5 = 0, r6 = 0, r7 = 0;

    // TODO refactor to draw chars from $00-$3F
    switch(character)
    {
    case 0x00: // @
        r1 = 0b011100;
        r2 = 0b100010;
        r3 = 0b101010;
        r4 = 0b101110;
        r5 = 0b101100;
        r6 = 0b100000;
        r7 = 0b011110;
        break;
    case 0x01: // A
        r1 = 0b001000;
        r2 = 0b010100;
        r3 = 0b100010;
        r4 = 0b100010;
        r5 = 0b111110;
        r6 = 0b100010;
        r7 = 0b100010;
        break;
    case 0x02: // B
        r1 = 0b111100;
        r2 = 0b100010;
        r3 = 0b100010;
        r4 = 0b111100;
        r5 = 0b100010;
        r6 = 0b100010;
        r7 = 0b111100;
        break;
    case 0x03: // C
        r1 = 0b011100;
        r2 = 0b100010;
        r3 = 0b100000;
        r4 = 0b100000;
        r5 = 0b100000;
        r6 = 0b100010;
        r7 = 0b011100;
        break;
    case 0x04: // D
        r1 = 0b111100;
        r2 = 0b100010;
        r3 = 0b100010;
        r4 = 0b100010;
        r5 = 0b100010;
        r6 = 0b100010;
        r7 = 0b111100;
        break;
    case 0x05: // E
        r1 = 0b111110;
        r2 = 0b100000;
        r3 = 0b100000;
        r4 = 0b111100;
        r5 = 0b100000;
        r6 = 0b100000;
        r7 = 0b111110;
        break;
    case 0x06: // F
        r1 = 0b111110;
        r2 = 0b100000;
        r3 = 0b100000;
        r4 = 0b111100;
        r5 = 0b100000;
        r6 = 0b100000;
        r7 = 0b100000;
        break;
    case 0x07: // G
        r1 = 0b011110;
        r2 = 0b100000;
        r3 = 0b100000;
        r4 = 0b100000;
        r5 = 0b100110;
        r6 = 0b100010;
        r7 = 0b011110;
        break;
    case 0x08: // H
        r1 = 0b100010;
        r2 = 0b100010;
        r3 = 0b100010;
        r4 = 0b111110;
        r5 = 0b100010;
        r6 = 0b100010;
        r7 = 0b100010;
        break;
    case 0x09: // I
        r1 = 0b011100;
        r2 = 0b001000;
        r3 = 0b001000;
        r4 = 0b001000;
        r5 = 0b001000;
        r6 = 0b001000;
        r7 = 0b011100;
        break;
    case 0x0A: // J
        r1 = 0b000010;
        r2 = 0b000010;
        r3 = 0b000010;
        r4 = 0b000010;
        r5 = 0b000010;
        r6 = 0b100010;
        r7 = 0b011100;
        break;
    case 0x0B: // K
        r1 = 0b100010;
        r2 = 0b100100;
        r3 = 0b101000;
        r4 = 0b110000;
        r5 = 0b101000;
        r6 = 0b100100;
        r7 = 0b100010;
        break;
    case 0x0C: // L
        r1 = 0b100000;
        r2 = 0b100000;
        r3 = 0b100000;
        r4 = 0b100000;
        r5 = 0b100000;
        r6 = 0b100000;
        r7 = 0b111110;
        break;
    case 0x0D: // M
        r1 = 0b100010;
        r2 = 0b110110;
        r3 = 0b101010;
        r4 = 0b100010;
        r5 = 0b100010;
        r6 = 0b100010;
        r7 = 0b100010;
        break;
    case 0x0E: // N
        r1 = 0b100010;
        r2 = 0b100010;
        r3 = 0b110010;
        r4 = 0b101010;
        r5 = 0b100110;
        r6 = 0b100010;
        r7 = 0b100010;
        break;
    case 0x0F: // O
        r1 = 0b011100;
        r2 = 0b100010;
        r3 = 0b100010;
        r4 = 0b100010;
        r5 = 0b100010;
        r6 = 0b100010;
        r7 = 0b011100;
        break;
    case 0x10: // P
        r1 = 0b111100;
        r2 = 0b100010;
        r3 = 0b100010;
        r4 = 0b111100;
        r5 = 0b100000;
        r6 = 0b100000;
        r7 = 0b100000;
        break;
    case 0x11: // Q
        r1 = 0b011100;
        r2 = 0b100010;
        r3 = 0b100010;
        r4 = 0b100010;
        r5 = 0b101010;
        r6 = 0b100100;
        r7 = 0b011010;
        break;
    case 0x12: // R
        r1 = 0b111100;
        r2 = 0b100010;
        r3 = 0b100010;
        r4 = 0b111100;
        r5 = 0b101000;
        r6 = 0b100100;
        r7 = 0b100010;
        break;
    case 0x13: // S
        r1 = 0b011100;
        r2 = 0b100010;
        r3 = 0b100000;
        r4 = 0b011100;
        r5 = 0b000010;
        r6 = 0b100010;
        r7 = 0b011100;
        break;
    case 0x14: // T
        r1 = 0b111110;
        r2 = 0b001000;
        r3 = 0b001000;
        r4 = 0b001000;
        r5 = 0b001000;
        r6 = 0b001000;
        r7 = 0b001000;
        break;
    case 0x15: // U
        r1 = 0b100010;
        r2 = 0b100010;
        r3 = 0b100010;
        r4 = 0b100010;
        r5 = 0b100010;
        r6 = 0b100010;
        r7 = 0b011100;
        break;
    case 0x16: // V
        r1 = 0b100010;
        r2 = 0b100010;
        r3 = 0b100010;
        r4 = 0b100010;
        r5 = 0b100010;
        r6 = 0b010100;
        r7 = 0b001000;
        break;
    case 0x17: // W
        r1 = 0b100010;
        r2 = 0b100010;
        r3 = 0b100010;
        r4 = 0b101010;
        r5 = 0b101010;
        r6 = 0b110110;
        r7 = 0b100010;
        break;
    case 0x18: // X
        r1 = 0b100010;
        r2 = 0b100010;
        r3 = 0b010100;
        r4 = 0b001000;
        r5 = 0b010100;
        r6 = 0b100010;
        r7 = 0b100010;
        break;
    case 0x19: // Y
        r1 = 0b100010;
        r2 = 0b100010;
        r3 = 0b010100;
        r4 = 0b001000;
        r5 = 0b001000;
        r6 = 0b001000;
        r7 = 0b001000;
        break;
    case 0x1A: // Z
        r1 = 0b111110;
        r2 = 0b000010;
        r3 = 0b000100;
        r4 = 0b001000;
        r5 = 0b010000;
        r6 = 0b100000;
        r7 = 0b111110;
        break;
    case 0x1B: // [
        r1 = 0b111110;
        r2 = 0b110000;
        r3 = 0b110000;
        r4 = 0b110000;
        r5 = 0b110000;
        r6 = 0b110000;
        r7 = 0b111110;
        break;
    case 0x1C: // \\
        r2 = 0b100000;
        r3 = 0b010000;
        r4 = 0b001000;
        r5 = 0b000100;
        r6 = 0b000010;
        break;
    case 0x1D: // ]
        r1 = 0b111110;
        r2 = 0b000110;
        r3 = 0b000110;
        r4 = 0b000110;
        r5 = 0b000110;
        r6 = 0b000110;
        r7 = 0b111110;
        break;
    case 0x1E: // ^
        r1 = 0b001000;
        r2 = 0b010100;
        r3 = 0b100010;
        break;
    case 0x1F: // _
        r7 = 0b111110;
        break;
    // $20 is whitespace
    case 0x21: // !
        r1 = 0b001000;
        r2 = 0b001000;
        r3 = 0b001000;
        r4 = 0b001000;
        r6 = 0b001000;
        break;
    case 0x22: // "
        r1 = 0b010100;
        r2 = 0b010100;
        break;
    case 0x23: // #
        r1 = 0b010100;
        r2 = 0b010100;
        r3 = 0b111110;
        r4 = 0b010100;
        r5 = 0b111110;
        r6 = 0b010100;
        r7 = 0b010100;
        break;
    case 0x24: // $
        r1 = 0b001000;
        r2 = 0b011110;
        r3 = 0b101000;
        r4 = 0b011100;
        r5 = 0b001010;
        r6 = 0b111100;
        r7 = 0b001000;
        break;
    case 0x25: // %
        r1 = 0b110000;
        r2 = 0b110010;
        r3 = 0b000100;
        r4 = 0b001000;
        r5 = 0b010000;
        r6 = 0b100110;
        r7 = 0b000110;
        break;
    case 0x26: // &
        r1 = 0b010000;
        r2 = 0b101000;
        r3 = 0b101000;
        r4 = 0b010000;
        r5 = 0b101010;
        r6 = 0b100100;
        r7 = 0b011010;
        break;
    case 0x27: // '
        r1 = 0b001000;
        r2 = 0b001000;
        break;
    case 0x28: // (
        r1 = 0b000100;
        r2 = 0b001000;
        r3 = 0b010000;
        r4 = 0b010000;
        r5 = 0b010000;
        r6 = 0b001000;
        r7 = 0b000100;
        break;
    case 0x29: // )
        r1 = 0b010000;
        r2 = 0b001000;
        r3 = 0b000100;
        r4 = 0b000100;
        r5 = 0b000100;
        r6 = 0b001000;
        r7 = 0b010000;
        break;
    case 0x2A: // *
        r1 = 0b001000;
        r2 = 0b101010;
        r3 = 0b011100;
        r4 = 0b001000;
        r5 = 0b011100;
        r6 = 0b101010;
        r7 = 0b001000;
        break;
    case 0x2B: // +
        r2 = 0b001000;
        r3 = 0b001000;
        r4 = 0b111110;
        r5 = 0b001000;
        r6 = 0b001000;
        break;
    case 0x2C: // ,
        r5 = 0b001000;
        r6 = 0b001000;
        r7 = 0b010000;
        break;
    case 0x2D: // -
        r4 = 0b111110;
        break;
    case 0x2E: // .
        r6 = 0b010000;
        break;
    case 0x2F: // /
        r2 = 0b000010;
        r3 = 0b000100;
        r4 = 0b001000;
        r5 = 0b010000;
        r6 = 0b100000;
        break;
    case 0x30: // 0
        r1 = 0b011100;
        r2 = 0b100010;
        r3 = 0b100110;
        r4 = 0b101010;
        r5 = 0b110010;
        r6 = 0b100010;
        r7 = 0b011100;
        break;
    case 0x31: // 1
        r1 = 0b001000;
        r2 = 0b011000;
        r3 = 0b001000;
        r4 = 0b001000;
        r5 = 0b001000;
        r6 = 0b001000;
        r7 = 0b011100;
        break;
    case 0x32: // 2
        r1 = 0b011100;
        r2 = 0b100010;
        r3 = 0b000010;
        r4 = 0b001100;
        r5 = 0b010000;
        r6 = 0b100000;
        r7 = 0b111110;
        break;
    case 0x33: // 3
        r1 = 0b111110;
        r2 = 0b000010;
        r3 = 0b000100;
        r4 = 0b001100;
        r5 = 0b000010;
        r6 = 0b100010;
        r7 = 0b011100;
        break;
    case 0x34: // 4
        r1 = 0b000100;
        r2 = 0b001100;
        r3 = 0b010100;
        r4 = 0b100100;
        r5 = 0b111110;
        r6 = 0b000100;
        r7 = 0b000100;
        break;
    case 0x35: // 5
        r1 = 0b111110;
        r2 = 0b100000;
        r3 = 0b111100;
        r4 = 0b000010;
        r5 = 0b000010;
        r6 = 0b100010;
        r7 = 0b011100;
        break;
    case 0x36: // 6
        r1 = 0b001110;
        r2 = 0b010000;
        r3 = 0b100000;
        r4 = 0b111100;
        r5 = 0b100010;
        r6 = 0b100010;
        r7 = 0b011100;
        break;
    case 0x37: // 7
        r1 = 0b111110;
        r2 = 0b000010;
        r3 = 0b000100;
        r4 = 0b001000;
        r5 = 0b010000;
        r6 = 0b010000;
        r7 = 0b010000;
        break;
    case 0x38: // 8
        r1 = 0b011100;
        r2 = 0b100010;
        r3 = 0b100010;
        r4 = 0b011100;
        r5 = 0b100010;
        r6 = 0b100010;
        r7 = 0b011100;
        break;
    case 0x39: // 9
        r1 = 0b011100;
        r2 = 0b100010;
        r3 = 0b100010;
        r4 = 0b011110;
        r5 = 0b000010;
        r6 = 0b000100;
        r7 = 0b111000;
        break;
    case 0x3A: // :
        r3 = 0b001000;
        r5 = 0b001000;
        break;
    case 0x3B: // ;
        r3 = 0b001000;
        r5 = 0b001000;
        r6 = 0b001000;
        r7 = 0b010000;
        break;
    case 0x3C: // <
        r1 = 0b000010;
        r2 = 0b000100;
        r3 = 0b001000;
        r4 = 0b010000;
        r5 = 0b001000;
        r6 = 0b000100;
        r7 = 0b000010;
        break;
    case 0x3D: // =
        r3 = 0b111100;
        r5 = 0b111100;
        break;
    case 0x3E: // >
        r1 = 0b100000;
        r2 = 0b010000;
        r3 = 0b001000;
        r4 = 0b000100;
        r5 = 0b001000;
        r6 = 0b010000;
        r7 = 0b100000;
        break;
    case 0x3F: // ?
        r1 = 0b011100;
        r2 = 0b100010;
        r3 = 0b000100;
        r4 = 0b001000;
        r5 = 0b001000;
        r7 = 0b001000;
        break;
    default:
        break;
    }

    if(page == 1)
    {
        lopage1[startAt] = flag & INVERSE ? ~r0 : r0;
        lopage1[startAt+40] = flag & INVERSE ? ~r1 : r1;
        lopage1[startAt+80] = flag & INVERSE ? ~r2 : r2;
        lopage1[startAt+120] = flag & INVERSE ? ~r3 : r3;
        lopage1[startAt+160] = flag & INVERSE ? ~r4 : r4;
        lopage1[startAt+200] = flag & INVERSE ? ~r5 : r5;
        lopage1[startAt+240] = flag & INVERSE ? ~r6 : r6;
        lopage1[startAt+280] = flag & INVERSE ? ~r7 : r7;
    }
    else
    {
        lopage2[startAt] = flag & INVERSE ? ~r0 : r0;
        lopage2[startAt+40] = flag & INVERSE ? ~r1 : r1;
        lopage2[startAt+80] = flag & INVERSE ? ~r2 : r2;
        lopage2[startAt+120] = flag & INVERSE ? ~r3 : r3;
        lopage2[startAt+160] = flag & INVERSE ? ~r4 : r4;
        lopage2[startAt+200] = flag & INVERSE ? ~r5 : r5;
        lopage2[startAt+240] = flag & INVERSE ? ~r6 : r6;
        lopage2[startAt+280] = flag & INVERSE ? ~r7 : r7;
    }
}

/**
 * interprets bytes for character and mode to display
 */
static void decodeChars()
{
    // The screen is split up into 3 sections of 8 rows each
    // going through the page is like dealing 24 cards to 3 players, but each card is one 40-character row
    // and the last 8 bytes of every 0x80 byte section is ignored
    for(unsigned short base = 0x0; base < 0x400; base += 0x80)
    {
        for(unsigned short ch = 0; ch < 0x28; ch++)
        {
            charFlags[base + ch] = 0;
            if(characters[base + ch] < 0x40)
            {
                charFlags[base + ch] |= INVERSE;
            }
            else if(characters[base + ch] < 0x80)
            {
                charFlags[base + ch] |= FLASHING;
                if(!showing)
                {
                    characters[base + ch] = 0xA0;
                }
            }

            insertChar(screenFlags & pri ? 1 : 2, ((base) * 5) / 2 + ch, (char) (characters[base + ch] % 0x40), charFlags[base + ch]);
        }
    }
    for(unsigned short base = 0x28; base < 0x400; base += 0x80)
    {
        for(unsigned short ch = 0; ch < 0x28; ch++)
        {
            charFlags[base + ch] = 0;
            if(characters[base + ch] < 0x40)
            {
                charFlags[base + ch] |= INVERSE;
            }
            else if(characters[base + ch] < 0x80)
            {
                charFlags[base + ch] |= FLASHING;
                if(!showing)
                {
                    characters[base + ch] = 0xA0;
                }
            }

            insertChar(screenFlags & pri ? 1 : 2, ((base - 0x28) * 5) / 2 + ch + 2560, (char) (characters[base + ch] % 0x40), charFlags[base + ch]);
        }
    }
    for(unsigned short base = 0x50; base < 0x400; base += 0x80)
    {
        for(unsigned short ch = 0; ch < 0x28; ch++)
        {
            charFlags[base + ch] = 0;
            if(characters[base + ch] < 0x40)
            {
                charFlags[base + ch] |= INVERSE;
            }
            else if(characters[base + ch] < 0x80)
            {
                charFlags[base + ch] |= FLASHING;
                if(!showing)
                {
                    characters[base + ch] = 0xA0;
                }
            }

            insertChar(screenFlags & pri ? 1 : 2, ((base - 0x50) * 5) / 2 + ch + 5120, (char) (characters[base + ch] % 0x40), charFlags[base + ch]);
        }
    }
}

/**
 * fills video memory with data depending on screen page contents
 */
static void textMode()
{
    WaitForSingleObject(memMutex, INFINITE);
    WaitForSingleObject(screenMutex, INFINITE);

    if(screenFlags & pri)
    {
        memcpy(characters, memory + 0x400, 0x400);
        memcpy(saveChars, memory + 0x400, 0x400);
    }
    else
    {
        memcpy(characters, memory + 0x800, 0x400);
        memcpy(saveChars, memory + 0x800, 0x400);
    }

    decodeChars();

    ReleaseMutex(memMutex);
    ReleaseMutex(screenMutex);
}

/**
 * Interprets screen pages in lo-res mode
 */
static void loResMode()
{

}

/**
 * Interprets screen pages in hi-res mode
 */
static void hiResMode()
{

}

/**
 * draws contents of the video memory onto the screen
 */
static void drawScreen()
{
    WaitForSingleObject(screenMutex, INFINITE);
    if(!(screenFlags & gr))
    {
        for(unsigned short byte = 0; byte < 7680; byte++)
        {
            for(int shift = 0; shift < 8; shift++)
            {
                // leftmost pixel is bit 6
                if(((screenFlags & pri ? lopage1[byte] : lopage2[byte]) & (0b10000000 >> shift)) != 0)
                {
                    // TODO maybe revisit this
                    int leftCoord = (((byte * 7 + shift) % 280) * 2);
                    int upCoord = (((byte * 7 + shift) / 280) * 2);
                    glRectf(((leftCoord - 2) / 280.0f) - 1, -(((upCoord) / 192.0f) - 1), (leftCoord) / 280.0f - 1,
                            -((upCoord + 2) / 192.0f - 1));
                }
            }
        }
    }
    ReleaseMutex(screenMutex);
}

/**
 * handles a key press for resetting and shutting down emulator
 * @param window window in focus
 * @param key key that was presses
 * @param scancode unused
 * @param action the state of the keypress
 * @param mods unused
 */
static void keyInput(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(action == GLFW_REPEAT || action == GLFW_RELEASE)
    {
        return;
    }

    switch(key)
    {
    case GLFW_KEY_F1:
    case GLFW_KEY_F2:
        WaitForSingleObject(runningMutex, INFINITE);
        running = 0;
        ReleaseMutex(runningMutex);
        break;
    case GLFW_KEY_F3:
        setRecord();
        return;
    case GLFW_KEY_F4:
        setPlay();
        return;
    case GLFW_KEY_ESCAPE:
        WaitForSingleObject(memMutex, INFINITE);
        memset(memory + 0xC000, 0x9B, 16);
        ReleaseMutex(memMutex);
        return;
    case GLFW_KEY_ENTER:
        WaitForSingleObject(memMutex, INFINITE);
        memset(memory + 0xC000, 0x8D, 16);
        ReleaseMutex(memMutex);
        return;
    case GLFW_KEY_LEFT:
        WaitForSingleObject(memMutex, INFINITE);
        memset(memory + 0xC000, 0x95, 16);
        ReleaseMutex(memMutex);
        return;
    case GLFW_KEY_RIGHT:
        WaitForSingleObject(memMutex, INFINITE);
        memset(memory + 0xC000, 0x88, 16);
        ReleaseMutex(memMutex);
        return;
    // N and P are the only 2 letter keys with other characters when holding shift
    case GLFW_KEY_N:
    {
        unsigned char data;
        if(mods & GLFW_MOD_CONTROL)
        {
            data = (unsigned char) (mods & GLFW_MOD_SHIFT ? 0x9E : 0x8E);
        }
        else
        {
            data = 0xCE;
        }
        memset(memory + 0xC000, data, 16);
    }
        return;
    case GLFW_KEY_P:
    {
        unsigned char data;
        if(mods & GLFW_MOD_CONTROL)
        {
            data = (unsigned char) (mods & GLFW_MOD_SHIFT ? 0x80 : 0x90);
        }
        else
        {
            data = 0xD0;
        }
        memset(memory + 0xC000, data, 16);
    }
        return;
    default:
        break;
    }

    if(key == GLFW_KEY_F1)
    {
        reset();
        return;
    }

    if(key >= GLFW_KEY_A && key <= GLFW_KEY_Z && mods & GLFW_MOD_CONTROL)
    {
        memset(memory + 0xC000, key - 0xC0, 16);
    }
}

/**
 * handles window close event
 * @param window the window to close
 */
static void closeWindow(GLFWwindow *window)
{

    WaitForSingleObject(runningMutex, INFINITE);
    running = 0;
    ReleaseMutex(runningMutex);
    glfwTerminate();
}

static void textInput(GLFWwindow *window, unsigned int codepoint)
{
    // not on the original keyboard
    switch(codepoint)
    {
    case '_':
    case '\\':
    case '|':
    case '~':
    case '`':
        return;
    default:
        break;
    }
    // only uppercase letters in the original
    if(isalpha(codepoint))
    {
        codepoint = (unsigned int) toupper(codepoint);
        if(codepoint == 'P' || codepoint == 'N')
        {
            return;
        }
    }

    WaitForSingleObject(memMutex, INFINITE);
    memset(memory + 0xC000, codepoint | 0b10000000, 16);
    ReleaseMutex(memMutex);
}

int startGLFW()
{
    if(!glfwInit())
    {
        return 1;
    }

    window = glfwCreateWindow(560, 384, "Apple ][", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return 2;
    }

    glfwMakeContextCurrent(window);

    //glfwSetWindowCloseCallback(window, closeWindow);
    glfwSetKeyCallback(window, keyInput);
    glfwSetCharCallback(window, textInput);
    glfwSwapInterval(1);

    fillBuffer = textMode;

    // check if the CPU already finished before we even got here so we draw at least once
    WaitForSingleObject(runningMutex, INFINITE);
    char alreadyDone = !running;
    ReleaseMutex(runningMutex);

    while (!glfwWindowShouldClose(window))
    {
        WaitForSingleObject(runningMutex, INFINITE);
        if(running || alreadyDone)
        {
            ReleaseMutex(runningMutex);
            glClear(GL_COLOR_BUFFER_BIT);

            // check if display mode has been changed
            WaitForSingleObject(screenMutex, INFINITE);
            if(screenFlagsChanged)
            {
                if(screenFlags & gr)
                {
                    fillBuffer = screenFlags & lores ? loResMode : hiResMode;
                }
                else
                {
                    fillBuffer = textMode;
                }
            }
            ReleaseMutex(screenMutex);

            fillBuffer();

            flashTimer--;
            if(!flashTimer)
            {
                flashTimer = 30;
                showing = (char) 1 - showing;
            }

            drawScreen();
            glfwSwapBuffers(window);
            glfwPollEvents();

            if(alreadyDone)
            {
                alreadyDone = 0;
            }
        }
        else
        {
            ReleaseMutex(runningMutex);
            glfwWaitEvents();
        }
    }
    return 0;
}