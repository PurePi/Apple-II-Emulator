#include <HIF.h>
#include <memory.h>
#include <GLFW/glfw3.h>
#include <mem.h>
#include <stdio.h>
#include <process.h>
#include <cpu.h>

char running = 0;
char screenFlagsChanged = 0;
char screenFlags = 0b0100;

unsigned short pageBase = 0x0400;
unsigned short pageEnd = 0x0BFF;

static GLFWwindow *window;

// the bytes to draw on the screen
static char lpage1[7680] = { 0 };
static char lpage2[7680] = { 0 };
static char hpage1[7680] = { 0 };
static char hpage2[7680] = { 0 };

static void (*fillBuffer)();

/**
 * inserts a single character into the video memory
 * @param page 1 = page 1, 2 = page 2
 */
static void insertChar(char page, int startAt, char character)
{
    char r0 = 0, r1 = 0, r2 = 0, r3 = 0, r4 = 0,
            r5 = 0, r6 = 0, r7 = 0;

    switch(character)
    {
    case '\t':
        r1 = 0b01111110;
        r2 = 0b01111110;
        r3 = 0b01111110;
        r4 = 0b01111110;
        r5 = 0b01111110;
        r6 = 0b01111110;
        break;
    case '!':
        r1 = 0b00001000;
        r2 = 0b00001000;
        r3 = 0b00001000;
        r4 = 0b00001000;
        r6 = 0b00001000;
        break;
    case '"':
        r1 = 0b00010100;
        r2 = 0b00010100;
        break;
    case '#':
        r1 = 0b00100100;
        r2 = 0b01111111;
        r3 = 0b00100100;
        r4 = 0b00100100;
        r5 = 0b01111110;
        r6 = 0b00100100;
        break;
    case '$':
        r1 = 0b00001000;
        r2 = 0b00011110;
        r3 = 0b00101000;
        r4 = 0b00011100;
        r5 = 0b00010100;
        r6 = 0b00111100;
        r7 = 0b00001000;
        break;
    case '%':
        r1 = 0b01100010;
        r2 = 0b10010100;
        r3 = 0b01101000;
        r4 = 0b00010110;
        r5 = 0b00101001;
        r6 = 0b01000110;
        break;
    case '&':
        r1 = 0b00111000;
        r2 = 0b01000000;
        r3 = 0b01000000;
        r4 = 0b00110010;
        r5 = 0b01001100;
        r6 = 0b00110010;
        break;
    case '\'':
        r1 = 0b00001000;
        r2 = 0b00001000;
        break;
    case '(':
        r1 = 0b00010000;
        r2 = 0b00100000;
        r3 = 0b01000000;
        r4 = 0b01000000;
        r5 = 0b00100000;
        r6 = 0b00010000;
        break;
    case ')':
        r1 = 0b00001000;
        r2 = 0b00000100;
        r3 = 0b00000010;
        r4 = 0b00000010;
        r5 = 0b00000100;
        r6 = 0b00001000;
        break;
    case '*':
        r1 = 0b00101010;
        r2 = 0b00011100;
        r3 = 0b00111110;
        r4 = 0b00011100;
        r5 = 0b00101010;
        break;
    case '+':
        r1 = 0b00001000;
        r2 = 0b00001000;
        r3 = 0b00111110;
        r4 = 0b00001000;
        r5 = 0b00001000;
        break;
    case ',':
        r6 = 0b00010000;
        r7 = 0b00100000;
        break;
    case '-':
        r3 = 0b00111100;
        break;
    case '.':
        r6 = 0b00001000;
        break;
    case '/':
        r1 = 0b00000010;
        r2 = 0b00000100;
        r3 = 0b00001000;
        r4 = 0b00010000;
        r5 = 0b00100000;
        r6 = 0b01000000;
        break;
    case '0':
        r1 = 0b00111100;
        r2 = 0b01000110;
        r3 = 0b01001010;
        r4 = 0b01010010;
        r5 = 0b01100010;
        r6 = 0b00111100;
        break;
    case '1':
        r1 = 0b00001000;
        r2 = 0b00011000;
        r3 = 0b00001000;
        r4 = 0b00001000;
        r5 = 0b00001000;
        r6 = 0b00011100;
        break;
    case '2':
        r1 = 0b00111100;
        r2 = 0b01000010;
        r3 = 0b00000100;
        r4 = 0b00011000;
        r5 = 0b00100000;
        r6 = 0b01111110;
        break;
    case '3':
        r1 = 0b00111100;
        r2 = 0b01000010;
        r3 = 0b00000100;
        r4 = 0b00011000;
        r5 = 0b00000100;
        r6 = 0b00000100;
        r7 = 0b01111000;
        break;
    case '4':
        r1 = 0b00001100;
        r2 = 0b00010100;
        r3 = 0b00100100;
        r4 = 0b01000100;
        r5 = 0b01111110;
        r6 = 0b00000100;
        break;
    case '5':
        r1 = 0b01111110;
        r2 = 0b01000000;
        r3 = 0b01111100;
        r4 = 0b00000010;
        r5 = 0b01000010;
        r6 = 0b00111100;
        break;
    case '6':
        r1 = 0b00111110;
        r2 = 0b01000000;
        r3 = 0b01111100;
        r4 = 0b01000010;
        r5 = 0b01000010;
        r6 = 0b00111100;
        break;
    case '7':
        r1 = 0b01111110;
        r2 = 0b00000100;
        r3 = 0b00001000;
        r4 = 0b00010000;
        r5 = 0b00100000;
        r6 = 0b01000000;
        break;
    case '8':
        r1 = 0b00111100;
        r2 = 0b01000010;
        r3 = 0b00111100;
        r4 = 0b01000010;
        r5 = 0b01000010;
        r6 = 0b00111100;
        break;
    case '9':
        r1 = 0b00111100;
        r2 = 0b01000010;
        r3 = 0b01000010;
        r4 = 0b00111110;
        r5 = 0b00000010;
        r6 = 0b00111100;
        break;
    case ':':
        r2 = 0b00010000;
        r6 = 0b00010000;
        break;
    case ';':
        r2 = 0b00010000;
        r6 = 0b00010000;
        r7 = 0b00010000;
        break;
    case '<':
        r2 = 0b00000110;
        r3 = 0b00011000;
        r4 = 0b01100000;
        r5 = 0b00011000;
        r6 = 0b00000110;
        break;
    case '=':
        r3 = 0b00111100;
        r5 = 0b00111100;
        break;
    case '>':
        r2 = 0b01100000;
        r3 = 0b00011000;
        r4 = 0b00000110;
        r5 = 0b00011000;
        r6 = 0b01100000;
        break;
    case '?':
        r1 = 0b00111000;
        r2 = 0b01000100;
        r3 = 0b00000100;
        r4 = 0b00011000;
        r6 = 0b00010000;
        break;
    case '@':
        r1 = 0b00011110;
        r2 = 0b00100010;
        r3 = 0b01001110;
        r4 = 0b01001010;
        r5 = 0b00101110;
        r6 = 0b00010000;
        r7 = 0b00001110;
        break;
    case 'A':
        r1 = 0b00011000;
        r2 = 0b00011000;
        r3 = 0b00100100;
        r4 = 0b00111100;
        r5 = 0b01000010;
        r6 = 0b01000010;
        break;
    case 'B':
        r1 = 0b01111100;
        r2 = 0b01000010;
        r3 = 0b01111100;
        r4 = 0b01000010;
        r5 = 0b01000010;
        r6 = 0b01111100;
        break;
    case 'C':
        r1 = 0b00011100;
        r2 = 0b00100010;
        r3 = 0b01000000;
        r4 = 0b01000000;
        r5 = 0b00100010;
        r6 = 0b00011100;
        break;
    case 'D':
        r1 = 0b01111100;
        r2 = 0b01000010;
        r3 = 0b01000010;
        r4 = 0b01000010;
        r5 = 0b01000010;
        r6 = 0b01111100;
        break;
    case 'E':
        r1 = 0b01111110;
        r2 = 0b01000000;
        r3 = 0b01111100;
        r4 = 0b01000000;
        r5 = 0b01000000;
        r6 = 0b01111110;
        break;
    case 'F':
        r1 = 0b01111110;
        r2 = 0b01000000;
        r3 = 0b01111100;
        r4 = 0b01000000;
        r5 = 0b01000000;
        r6 = 0b01000000;
        break;
    case 'G':
        r1 = 0b00011100;
        r2 = 0b00100010;
        r3 = 0b01000000;
        r4 = 0b01001110;
        r5 = 0b00100010;
        r6 = 0b00011100;
        break;
    case 'H':
        r1 = 0b01000010;
        r2 = 0b01000010;
        r3 = 0b01111110;
        r4 = 0b01000010;
        r5 = 0b01000010;
        r6 = 0b01000010;
        break;
    case 'I':
        r1 = 0b00111110;
        r2 = 0b00001000;
        r3 = 0b00001000;
        r4 = 0b00001000;
        r5 = 0b00001000;
        r6 = 0b00111110;
        break;
    case 'J':
        r1 = 0b00001110;
        r2 = 0b00000010;
        r3 = 0b00000010;
        r4 = 0b00000010;
        r5 = 0b00100010;
        r6 = 0b00011100;
        break;
    case 'K':
        r1 = 0b01000010;
        r2 = 0b01000100;
        r3 = 0b01001000;
        r4 = 0b01111000;
        r5 = 0b01000100;
        r6 = 0b01000010;
        break;
    case 'L':
        r1 = 0b01000000;
        r2 = 0b01000000;
        r3 = 0b01000000;
        r4 = 0b01000000;
        r5 = 0b01000000;
        r6 = 0b01111100;
        break;
    case 'M':
        r1 = 0b01000010;
        r2 = 0b01100110;
        r3 = 0b01011010;
        r4 = 0b01000010;
        r5 = 0b01000010;
        r6 = 0b01000010;
        break;
    case 'N':
        r1 = 0b01000010;
        r2 = 0b01100010;
        r3 = 0b01010010;
        r4 = 0b01001010;
        r5 = 0b01000110;
        r6 = 0b01000010;
        break;
    case 'O':
        r1 = 0b00011100;
        r2 = 0b00100010;
        r3 = 0b00100010;
        r4 = 0b00100010;
        r5 = 0b00100010;
        r6 = 0b00011100;
        break;
    case 'P':
        r1 = 0b01111100;
        r2 = 0b01000010;
        r3 = 0b01111100;
        r4 = 0b01000000;
        r5 = 0b01000000;
        r6 = 0b01000000;
        break;
    case 'Q':
        r1 = 0b00011100;
        r2 = 0b00100010;
        r3 = 0b00100010;
        r4 = 0b00100010;
        r5 = 0b00100010;
        r6 = 0b00011100;
        r7 = 0b00000010;
        break;
    case 'R':
        r1 = 0b01111100;
        r2 = 0b01000010;
        r3 = 0b01111100;
        r4 = 0b01001000;
        r5 = 0b01000100;
        r6 = 0b01000010;
        break;
    case 'S':
        r1 = 0b00111100;
        r2 = 0b01000010;
        r3 = 0b00100000;
        r4 = 0b00011100;
        r5 = 0b01000010;
        r6 = 0b00111100;
        break;
    case 'T':
        r1 = 0b00111110;
        r2 = 0b00001000;
        r3 = 0b00001000;
        r4 = 0b00001000;
        r5 = 0b00001000;
        r6 = 0b00001000;
        break;
    case 'U':
        r1 = 0b00100010;
        r2 = 0b00100010;
        r3 = 0b00100010;
        r4 = 0b00100010;
        r5 = 0b00100010;
        r6 = 0b00011100;
        break;
    case 'V':
        r1 = 0b01000010;
        r2 = 0b01000010;
        r3 = 0b00100100;
        r4 = 0b00100100;
        r5 = 0b00011000;
        r6 = 0b00011000;
        break;
    case 'W':
        r1 = 0b01000010;
        r2 = 0b01000010;
        r3 = 0b01000010;
        r4 = 0b01011010;
        r5 = 0b01100110;
        r6 = 0b01000010;
        break;
    case 'X':
        r1 = 0b01000010;
        r2 = 0b00100100;
        r3 = 0b00011000;
        r4 = 0b00011000;
        r5 = 0b00100100;
        r6 = 0b01000010;
        break;
    case 'Y':
        r0 = 0b01000001;
        r1 = 0b00100010;
        r2 = 0b00010100;
        r3 = 0b00001000;
        r4 = 0b00001000;
        r5 = 0b00001000;
        r6 = 0b00001000;
        break;
    case 'Z':
        r1 = 0b01111110;
        r2 = 0b00000100;
        r3 = 0b00001000;
        r4 = 0b00010000;
        r5 = 0b00100000;
        r6 = 0b01111110;
        break;
    case '[':
        r1 = 0b01110000;
        r2 = 0b01000000;
        r3 = 0b01000000;
        r4 = 0b01000000;
        r5 = 0b01000000;
        r6 = 0b01110000;
        break;
    case '\\':
        r1 = 0b01000000;
        r2 = 0b00100000;
        r3 = 0b00010000;
        r4 = 0b00001000;
        r5 = 0b00000100;
        r6 = 0b00000010;
        break;
    case ']':
        r1 = 0b00001110;
        r2 = 0b00000010;
        r3 = 0b00000010;
        r4 = 0b00000010;
        r5 = 0b00000010;
        r6 = 0b00001110;
        break;
    case '^':
        r1 = 0b00001000;
        r2 = 0b00010100;
        break;
    case '_':
        r6 = 0b01111110;
        break;
    case '`':
        r0 = 0b00010000;
        r1 = 0b00001000;
        break;
    case 'a':
        r2 = 0b00111100;
        r3 = 0b00000010;
        r4 = 0b00111110;
        r5 = 0b01000110;
        r6 = 0b00111010;
        break;
    case 'b':
        r1 = 0b01000000;
        r2 = 0b01000000;
        r3 = 0b01111100;
        r4 = 0b01000010;
        r5 = 0b01100010;
        r6 = 0b01011100;
        break;
    case 'c':
        r3 = 0b00011100;
        r4 = 0b00100000;
        r5 = 0b00100000;
        r6 = 0b00011100;
        break;
    case 'd':
        r1 = 0b00000010;
        r2 = 0b00000010;
        r3 = 0b00111110;
        r4 = 0b01000010;
        r5 = 0b01000110;
        r6 = 0b00111010;
        break;
    case 'e':
        r2 = 0b00111100;
        r3 = 0b01000010;
        r4 = 0b01111110;
        r5 = 0b01000000;
        r6 = 0b00111100;
        break;
    case 'f':
        r1 = 0b00001100;
        r2 = 0b00001000;
        r3 = 0b00011100;
        r4 = 0b00001000;
        r5 = 0b00001000;
        r6 = 0b00001000;
        break;
    case 'g':
        r2 = 0b00011010;
        r3 = 0b00100110;
        r4 = 0b00100010;
        r5 = 0b00011010;
        r6 = 0b00000010;
        r7 = 0b00011100;
        break;
    case 'h':
        r1 = 0b00100000;
        r2 = 0b00100000;
        r3 = 0b00111000;
        r4 = 0b00100100;
        r5 = 0b00100100;
        r6 = 0b00100100;
        break;
    case 'i':
        r1 = 0b00001000;
        r3 = 0b00001000;
        r4 = 0b00001000;
        r5 = 0b00001000;
        r6 = 0b00001000;
        break;
    case 'j':
        r1 = 0b00000100;
        r2 = 0b00001100;
        r3 = 0b00000100;
        r4 = 0b00000100;
        r5 = 0b00100010;
        r6 = 0b00011000;
        break;
    case 'k':
        r1 = 0b00100000;
        r2 = 0b00100000;
        r3 = 0b00100100;
        r4 = 0b00101000;
        r5 = 0b00110000;
        r6 = 0b00101100;
        break;
    case 'l':
        r1 = 0b00010000;
        r2 = 0b00010000;
        r3 = 0b00010000;
        r4 = 0b00010000;
        r5 = 0b00010000;
        r6 = 0b00011000;
        break;
    case 'm':
        r3 = 0b01100110;
        r4 = 0b01011010;
        r5 = 0b01000010;
        r6 = 0b01000010;
        break;
    case 'n':
        r3 = 0b00101110;
        r4 = 0b00110010;
        r5 = 0b00100010;
        r6 = 0b00100010;
        break;
    case 'o':
        r3 = 0b00111100;
        r4 = 0b01000010;
        r5 = 0b01000010;
        r6 = 0b00111100;
        break;
    case 'p':
        r2 = 0b01011100;
        r3 = 0b01100010;
        r4 = 0b01000010;
        r5 = 0b01111100;
        r6 = 0b01000000;
        r7 = 0b01000000;
        break;
    case 'q':
        r2 = 0b00111010;
        r3 = 0b01000110;
        r4 = 0b01000010;
        r5 = 0b00111110;
        r6 = 0b00000010;
        r7 = 0b00000010;
        break;
    case 'r':
        r3 = 0b00101100;
        r4 = 0b00110010;
        r5 = 0b00100000;
        r6 = 0b00100000;
        break;
    case 's':
        r2 = 0b00011110;
        r3 = 0b00100000;
        r4 = 0b00011100;
        r5 = 0b00000010;
        r6 = 0b00111100;
        break;
    case 't':
        r1 = 0b00010000;
        r2 = 0b00111100;
        r3 = 0b00010000;
        r4 = 0b00010000;
        r5 = 0b00010000;
        r6 = 0b00011000;
        break;
    case 'u':
        r3 = 0b00100010;
        r4 = 0b00100010;
        r5 = 0b00100110;
        r6 = 0b00011010;
        break;
    case 'v':
        r3 = 0b01000010;
        r4 = 0b01000010;
        r5 = 0b00100100;
        r6 = 0b00011000;
        break;
    case 'w':
        r3 = 0b01000010;
        r4 = 0b01000010;
        r5 = 0b01011010;
        r6 = 0b00100100;
        break;
    case 'x':
        r3 = 0b01000010;
        r4 = 0b00100100;
        r5 = 0b00011000;
        r6 = 0b01100110;
        break;
    case 'y':
        r2 = 0b01000010;
        r3 = 0b00100010;
        r4 = 0b00010100;
        r5 = 0b00001000;
        r6 = 0b00010000;
        r7 = 0b01100000;
        break;
    case 'z':
        r3 = 0b00111100;
        r4 = 0b00001000;
        r5 = 0b00010000;
        r6 = 0b00111100;
        break;
    case '{':
        r1 = 0b00011100;
        r2 = 0b00010000;
        r3 = 0b00110000;
        r4 = 0b00110000;
        r5 = 0b00010000;
        r6 = 0b00011100;
        break;
    case '|':
        r0 = 0b00001000;
        r1 = 0b00001000;
        r2 = 0b00001000;
        r3 = 0b00001000;
        r4 = 0b00001000;
        r5 = 0b00001000;
        r6 = 0b00001000;
        r7 = 0b00001000;
        break;
    case '}':
        r1 = 0b00111000;
        r2 = 0b00001000;
        r3 = 0b00001100;
        r4 = 0b00001100;
        r5 = 0b00001000;
        r6 = 0b00111000;
        break;
    case '~':
        r4 = 0b00110010;
        r5 = 0b01001100;
        break;
    default:
        break;
    }

    if(page == 1)
    {
        lpage1[startAt] = r0;
        lpage1[startAt+40] = r1;
        lpage1[startAt+80] = r2;
        lpage1[startAt+120] = r3;
        lpage1[startAt+160] = r4;
        lpage1[startAt+200] = r5;
        lpage1[startAt+240] = r6;
        lpage1[startAt+280] = r7;
    }
    else
    {
        lpage2[startAt] = r0;
        lpage2[startAt+40] = r1;
        lpage2[startAt+80] = r2;
        lpage2[startAt+120] = r3;
        lpage2[startAt+160] = r4;
        lpage2[startAt+200] = r5;
        lpage2[startAt+240] = r6;
        lpage2[startAt+280] = r7;
    }
}

