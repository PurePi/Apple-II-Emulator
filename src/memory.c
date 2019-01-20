#include <memory.h>
#include <HIF.h>
#include <stdio.h>

unsigned char memory[0x10000];

// for accessing memory
HANDLE memMutex;
// for telling the GLFW thread that the contents of one of the screen pages or flags has changed
HANDLE screenMutex;
// for telling GLFW the CPU if the CLU is running or not
HANDLE runningMutex;

/**
 * interacts with hardware components if memory-mapped IO has been referenced
 *
 * emulates the function of the I/O selector
 */
static void ioSelect(unsigned short address)
{
    if(address >= 0xC010 && address < 0xD000)
    {
        if(address < 0xC020)
        {
            // clear keyboard strobe
            memset(memory + 0xC000, memory[0xC000] & 0b01111111, 16);
        }
        else if(address < 0xC030)
        {
            // cassette output toggle
        }
        else if(address < 0xC040)
        {
            // speaker toggle
        }
        else if(address < 0xC050)
        {
            // utility strobe
        }
        else if(address < 0xC058)
        {
            WaitForSingleObject(screenMutex, INFINITE);
            screenFlagsChanged = 1;
            switch(address)
            {
            case 0xC050: // set graphics mode
                screenFlags |= gr;
                break;
            case 0xC051: // set text mode
                screenFlags &= ~gr;
                break;
            case 0xC052: // set all text or graphics
                screenFlags &= ~mix;
                break;
            case 0xC053: // mix text and graphics
                screenFlags |= mix;
                break;
            case 0xC054: // display primary page
                screenFlags |= pri;
                break;
            case 0xC055: // display secondary page
                screenFlags &= ~pri;
                break;
            case 0xC056: // set low-res graphics
                screenFlags |= lores;
                break;
            case 0xC057: // set hi-res graphics
                screenFlags &= ~lores;
                break;
            default:
                break;
            }
            ReleaseMutex(screenMutex);
        }
        else if(address < 0xC060)
        {
            switch(address)
            {
            case 0xC058:
            case 0xC059: // annunciator 0

                break;
            case 0xC05A:
            case 0xC05B: // annunciator 1

                break;
            case 0xC05C:
            case 0xC05D: // annunciator 2

                break;
            case 0xC05E:
            case 0xC05F: // annunciator 3

                break;
            default:
                break;
            }
        }
        else if(address < 0xC070)
        {
            switch(address)
            {
            case 0xC060:
            case 0xC068: // cassette input

                break;
            case 0xC061:
            case 0xC069: // pushbutton 1

                break;
            case 0xC062:
            case 0xC06A: // pushbutton 2

                break;
            case 0xC063:
            case 0xC06B: // pushbutton 3

                break;
            case 0xC064:
            case 0xC06C: // game controller input 0

                break;
            case 0xC065:
            case 0xC06D: // game controller input 1

                break;
            case 0xC066:
            case 0xC06E: // game controller input 2

                break;
            case 0xC067:
            case 0xC06F: // game controller input 3

                break;
            default:
                break;
            }
        }
        else if(address < 0xC080)
        {
            // game controller strobe
        }
        else if(address < 0xC090)
        {
            // peripheral card GPIO slot 0
        }
        else if(address < 0xC0A0)
        {
            // peripheral card GPIO slot 1
        }
        else if(address < 0xC0B0)
        {
            // peripheral card GPIO slot 2
        }
        else if(address < 0xC0C0)
        {
            // peripheral card GPIO slot 3
        }
        else if(address < 0xC0D0)
        {
            // peripheral card GPIO slot 4
        }
        else if(address < 0xC0E0)
        {
            // peripheral card GPIO slot 5
        }
        else if(address < 0xC0F0)
        {
            // peripheral card GPIO slot 6
        }
        else if(address < 0xC100)
        {
            // peripheral card GPIO slot 7
        }
    }
}

/**
 * fetches working data from memory according to addressing mode
 *
 * @param address address to read from
 * @return data fetched from memory
 */
unsigned char readByte(unsigned short address)
{
    WaitForSingleObject(memMutex, INFINITE);

    unsigned char ret = memory[address];

    //printf("reading %02X from %04X\n", ret, address);

    ioSelect(address);

    ReleaseMutex(memMutex);

    return ret;
}

/**
 * stores a byte in memory according to addressing mode
 * @param byte value to store
 */
void writeByte(unsigned short address, unsigned char byte)
{
    WaitForSingleObject(memMutex, INFINITE);

    //printf("writing %02X to %04X\n", byte, address);
    memory[address] = byte;
    ioSelect(address);

    ReleaseMutex(memMutex);

    // peripheral card PROM space slots n=1 to 7 each get page $Cn
    // each card (including 0) also gets 16 GPIO addresses from $C080 to $C0F0
    // CPU references to $C080 to $C0F0 turn on DEVICE SELECT line for card, turning their flip-flops on
    // CPU references to page $Cn turn on IO SELECT line for card
    // card PROM reference to $CFFF will cause all cards to turn their flip-flops off
    // when CPU executes next instruction in PROM, IO SELECT will be re-enabled

    // 1. access GPIO space to turn on DEVICE SELECT flip flop to partially turn on expansion ROM
    // 2. access PROM space to turn on IO SELECT line, completely turning of expansion ROM
    // 3. PROM accesses $CFFF turn turn off all DEVICE SELECT flip-flops
    // 4. fetching next instruction in PROM re-activates expansion ROM
    // 5. PROM now has full access to its expansion ROM via address $C800-$CFFF

    // flags DS && IOSelect required to turn on XROM_ACCESS (latch)
    // then IOSelect && XROM_ACCESS required to access expansion rom for that card

    // TODO combine read and write byte functions, write these flags for each card, write char arrays to hold PROM and XROM for each card, check flags and if address in $C800 to $CFFF to determine which of these arrays to access
    // TODO IO card hardware will be packaged as a dynamic shared object, and PROM and XROM will be bin files with the same name. object file should have an init function
    // its init function should create a thread that deals with its GPIO space (address will be assigned to it through init parameter when object is loaded)
    // use dlopen and dlsym to get pointer to init function
}