#include <getopt.h>
#include <stdlib.h>
#include <stdbool.h>


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


#endif
// CLI_H