// TODO decode hex into ascii and display mode before placing onto page memory

/**
 * fills video memory with data depending on screen page contents
 */
static void textMode()
{
    WaitForSingleObject(memMutex, INFINITE);
    if(screenFlags & pri)
    {
        for(unsigned short base = 0x400; base < 0x800; base += 0x80)
        {
            for(unsigned short ch = 0; ch < 0x28; ch++)
            {
                insertChar(1, ((base - 0x400) * 5) / 2 + ch, memory[base + ch]);
            }
        }
        for(unsigned short base = 0x428; base < 0x800; base += 0x80)
        {
            for(unsigned short ch = 0; ch < 0x28; ch++)
            {
                insertChar(1, ((base - 0x428) * 5) / 2 + ch + 3840, memory[base + ch]);
            }
        }
    }
    else
    {
        for(unsigned short base = 0x800; base < 0xC00; base += 0x80)
        {
            for(unsigned short ch = 0; ch < 0x28; ch++)
            {
                insertChar(2, ((base - 0x800) * 5) / 2 + ch, memory[base + ch]);
            }
        }
        for(unsigned short base = 0x828; base < 0xC00; base += 0x80)
        {
            for(unsigned short ch = 0; ch < 0x28; ch++)
            {
                insertChar(2, ((base - 0x828) * 5) / 2 + ch + 3840, memory[base + ch]);
            }
        }
    }

    ReleaseMutex(memMutex);
}

