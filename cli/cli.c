#include "cli.h"
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


void
cli_update_ast(char * optarg, struct cli_ast_opts * ast)
{
    char * sep = strchr(optarg, ':');
    if (strncmp(optarg, "dot", 3) == 0
        && (optarg[3] == '\0' || optarg[3] == ':')) {
        ast->dot = true;
        if (sep) ast->dot_path = strdup(sep + 1);
        return;
    }
    if (strncmp(optarg, "text", 4) == 0
        && (optarg[4] == '\0' || optarg[4] == ':')) {
        ast->text = true;
        if (sep) ast->text_path = strdup(sep + 1);
        return;
    }
    fprintf(stderr, "Unknown format string '%s'\n", optarg);
    ast->err = true;
}


struct cli_opts
cli_get_opts(int argc, char * const *argv)
{
    static struct option opts[] = {
        {"lex-report", optional_argument, NULL, 'l' },
        {"ast-report", required_argument, NULL, 'a' },
        {"ir-report", optional_argument, NULL, 'i' },
        {"compile", optional_argument, NULL, 'c' },
        {"help", optional_argument, NULL, 'h' },
        {0, 0, 0, 0}
    };
    struct cli_opts cliopts = {0};
    int opt;
    int optindx = 0;
    while ((opt
            = getopt_long(argc, argv, "l::a:i::c:h", opts, &optindx))
           != -1) {
        switch (opt) {
        case 'l':
            cliopts.lxr.rprt = true;
            cliopts.lxr.path = optarg;
            break;
        case 'a':
            cli_update_ast(optarg, &cliopts.ast);
            if (cliopts.ast.err) {
                cliopts.err = true;
                return cliopts;
            }
            break;
        case 'i':
            cliopts.ir.rprt = true;
            cliopts.ir.path = optarg;
            break;
        case 'c':
            cliopts.cg.generate = true;
            cliopts.cg.outpath = optarg;
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
    fprintf(stdout,
            "Usage: minilang [OPTION]... FILE\n"
            "\n"
            "Parse FILE as a MiniLang source file.\n"
            "\n"
            "Options:\n"
            "  -l, --lex-report[=FILE]     Write a lexer token report to FILE. If\n"
            "                              FILE is omitted, output will be written\n"
            "                              to stdout.\n"
            "  -a, --ast-report=FMT[:FILE] Write an AST report in FMT (dot or text).\n"
            "  -i, --ir-report[=FILE]      Write intermediate representation to FILE. If\n"
            "                              FILE is omitted, output will be written\n"
            "                              to stdout.\n"
            "                              If FILE is provided after a colon, output\n"
            "                              is written there; otherwise, it goes to stdout.\n"
            " -c, --compile=FILE           Generate object code and write to FILE."
            "  -h, --help                  Display this help and exit.\n"
            "\n"
            "Arguments:\n"
            "  FILE                        Input source file to parse.\n"
            "\n"
            "Examples:\n"
            "  minilang program.ml\n"
            "  minilang --lex-report=lex.txt program.ml\n"
            "  minilang --ast-report=dot program.ml\n"
            "  minilang -a text:ast.txt program.ml\n"
            "\n"
            "Report bugs to: <raian.csecu@gmail.com>\n"
    );
}
