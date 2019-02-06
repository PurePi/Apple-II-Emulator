#include <stdio.h>
#include <HIF.h>
#include <cpu.h>
#include <memory.h>
#include <ctype.h>
#include "peripheral.h"
#include "json.h"

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
            MessageBox(0, errorMsg, "ROM Error", MB_OK);
            return 0;
        }
        fclose(fp);
        return 1;
    }
    return 0;
}

static char readRoms()
{
    char *json;
    static char romName[FILENAME_MAX];
    FILE *config = fopen("roms.json", "rb");
    if(config)
    {
        fseek(config, 0, SEEK_END);
        size_t fileLength = (size_t) ftell(config);
        fseek(config, 0, SEEK_SET);

        json = malloc(fileLength + 1);

        if(fread(json, 1, fileLength, config) != fileLength)
        {
            sprintf(errorMsg, "Error when reading roms.json");
            fclose(config);
            free(json);
            MessageBox(0, errorMsg, "ROM Error", MB_OK);
            return 0;
        }
        json[fileLength] = 0;
        fclose(config);
    }
    else
    {
        sprintf(errorMsg, "Error when opening roms.json");
        MessageBox(0, errorMsg, "ROM Error", MB_OK);
        return 0;
    }

    jsmn_parser p;
    jsmntok_t tokens[50];
    jsmn_init(&p);
    int jsmnResult = jsmn_parse(&p, json, strlen(json), tokens, 50);
    if(jsmnResult < 0)
    {
        sprintf(errorMsg, "Error parsing roms.json");
        MessageBox(0, errorMsg, "ROM Error", MB_OK);
        free(json);
        return 0;
    }

    if(jsmnResult == 0 || tokens[0].type != JSMN_OBJECT)
    {
        sprintf(errorMsg, "Top level of roms.json should be an object.");
        MessageBox(0, errorMsg, "ROM Error", MB_OK);
        free(json);
        return 0;
    }

    for(int tok = 1; tok < jsmnResult; tok++)
    {
        if(tokens[tok].type == JSMN_STRING)
        {
            // get "rom/" + file + ".bin"
            json[tokens[tok].end] = 0;
            memcpy(romName, "rom/", 4);
            memcpy(romName + 4, json + tokens[tok].start, strlen(json + tokens[tok].start));
            memcpy(romName + 4 + strlen(json + tokens[tok].start), ".bin", 5);

            // ensure it has the correct data following it (array of two numbers)
            if(tokens[tok+1].type != JSMN_ARRAY || tokens[tok+1].size != 2 || tokens[tok+2].type != JSMN_PRIMITIVE || !isdigit((int) json[tokens[tok+2].start]) || tokens[tok+3].type != JSMN_PRIMITIVE || !isdigit((int) json[tokens[tok+3].start]))
            {
                sprintf(errorMsg, "Invalid format in roms.json");
                fclose(config);
                free(json);
                MessageBox(0, errorMsg, "ROM Error", MB_OK);
                return 0;
            }

            FILE *romFile = fopen(romName, "rb");
            if(romFile)
            {
                fseek(config, 0, SEEK_END);
                size_t fileLength = (size_t) ftell(config);
                fseek(config, 0, SEEK_SET);

                int startAddr = strtol(json + tokens[tok+2].start, NULL, 0);
                int endAddr = strtol(json + tokens[tok+3].start, NULL, 0);
                endAddr = endAddr > 65535 ? 65535 : endAddr;

                if(startAddr < 0 || startAddr > 65535)
                {
                    sprintf(errorMsg, "Invalid starting address for ROM file %s", romName);
                    fclose(config);
                    free(json);
                    MessageBox(0, errorMsg, "ROM Error", MB_OK);
                    return 0;
                }

                if(startAddr + fileLength - 1 > endAddr)
                {
                    sprintf(errorMsg, "ROM file %s (%d bytes) exceeds maximum length (%d bytes)", romName, fileLength, endAddr - startAddr + 1);
                    fclose(config);
                    free(json);
                    MessageBox(0, errorMsg, "ROM Error", MB_OK);
                    return 0;
                }

                if(fread(memory + startAddr, 1, fileLength, romFile) != fileLength)
                {
                    sprintf(errorMsg, "Error when reading %s", romName);
                    fclose(config);
                    free(json);
                    MessageBox(0, errorMsg, "ROM Error", MB_OK);
                    return 0;
                }
                fclose(romFile);
            }
            else
            {
                sprintf(errorMsg, "Error when opening %s", romName);
                MessageBox(0, errorMsg, "ROM Error", MB_OK);
                free(json);
                return 0;
            }
        }
    }

    free(json);
    return 1;
}

int main()
{
    if(!readRoms())
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
