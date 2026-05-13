/* lex.yy.h must be included before lexer_util.h so the concrete
 * yyscan_t typedef (and YYSTYPE/YYLTYPE via parser.tab.h) are visible
 * when lexer_util.h is processed. */
#include "lex.yy.h"
#include <stdio.h>
#include "lexer_util.h"


/**************/
/* Public API */
/**************/
struct lxr_rprt_ctx
lxr_rprt_ctx_init(bool rprt, char * path)
{
    struct lxr_rprt_ctx ctx = { .rprt = rprt, .path = path };
    if (!rprt) return ctx;
    if (!path) {
        ctx.strm = stdout;
        return ctx;
    }
    ctx.strm = fopen(ctx.path, "w");
    if (!ctx.strm) {
        ctx.err = true;
        fopen("Failed to open file '%s'\n.", ctx.path);
        return ctx;
    }
    return ctx;
}


char *
lxr_toktostr(int token_type)
{
    switch (token_type) {
    case ADD:         return "ADD";
    case AST_SUBEXPR: return "AST_SUBEXPR";
    case DIV:         return "DIV";
    case INTEGER:     return "INTEGER";
    case LEX_BLNK:    return "LEX_BLNK";
    case LEX_CONT:    return "LEX_CONT";
    case LEX_ERR:     return "LEX_ERR";
    case LPRN:        return "LPRN";
    case MOD:         return "MOD";
    case MUL:         return "MUL";
    case NEWLINE:     return "NEWLINE";
    case RPRN:        return "RPRN";
    case SUB:         return "SUB";
    default:          return "UNIDENTIFIED";
    }
}

void lxr_updt_loc(YYLTYPE *yylloc, void *yyscanner)
{
    int yyleng   = yyget_leng(yyscanner);
    char *yytext = yyget_text(yyscanner);
    if (yylloc->last_line == 0) {
        yylloc->last_line = 1;
        yylloc->last_column = 1;
    }
    yylloc->first_line = yylloc->last_line;
    yylloc->first_column = yylloc->last_column;
    if (strchr(yytext, '\n')) {
        yylloc->last_line++;
        yylloc->last_column = 1;
    } else {
        yylloc->last_column += yyleng;
    }
}


int lxr_process_proc(int token_type, YYSTYPE *yylval, YYLTYPE *yylloc, void *yyscanner)
{
    char *yytext = yyget_text(yyscanner);
    lxr_updt_loc(yylloc, yyscanner);
    switch (token_type) {
        /* Ignore cases */
    case LEX_BLNK: // lxr_print_token(LEX_BLNK, yylval);
    case LEX_CONT: // lxr_print_token(LEX_CONT, yylval);
        return LEX_IGNR;

        /* Cases with Semantic Value */
    case INTEGER:
        if (yylval) yylval->INTEGER = atoi(yytext);
        lxr_print_token(INTEGER, yylval);
        return INTEGER;

        /* Punctuators */
    case ADD:  lxr_print_token(ADD, yylval); return ADD;
    case SUB:  lxr_print_token(SUB, yylval); return SUB;
    case MUL:  lxr_print_token(MUL, yylval); return MUL;
    case DIV:  lxr_print_token(DIV, yylval); return DIV;
    case MOD:  lxr_print_token(MOD, yylval); return MOD;
    case LPRN: lxr_print_token(LPRN, yylval); return LPRN;
    case RPRN: lxr_print_token(RPRN, yylval); return RPRN;

    case NEWLINE:
        lxr_print_token(NEWLINE, yylval);
        return token_type;

        /* Error */
    case LEX_ERR:
        /* lxr_print_token(LEX_ERR, yylval);; */
        fprintf(stderr,
                "[lexing error]: Unexpected %s at %d:%d-%d:%d\n",
                yytext,
                yylloc->first_line,
                yylloc->first_column,
                yylloc->last_line,
                yylloc->last_column);
        return LEX_ERR;
    }

    return LEX_ERR;
}


void
ast_print_preorder(struct ast_node * root, FILE * strm)
{
    if (strm == NULL) return;
    if (root == NULL) return;

}
