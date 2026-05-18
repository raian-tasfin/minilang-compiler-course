#include "util.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

void *
vm_ensure_ptr_proc(char * owner, void * ptr, char * name)
{
    if (!name) name = "";
    if (ptr) return ptr;
    fprintf(stderr, "[%s]: Null pointer provided for \"%s\"\n", owner, name);
    return NULL;
}


int
vm_err(char * owner, const char *format, ...)
{
    int ret = 0;
    if (owner) ret += fprintf(stderr, "[%s]: ", owner);

    va_list args;
    va_start(args, format);
    ret += vfprintf(stderr, format, args);
    va_end(args);
    return ret;
}


void *
vm_malloc(char * owner, int size, char * resource_name)
{
    if (!(vm_ensure_ptr(owner, resource_name))) return NULL;
    void * rec = NULL;
    if ((rec = malloc(size))) return rec;
    vm_err(owner, "Failed allocating for \"%s\"\n", resource_name);
    return rec;
}


void *
vm_realloc(char * owner, void * ptr, int size, char * resource_name)
{
    if (!(vm_ensure_ptr(owner, resource_name))) return NULL;

    void * tmp = realloc(ptr, size);
    if (!tmp) {
        vm_err(owner, "Failed reallocating \"%s\" to size %d\n", resource_name, size);
        return NULL;
    }
    return tmp;
}



FILE *
vm_fopen(char * owner, char * path, char * mode, FILE * default_file)
{
    if (!vm_ensure_ptr_proc(owner, mode, "mode")) return NULL;
    if (!path) {
        if (default_file) return default_file;
        vm_err(owner, "No path or default file provided for fopen.");
        return NULL;
    }
    FILE * res = fopen(path, mode);
    if (!res) {
        vm_err(owner,
               "Error opening \"%s\". %s\n",
               path,
               strerror(errno));
        return NULL;
    }
    return res;
}


bool
vm_fclose(char * owner, FILE * stream)
{
    if (!stream) return true;
    if (stream == stdout) return true;
    if (stream == stderr) return true;
    if (stream == stdin) return true;

    if (fclose(stream) != 0) {
        vm_err(owner, "Error closing file: %s\n", strerror(errno));
        return false;
    }

    return true;
}
