#include "cli.h"


struct cli_opts
cli_get_opts(int argc, char * const *argv)
{
    static struct option opts[] = {
        {"lex-report", optional_argument, NULL, 'l' },
        {"help", optional_argument, NULL, 'h' },
        {0, 0, 0, 0}
    };
    struct cli_opts cliopts = {0};
    int opt;
    int optindx = 0;
    while ((opt
            = getopt_long(argc, argv, "l::h", opts, &optindx))
           != -1) {
        switch (opt) {
        case 'l':
            cliopts.lxr.rprt = true;
            cliopts.lxr.path = optarg;
            break;
        case 'h':
            cliopts.err = true;
            cli_help();
            return cliopts;
        case '?':
            cliopts.err = true;
            return cliopts;
        }
    }
    if (argc - optind != 1)  {
        fprintf(stderr, "Expected exactly one input file.\n");
        cliopts.err = true;
        return cliopts;
    }
    cliopts.input_path = argv[optind];
    optind++;
    return cliopts;
}


void
cli_help(void)
{
    fprintf(
            stdout,
"Usage: minilang [OPTION]... FILE\n"
"\n"
"Parse FILE as a MiniLang source file.\n"
"\n"
"Options:\n"
"  -l, --lex-report[=FILE]   write a lexer token report to FILE. If\n"
"                            FILE is omitted, output will be written\n"
"                            to stdout\n"
"  -h, --help                display this help and exit\n"
"\n"
"Arguments:\n"
"  FILE                      input source file to parse\n"
"\n"
"Examples:\n"
"  minilang program.ml\n"
"  minilang --lex-report=lex.txt program.ml\n"
"  minilang -llex.txt program.ml\n"
"\n"
"Report bugs to: <raian.csecu@gmail.com>\n"
    );
}
