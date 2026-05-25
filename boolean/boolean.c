#include "boolean.h"
#include <string.h>

char *
bool_to_str(bool val)
{
    if (val) return TRUE_STR;
    return FALSE_STR;
}

bool
str_to_bool(char * val)
{
    if (strcmp(val, TRUE_STR) == 0) return true;
    return false;
}
