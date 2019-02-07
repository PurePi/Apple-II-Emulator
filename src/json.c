#include "json.h"
#include <mem.h>
#include <stdio.h>

/**
 * checks if token matches str
 * @param json json string
 * @param token token to check
 * @param str string t check against
 * @return 0 = false, 1 = true
 */
int jsoneq(const char *json, jsmntok_t *token, const char *str)
{
    if (token->type == JSMN_STRING && (int) strlen(str) == token->end - token->start &&
        strncmp(json + token->start, str, (size_t) (token->end - token->start)) == 0)
    {
        return 1;
    }
    return 0;
}

/**
 * checks if token starts with str
 * @param json json string
 * @param token token to check
 * @param str string t check against
 * @return 0 = false, 1 = true
 */
int jsonStart(const char *json, jsmntok_t *token, const char *str)
{
    if (token->type == JSMN_STRING && (int) strlen(str) + 1 == token->end - token->start &&
        strncmp(json + token->start, str, (size_t) (token->end - token->start - 1)) == 0)
    {
        return 1;
    }
    return 0;
}

char errorMsg[FILENAME_MAX] = { 0 };