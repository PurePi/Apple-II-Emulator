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
char readCards()
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
            printf("Error when reading config file");
            fclose(config);
            return 0;
        }
        json[fileLength] = 0;
        fclose(config);
    }
    else
    {
        printf("Error when opening config file");
        return 0;
    }

    jsmn_parser p;
    jsmntok_t tokens[19];
    jsmn_init(&p);
    int jsmnResult = jsmn_parse(&p, json, strlen(json), tokens, 19);
    if(jsmnResult < 0)
    {
        printf("Error parsing JSON file\n");
        return 0;
    }
    if(jsmnResult == 0 || tokens[0].type != JSMN_OBJECT)
    {
        printf("Top level of config file should be an object.\n");
        return 0;
    }

    for(int tok = 1; tok < jsmnResult; tok++)
    {
        if(jsonStart(json, &tokens[tok], "slot "))
        {
            strncpy(periphCard, json + tokens[tok].start, (size_t) (tokens[tok].end - tokens[tok].start));
            //printf("card # %c\n", json[tokens[tok].end - 1]);
            json[tokens[tok+1].end] = 0;
            if(!isdigit(json[tokens[tok].end - 1]) ||  strlen(json + tokens[tok+1].start) == 0)
            {
                continue;
            }
            json[tokens[tok].end] = 0;
            //printf("value: %s\n", json + tokens[tok+1].start);
            memcpy(periphCard, "cards/", 6);
            memcpy(periphCard + 6, json + tokens[tok+1].start, strlen(json + tokens[tok+1].start));
            //memcpy(periphCard + 6 + strlen(json + tokens[tok+1].start), ".dll", 5);
            int cardNumber = strtol(json +tokens[tok].end - 1, NULL, 10);
            if(errno == EINVAL)
            {
                printf("Error parsing card number in config.json\n");
                return 0;
            }
            if(cardNumber > 7)
            {
                printf("Error: found card number > 7 (%d)", cardNumber);
                return 0;
            }

            // TODO make a dll to test this
            printf("Finding %s for slot %d\n", periphCard, cardNumber);
            peripherals[cardNumber].handle = dlopen(periphCard, RTLD_LAZY | RTLD_LOCAL);
            peripherals[cardNumber].startup = dlsym(peripherals[cardNumber].handle, "startup");
            if(peripherals[cardNumber].startup == NULL)
            {
                printf("Could not find startup in %s", periphCard);
            }
            peripherals[cardNumber].shutdown = dlsym(peripherals[cardNumber].handle, "shutdown");
            if(peripherals[cardNumber].shutdown == NULL)
            {
                printf("Could not find shutdown in %s", periphCard);
            }

            if(cardNumber != 0)
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
                        printf("PROM file %s is larger than 256 bytes", periphCard);
                    }

                    if(fread(memory + 0xC000 + (0x100 * cardNumber), 1, fileLength, promFile) != fileLength)
                    {
                        printf("Error when reading %s", periphCard);
                        fclose(promFile);
                        return 0;
                    }
                    fclose(promFile);
                }
                else
                {
                    printf("Error when opening %s", periphCard);
                    return 0;
                }
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
                    printf("XROM file %s is larger than 2048 bytes", periphCard);
                }

                if(fread(peripherals[cardNumber].expansionRom, 1, fileLength, xromFile) != fileLength)
                {
                    printf("Error when reading %s", periphCard);
                    fclose(xromFile);
                    return 0;
                }
                fclose(xromFile);
            }
            else
            {
                printf("Error when opening %s", periphCard);
                return 0;
            }

        }
    }
    return 1;
}