#include "util.h"
#include <stdarg.h>

int
vm_fprintf(FILE * f, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int ret = vfprintf(f, fmt, args);
    va_end(args);
    return ret;
}


int
vm_err(char * owner, const char *format, ...)
{
    int ret = 0;
    if (owner) ret += fprintf(stderr, "[%s]: ", owner);
    va_list args;
    ret += vm_fprintf(stderr, format, args);
    return ret;
}
