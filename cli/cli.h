#include <getopt.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>


#ifndef CLI_H
#define CLI_H 1


struct cli_opts {
    struct {
        bool rprt;
        char * path;
    } lxr;
    char * input_path;
    bool err;
};

struct cli_opts
cli_get_opts(int argc, char * const * argv);

void
cli_help(void);

#endif
// CLI_H
