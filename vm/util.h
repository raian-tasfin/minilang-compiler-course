#include <stdio.h>

#ifndef VM_UTIL_H
#define VM_UTIL_H 1

int vm_fprintf(FILE * f, const char *fmt, ...);
int vm_err(char * owner, const char *format, ...);


#endif
// VM_UTIL_H
