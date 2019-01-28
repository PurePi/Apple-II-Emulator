#include <stdio.h>
#include <jsmn.h>
#include <malloc.h>
#include <mem.h>
#include <ctype.h>
#include <errno.h>
#include <dlfcn.h>
#include <memory.h>
#include "peripheral.h"

struct peripheralCard peripherals[8] = { 0 };

char errorMsg[FILENAME_MAX] = { 0 };

/**
 * checks if token starts with str
 * @param json json string
 * @param token token to check
 * @param str string t check against
 * @return 0 = false, 1 = true
 */
static int jsonStart(const char *json, jsmntok_t *token, const char *str)
{
    if (token->type == JSMN_STRING && (int) strlen(str) + 1 == token->end - token->start &&
        strncmp(json + token->start, str, (size_t) (token->end - token->start - 1)) == 0)
    {
        return 1;
    }
    return 0;
}

/**
 * reads the config file and loads peripheral cards for use
 * @return 0 = failure, 1 = success
 */
char mountCards()
{
    static char periphCard[FILENAME_MAX];
    char *json;
    FILE *config = fopen("config.json", "rb");
    if(config)
    {
        fseek(config, 0, SEEK_END);
        size_t fileLength = (size_t) ftell(config);
        fseek(config, 0, SEEK_SET);

        json = malloc(fileLength + 1);

        if(fread(json, 1, fileLength, config) != fileLength)
        {
            sprintf(errorMsg, "Error when reading config file");
            fclose(config);
            MessageBox(0, errorMsg, "Peripheral Card Error", MB_OK);
            return 0;
        }
        json[fileLength] = 0;
        fclose(config);
    }
    else
    {
        sprintf(errorMsg, "Error when opening config file");
        MessageBox(0, errorMsg, "Peripheral Card Error", MB_OK);
        return 0;
    }

    jsmn_parser p;
    jsmntok_t tokens[19];
    jsmn_init(&p);
    int jsmnResult = jsmn_parse(&p, json, strlen(json), tokens, 19);
    if(jsmnResult < 0)
    {
        sprintf(errorMsg, "Error parsing JSON file\n");
        MessageBox(0, errorMsg, "Peripheral Card Error", MB_OK);
        return 0;
    }
    if(jsmnResult == 0 || tokens[0].type != JSMN_OBJECT)
    {
        sprintf(errorMsg, "Top level of config file should be an object.\n");
        MessageBox(0, errorMsg, "Peripheral Card Error", MB_OK);
        return 0;
    }

    for(int tok = 1; tok < jsmnResult; tok++)
    {
        if(jsonStart(json, &tokens[tok], "slot "))
        {
            strncpy(periphCard, json + tokens[tok].start, (size_t) (tokens[tok].end - tokens[tok].start));
            json[tokens[tok+1].end] = 0;
            if(!isdigit(json[tokens[tok].end - 1]) ||  strlen(json + tokens[tok+1].start) == 0)
            {
                continue;
            }
            json[tokens[tok].end] = 0;

            memcpy(periphCard, "cards/", 6);
            memcpy(periphCard + 6, json + tokens[tok+1].start, strlen(json + tokens[tok+1].start));
            memcpy(periphCard + 6 + strlen(json + tokens[tok+1].start), ".dll", 5);
            int cardNumber = strtol(json +tokens[tok].end - 1, NULL, 10);
            if(errno == EINVAL)
            {
                sprintf(errorMsg, "Error parsing card number in config.json\n");
                MessageBox(0, errorMsg, "Peripheral Card Error", MB_OK);
                return 0;
            }
            if(cardNumber > 7)
            {
                sprintf(errorMsg, "Error: found card number > 7 (%d)", cardNumber);
                MessageBox(0, errorMsg, "Peripheral Card Error", MB_OK);
                return 0;
            }

            peripherals[cardNumber].handle = dlopen(periphCard, RTLD_LAZY | RTLD_LOCAL);
            peripherals[cardNumber].startup = dlsym(peripherals[cardNumber].handle, "startup");
            if(peripherals[cardNumber].startup == NULL)
            {
                sprintf(errorMsg, "Could not find startup in %s", periphCard);
                MessageBox(0, errorMsg, "Peripheral Card Error", MB_OK);
                return 0;
            }
            peripherals[cardNumber].shutdown = dlsym(peripherals[cardNumber].handle, "shutdown");
            if(peripherals[cardNumber].shutdown == NULL)
            {
                sprintf(errorMsg, "Could not find shutdown in %s", periphCard);
                MessageBox(0, errorMsg, "Peripheral Card Error", MB_OK);
                return 0;
            }
            peripherals[cardNumber].deviceSelect = dlsym(peripherals[cardNumber].handle, "deviceSelect");
            if(peripherals[cardNumber].deviceSelect == NULL)
            {
                sprintf(errorMsg, "Could not find deviceSelect in %s", periphCard);
                MessageBox(0, errorMsg, "Peripheral Card Error", MB_OK);
                return 0;
            }

            if(cardNumber == 0)
            {
                peripherals[cardNumber].memRef = dlsym(peripherals[cardNumber].handle, "memRef");
                if(peripherals[cardNumber].memRef == NULL)
                {
                    sprintf(errorMsg, "Could not find memRef in %s", periphCard);
                    MessageBox(0, errorMsg, "Peripheral Card Error", MB_OK);
                    return 0;
                }
                // startup takes pointer to memory in this case
                ((void (*) (unsigned char *)) peripherals[cardNumber].startup)(memory);
            }
            else
            {
                memcpy(periphCard + 6 + strlen(json + tokens[tok+1].start), "PROM.bin", 9);
                FILE *promFile = fopen(periphCard, "rb");
                if(promFile)
                {
                    fseek(promFile, 0, SEEK_END);
                    size_t fileLength = (size_t) ftell(promFile);
                    fseek(promFile, 0, SEEK_SET);

                    if(fileLength > 0x100)
                    {
                        sprintf(errorMsg, "PROM file %s is larger than 256 bytes", periphCard);
                        MessageBox(0, errorMsg, "Peripheral Card Error", MB_OK);
                    }

                    if(fread(memory + 0xC000 + (0x100 * cardNumber), 1, fileLength, promFile) != fileLength)
                    {
                        sprintf(errorMsg, "Error when reading %s", periphCard);
                        MessageBox(0, errorMsg, "Peripheral Card Error", MB_OK);
                        fclose(promFile);
                        return 0;
                    }
                    fclose(promFile);
                }
                else
                {
                    sprintf(errorMsg, "Error when opening %s", periphCard);
                    MessageBox(0, errorMsg, "Peripheral Card Error", MB_OK);
                    return 0;
                }
                memcpy(periphCard + 6 + strlen(json + tokens[tok+1].start), "XROM.bin", 9);
                FILE *xromFile = fopen(periphCard, "rb");
                if(xromFile)
                {
                    fseek(xromFile, 0, SEEK_END);
                    size_t fileLength = (size_t) ftell(xromFile);
                    fseek(xromFile, 0, SEEK_SET);

                    if(fileLength > 0x800)
                    {
                        sprintf(errorMsg, "XROM file %s is larger than 2048 bytes", periphCard);
                        MessageBox(0, errorMsg, "Peripheral Card Error", MB_OK);
                    }

                    if(fread(peripherals[cardNumber].expansionRom, 1, fileLength, xromFile) != fileLength)
                    {
                        sprintf(errorMsg, "Error when reading %s", periphCard);
                        MessageBox(0, errorMsg, "Peripheral Card Error", MB_OK);
                        fclose(xromFile);
                        return 0;
                    }
                    fclose(xromFile);
                }
                else
                {
                    sprintf(errorMsg, "Error when opening %s", periphCard);
                    MessageBox(0, errorMsg, "Peripheral Card Error", MB_OK);
                    return 0;
                }

                // startup does not take pointer to memory if not slot 0
                ((void (*)()) peripherals[cardNumber].startup)();
            }
        }
    }
    return 1;
}

/**
 * unmounts all peripheral cards and shuts them down
 */
void unmountCards()
{
    for(int card = 0; card < 8; card++)
    {
        if(peripherals[card].handle != NULL)
        {
            peripherals[card].shutdown();
        }
    }
    // second loop because same card (dll) could be mounted to multiple slots, so we shutdown all of them before dlclosing them
    for(int card = 0; card < 8; card++)
    {
        if(peripherals[card].handle != NULL)
        {
            dlclose(peripherals[card].handle);
            peripherals[card].handle = NULL;
        }
    }
}