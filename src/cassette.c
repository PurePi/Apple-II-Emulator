#include <stdio.h>
#include <malloc.h>
#include <mem.h>
#include <jsmn.h>
#include "cassette.h"
#include "memory.h"
#include "json.h"

static FILE *tape = NULL;
// 1 = write, 2 = read
static char mode = 0;

/**
 * reads config.json for "tape" value
 * @return pointer to tape name if fond, or NULL if error
 */
static char *readConfig()
{
    static char *json;
    FILE *config = fopen("config.json", "rb");
    if(config)
    {
        fseek(config, 0, SEEK_END);
        size_t fileLength = (size_t) ftell(config);
        fseek(config, 0, SEEK_SET);

        json = malloc(fileLength + 1);

        if(fread(json, 1, fileLength, config) != fileLength)
        {
            sprintf(errorMsg, "Error when reading config.json");
            fclose(config);
            MessageBox(0, errorMsg, "Cassette Error", MB_OK);
            return NULL;
        }
        json[fileLength] = 0;
        fclose(config);
    }
    else
    {
        sprintf(errorMsg, "Error when opening config.json");
        MessageBox(0, errorMsg, "Cassette Error", MB_OK);
        return NULL;
    }

    jsmn_parser p;
    jsmntok_t tokens[19];
    jsmn_init(&p);
    int jsmnResult = jsmn_parse(&p, json, strlen(json), tokens, 19);
    if(jsmnResult < 0)
    {
        sprintf(errorMsg, "Error parsing config.json");
        MessageBox(0, errorMsg, "Cassette Error", MB_OK);
        return NULL;
    }
    if(jsmnResult == 0 || tokens[0].type != JSMN_OBJECT)
    {
        sprintf(errorMsg, "Top level of config.json should be an object.");
        MessageBox(0, errorMsg, "Cassette Error", MB_OK);
        return NULL;
    }

    for(int tok = 1; tok < jsmnResult; tok++)
    {
        if(jsoneq(json, &tokens[tok], "tape"))
        {
            json[tokens[tok+1].end] = 0;
            memmove(json, "tapes/", 6);
            memmove(json + 6, json + tokens[tok+1].start, strlen(json + tokens[tok+1].start));
            memmove(json + 6 + strlen(json + tokens[tok+1].start), ".bin", 5);
            return json;
        }
    }
    sprintf(errorMsg, "Failed to find cassette");
    MessageBox(0, errorMsg, "Cassette Error", MB_OK);
    return NULL;
}

/**
 * sets the cassette to record (opens cassette file for writing)
 */
void setRecord()
{
    if(tape)
    {
        fclose(tape);
        tape = NULL;
        if(mode == 1)
        {
            return;
        }
    }
    char *json = readConfig();
    if(json == NULL)
    {
        return;
    }
    tape = fopen(json, "wb");
    free(json);
    if(tape == NULL)
    {
        return;
    }
    fseek(tape, 0, SEEK_SET);
    mode = 1;
}

/**
 * sets the cassette to play (opens cassette file for reading)
 */
void setPlay()
{
    if(tape)
    {
        fclose(tape);
        tape = NULL;
        if(mode == 2)
        {
            return;
        }
    }
    char *json = readConfig();
    if(json == NULL)
    {
        return;
    }
    tape = fopen(json, "rb");
    free(json);
    if(tape == NULL)
    {
        printf("Error opening tape\n");
        return;
    }
    fseek(tape, 0, SEEK_SET);
    mode = 2;
}

/**
 * records a byte from $C020 onto the cassette
 */
void record()
{
    if(!tape || mode != 1)
    {
        return;
    }
    fwrite(memory + 0xC020, 1, 1, tape);
}

/**
 * reads a byte from the cassette into $C060
 */
void playback()
{
    if(!tape || mode != 2)
    {
        return;
    }
    fread(memory + 0xC060, 1, 1, tape);
    memory[0xC068] = memory[0xC060];
}

/**
 * closes the tape file if it
 */
void closeTape()
{
    if(tape)
    {
        fclose(tape);
        tape = NULL;
    }
}