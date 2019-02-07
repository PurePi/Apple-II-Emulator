#ifndef APPLE_II_JSON_H
#define APPLE_II_JSON_H

#include "jsmn.h"

extern int jsoneq(const char *json, jsmntok_t *token, const char *str);
extern int jsonStart(const char *json, jsmntok_t *token, const char *str);

extern char errorMsg[];

#endif //APPLE_II_JSON_H
