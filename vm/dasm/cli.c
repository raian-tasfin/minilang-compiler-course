#include "cli.h"
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>


/**************
 * Public API *
 **************/
void
vmdasm_cli_help(void)
{
    fprintf(stdout,
            "Usage: vm dasm [OPTION]... FILE\n"
            "\n"
            "Run FILE as a MiniLang object file.\n"
            "sub-command of vm.\n"
            "\n"
            "Options:\n"
            "  -o, --outpath=FILE      If specified, assembly code is emitted to\n"
            "                          FILE. If omitted, it's written to stdout.\n"
            "  -h, --help              Display this help and exit.\n"
            "\n"
            "Arguments:\n"
            "  FILE                    Input object file to dasm.\n"
            "\n"
            "Examples:\n"
            "  vm dasm program.mlo\n"
            "  vm dasm -ooutput.mlasm program.mlo\n"
            "  vm dasm --outpath=output.mlasm program.mlo\n"
            "\n"
            "Report bugs to: <raian.csecu@gmail.com>\n"
    );
}

struct vmdasm_cli_opts
vmdasm_cli_getopts(int argc, char **argv)
{
    static struct option opts[] = {
        { "outpath", required_argument, NULL, 'o'},
        { "help", no_argument, NULL, 'h'},
        { 0, 0, 0, 0},
    };

    struct vmdasm_cli_opts cliopts = {0};
    int opt;
    int optindx = 0;
    while ((opt = getopt_long(argc, (char*const*)(argv), "o:h", opts, &optindx)) != -1) {
        switch (opt) {
        case 'o':
            cliopts.output_path = optarg;
            break;
        case 'h':
            cliopts.err = true;
            vmdasm_cli_help();
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



bool
vmdasm_cli_main(int argc, char **argv)
{
    /* Parse CLI options */
    struct vmdasm_cli_opts opts = vmdasm_cli_getopts(argc, argv);
    if (opts.err) return false;
    fprintf(stdout,
            "[VM DASM]: Parsed CLI options.\n"
            "           input  path: %s\n"
            "           output path: %s\n",
            opts.input_path ? opts.input_path : "NULL (ERROR: Input path must be present)",
            opts.output_path ? opts.output_path : "stdout"
            );

    return true;
}
