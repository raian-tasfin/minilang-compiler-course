#include "cli.h"
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>

struct vmrun_cli_opts
vmrun_cli_get_opts(int argc, char ** argv)
{
    static struct option opts[] = {
        { "output-path", required_argument, NULL, 'o'},
        { "help", no_argument, NULL, 'h'},
        { 0, 0, 0, 0},
    };

    struct vmrun_cli_opts cliopts = {0};
    int opt;
    int optindx = 0;
    while ((opt = getopt_long(argc, (char*const*)(argv), "o:h", opts, &optindx)) != -1) {
        switch (opt) {
        case 'o':
            cliopts.output_path = optarg;
            break;
        case 'h':
            cliopts.err = true;
            vmrun_cli_help();
            return cliopts;
        case '?':
            cliopts.err = true;
            return cliopts;
        }
    }
    if (argc - optind != 1) {
        fprintf(stderr, "Expected exactly one input file.\n");
        cliopts.err = true;
        return cliopts;
    }
    cliopts.input_path = argv[optind];
    optind++;
    return cliopts;
}


void
vmrun_cli_help(void)
{
    fprintf(stdout,
            "Usage: vmrun run [OPTION]... FILE\n"
            "\n"
            "Run FILE as a MiniLang object file. This is here is the default\n"
            "sub-command of vm.\n"
            "\n"
            "Options:\n"
            "  -o, --output-path=FILE      If specified, PRNT operation writes to\n"
            "                              FILE. If omitted, output writes to stdout\n"
            "  -h, --help                  Display this help and exit.\n"
            "\n"
            "Arguments:\n"
            "  FILE                        Input object file to run.\n"
            "\n"
            "Examples:\n"
            "  vm run program.mlo\n"
            "  vm run -ooutput.txt program.mlo\n"
            "  vm run --output-path=output.txt program.mlo\n"
            "\n"
            "Report bugs to: <raian.csecu@gmail.com>\n"
    );
}
