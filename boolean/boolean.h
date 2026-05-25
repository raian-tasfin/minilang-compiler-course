#include <stdbool.h>

#ifndef BOOLEAN_H
#define BOOLEAN_H 1

#define TRUE_STR   "True"
#define FALSE_STR  "False"

char * bool_to_str(bool val);
bool   str_to_bool(char * val);

#endif
// BOOLEAN_H
