#include <stdio.h>
#include <HIF.h>
#include <cpu.h>
#include <memory.h>
#include "peripheral.h"

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
            printf("Error when reading %s", filename);
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
    if(!readFile("rom/rom.bin", memory + 0xFA00))
    {
        return 0;
    }

    if(!readCards())
    {
        return 0;
    }

    memMutex = CreateMutex(NULL, FALSE, NULL);
    screenMutex = CreateMutex(NULL, FALSE, NULL);
    runningMutex = CreateMutex(NULL, FALSE, NULL);

    reset();

    switch(startGLFW())
    {
    case 1:
        printf("glfw init error\n");
        break;
    case 2:
        printf("glfw create window error\n");
    default:
        break;
    }

    return 0;
}