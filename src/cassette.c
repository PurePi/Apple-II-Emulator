#include <stdio.h>
#include <malloc.h>
#include <mem.h>
#include <jsmn.h>
#include "cassette.h"
#include "memory.h"

static FILE *tape = NULL;
// 1 = write, 2 = read
static char mode = 0;

/**
 * checks if token matches str
 * @param json json string
 * @param token token to check
 * @param str string t check against
 * @return 0 = false, 1 = true
 */
static int jsoneq(const char *json, jsmntok_t *token, const char *str)
{
    if (token->type == JSMN_STRING && (int) strlen(str) == token->end - token->start &&
        strncmp(json + token->start, str, (size_t) (token->end - token->start)) == 0)
    {
        return 1;
    }
    return 0;
}

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
            printf("Error when opening config file");
            fclose(config);
            return NULL;
        }
        json[fileLength] = 0;
        fclose(config);
    }
    else
    {
        return NULL;
    }

    jsmn_parser p;
    jsmntok_t tokens[19];
    jsmn_init(&p);
    int jsmnResult = jsmn_parse(&p, json, strlen(json), tokens, 19);
    if(jsmnResult < 0)
    {
        printf("Error parsing JSON file\n");
        return NULL;
    }
    if(jsmnResult == 0 || tokens[0].type != JSMN_OBJECT)
    {
        printf("Top level of config file should be an object.\n");
        return NULL;
    }

    for(int tok = 1; tok < jsmnResult; tok++)
    {
        if(jsoneq(json, &tokens[tok], "tape"))
        {
            json[tokens[tok+1].end] = 0;
            memcpy(json, "tapes/", 6);
            memcpy(json + 6, json + tokens[tok+1].start, strlen(json + tokens[tok+1].start));
            memcpy(json + 6 + strlen(json + tokens[tok+1].start), ".bin", 5);
            return json;
        }
    }
    printf("Failed to find cassette\n");
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