// TODO add more graphics modes

/**
 * draws contents of the video memory onto the screen
 */
static void drawScreen()
{
    if(!(screenFlags & gr))
    {
        for(unsigned short byte = 0; byte < 7680; byte++)
        {
            for(int shift = 0; shift < 8; shift++)
            {
                // leftmost pixel is MSb
                if(((screenFlags & pri ? lpage1[byte] : lpage2[byte]) & (0b10000000 >> shift)) != 0)
                {
                    //int leftCoord =

                    // TODO fix drawing coordinates
                    int leftCoord = (((byte * 7 + shift) % 280) * 2);
                    int upCoord = (((byte * 7 + shift) / 280) * 2);
                    glRectf((leftCoord / 280.0f) - 1, -((upCoord / 192.0f) - 1), (leftCoord + 2) / 280.0f - 1,
                            -((upCoord + 2) / 192.0f - 1));
                }
            }
        }
    }
}

/**
 * handles a key press for resetting and shutting down emulator
 * @param window window in focus
 * @param key key that was presses
 * @param scancode unused
 * @param action unused
 * @param mods unused
 */
static void keyInput(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(action == GLFW_REPEAT || action == GLFW_RELEASE)
    {
        return;
    }

    // tab is not handled by text input
    if(key == GLFW_KEY_TAB)
    {
        WaitForSingleObject(memMutex, INFINITE);
        memset(memory + 0xC000, '\t' | 0b10000000, 16);
        ReleaseMutex(memMutex);
        return;
    }

    char released = 0;
    WaitForSingleObject(runningMutex, INFINITE);
    if(key == GLFW_KEY_F1)
    {
        if(cpuThread != 0)
        {
            printf("resetting CPU\n");
            running = 0;
        }
        ReleaseMutex(runningMutex);
        released = 1;
        reset();
    }
    else if(key == GLFW_KEY_F2 && cpuThread != 0)
    {
        printf("shutting down CPU\n");
        running = 0;
        // TODO figure out shutting everything down
    }
    if(!released)
    {
        ReleaseMutex(runningMutex);
    }


}

static void textInput(GLFWwindow *window, unsigned int codepoint)
{
    char data = (char) codepoint;

    WaitForSingleObject(memMutex, INFINITE);
    memset(memory + 0xC000, data | 0b10000000, 16);
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

    glfwSetKeyCallback(window, keyInput);
    glfwSetCharCallback(window, textInput);

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

            fillBuffer();
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
