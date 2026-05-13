#include <getopt.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>


#ifndef CLI_H
#define CLI_H 1

struct cli_lxr_opts {
    bool rprt;
    char * path;
};

struct cli_ast_opts {
    bool dot;
    char * dot_path;
    bool text;
    char * text_path;
    bool err;
};

struct cli_opts {
    struct cli_lxr_opts lxr;
    struct cli_ast_opts ast;
    char * input_path;
    bool err;
};

struct cli_opts
cli_get_opts(int argc, char * const * argv);

void
cli_help(void);

#endif
// CLI_H
