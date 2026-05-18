#include <stdio.h>
#include <stdbool.h>

#ifndef VM_UTIL_H
#define VM_UTIL_H 1

void * vm_ensure_ptr_proc(char * owner, void * ptr, char * name);
#define vm_ensure_ptr(owner, name) (vm_ensure_ptr_proc((owner), (name), #name))


int vm_fprintf(FILE * f, const char *fmt, ...);
int vm_err(char * owner, const char *format, ...);
void * vm_malloc(char * owner, int size, char * resource_name);
void * vm_realloc(char * owner, void * ptr, int size, char * resource_name);
FILE * vm_fopen(char * owner, char * path, char * mode, FILE * default_file);
bool vm_fclose(char * owner, FILE * stream);

#endif
// VM_UTIL_H
