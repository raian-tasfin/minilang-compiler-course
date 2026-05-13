#include "cli.h"

struct cli_opts
cli_get_opts(int argc, char * const *argv)
{
    static struct option opts[] = {
        {"lex-report", optional_argument, NULL, 'l' },
        {0, 0, 0, 0}
    };
    struct cli_opts cliopts = {0};
    int opt;
    int optindx = 0;
    while ((opt
            = getopt_long(argc, argv, "l::", opts, &optindx))
           != -1) {
        switch (opt) {
        case 'l':
            cliopts.lxr.rprt = true;
            cliopts.lxr.path = optarg;
            break;
        case '?':
            cliopts.err = true;
            return cliopts;
        }
    }
    if (argc - optind != 1)  {
        cliopts.err = true;
        return cliopts;
    }
    cliopts.input_path = argv[optind];
    optind++;
    return cliopts;
}
