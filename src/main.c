#include <stdio.h>
#include <HIF.h>
#include <cpu.h>
#include <memory.h>
#include "peripheral.h"

extern char errorMsg[FILENAME_MAX];

/**
 * reads a file into memory
 * @param filename name of file to read
 * @param readTo where in memory to place the contents
 * @return 0 = failure, 1 = success
 */
static char readFile(char *filename, void *readTo)
{
    size_t fileLength;
    FILE *fp = fopen(filename, "rb");
    if(fp)
    {
        fseek(fp, 0, SEEK_END);
        fileLength = (size_t) ftell(fp);
        fseek(fp, 0, SEEK_SET);
        if(fread(readTo, 1, fileLength, fp) != fileLength)
        {
            sprintf(errorMsg, "Error when reading %s", filename);
            fclose(fp);
            return 0;
        }
        fclose(fp);
        return 1;
    }
    return 0;
}

int main()
{
    if(!readFile("rom/vectors.bin", memory + 0xFFFA))
    {
        return 0;
    }
    if(!readFile("rom/rom.bin", memory + 0xF800))
    {
        return 0;
    }

    // for text mode (default from a cold start), set all to black space
    memset(memory + 0x400, 0xA0, 0x800);

    memMutex = CreateMutex(NULL, FALSE, NULL);
    screenMutex = CreateMutex(NULL, FALSE, NULL);
    runningMutex = CreateMutex(NULL, FALSE, NULL);

    reset();

    switch(startGLFW())
    {
    case 1:
        sprintf(errorMsg, "glfw init error\n");
        MessageBox(0, errorMsg, "Peripheral Card Error", MB_OK);
        break;
    case 2:
        sprintf(errorMsg, "glfw create window error\n");
        MessageBox(0, errorMsg, "Peripheral Card Error", MB_OK);
    default:
        break;
    }

    return 0;
}
