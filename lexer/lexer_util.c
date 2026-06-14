/* lex.yy.h must be included before lexer_util.h so the concrete
 * yyscan_t typedef (and YYSTYPE/YYLTYPE via parser.tab.h) are visible
 * when lexer_util.h is processed. */
#include "lex.yy.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lexer_util.h"
#include "../boolean/boolean.h"


/*********************
 * Private Utilities *
 *********************/
void
lxr_print(struct lxr_ctx * ctx, const char *fmt, ...)
{
    if (!ctx || !ctx->rprt) return;
    va_list args;
    va_start(args, fmt);
    vfprintf(ctx->rprt, fmt, args);
    va_end(args);
}

/**************/
/* Public API */
/**************/
struct lxr_ctx
lxr_ctx_init(struct cli_opts * cliopts)
{
    struct lxr_ctx ctx = { 0 };
    ctx.open_parens = 0;
    if (!cliopts) return ctx;

    /****************
     * Input Stream *
     ****************/
    ctx.instream = fopen(cliopts->input_path, "r");
    if (!ctx.instream) {
        fprintf(stderr, "Could not open file '%s'\n", cliopts->input_path);
        ctx.err = true;
        return ctx;
    }

    /**************
     * Report Ctx *
     **************/
    if (!cliopts->lxr.rprt) return ctx;
    if (!cliopts->lxr.path) ctx.rprt = stdout;
    else ctx.rprt = fopen(cliopts->lxr.path, "w");
    if (!ctx.rprt) {
        fprintf(stderr, "Could not open file '%s'\n", cliopts->lxr.path);
        ctx.err = true;
    }

    return ctx;
}


char *
lxr_toktostr(int token_type)
{
    switch (token_type) {
    case LEX_BLNK:           return "LEX_BLNK";
    case LEX_CONT:           return "LEX_CONT";
    case NEWLINE:            return "NEWLINE";
    case LEX_ERR:            return "LEX_ERR";
    case INTEGER:            return "INTEGER";
    case BOOLEAN:            return "BOOLEAN";
    case ADD:                return "ADD";
    case SUB:                return "SUB";
    case MUL:                return "MUL";
    case DIV:                return "DIV";
    case MOD:                return "MOD";
    case AND:                return "AND";
    case OR:                 return "OR";
    case XOR:                return "XOR";
    case NOT:                return "NOT";
    case LT:                 return "LT";
    case LE:                 return "LE";
    case GT:                 return "GT";
    case GE:                 return "GE";
    case NE:                 return "NE";
    case EQ:                 return "EQ";
    case LPRN:               return "LPRN";
    case RPRN:               return "RPRN";
    case PRNT:               return "PRNT";
    case WHILE:              return "WHILE";
    case LBRACE:             return "LBRACE";
    case RBRACE:             return "RBRACE";
    case TYPE_SPEC_INTEGER:  return "TYPE_SPEC_INTEGER";
    case TYPE_SPEC_BOOLEAN:  return "TYPE_SPEC_BOOLEAN";
    case ASSIGN:             return "ASSIGN";
    case IDENT:              return "IDENT";
    default: return "UNIDENTIFIED";
    }
}

void
lxr_print_token(int token_type,
                YYSTYPE * yylval,
                struct lxr_ctx * ctx)
{
    lxr_print(ctx, "%s", lxr_toktostr(token_type));

    // no additional value to print
    if (!yylval) {
        lxr_print(ctx, "\n");
        return;
    }

    // additional value to print
    switch (token_type) {
    case INTEGER: lxr_print(ctx, ": %d", yylval->INTEGER); break;
    case BOOLEAN: lxr_print(ctx, ": %s", yylval->BOOLEAN ? "True": "False"); break;
    case IDENT:   lxr_print(ctx, ": %s", yylval->IDENT); break;
    }
    lxr_print(ctx, "\n");
}

void lxr_updt_loc(YYLTYPE *yylloc, void *yyscanner)
{
    int yyleng   = yyget_leng(yyscanner);
    char *yytext = yyget_text(yyscanner);
    if (yylloc->last_line == 0) {
        yylloc->last_line   = 1;
        yylloc->last_column = 0;
    }
    yylloc->first_line   = yylloc->last_line;
    yylloc->first_column = yylloc->last_column;
    if (strchr(yytext, '\n')) {
        yylloc->last_line++;
        yylloc->last_column = 0;
    } else {
        yylloc->last_column += yyleng;
    }
}


int
lxr_process_proc(int token_type,
                 YYSTYPE * yylval,
                 YYLTYPE * yylloc,
                 void * yyscanner,
                 struct lxr_ctx * ctx)
{
    char *yytext = yyget_text(yyscanner);
    lxr_updt_loc(yylloc, yyscanner);

    switch (token_type) {
        /* Ignore cases */
    case LEX_BLNK:
    case LEX_CONT:
        return LEX_IGNR;

        /* Cases with Semantic Value */
    case INTEGER:
        if (yylval) yylval->INTEGER = atoi(yytext);
        lxr_print_token(INTEGER, yylval, ctx);
        return INTEGER;
    case BOOLEAN:
        if (yylval) yylval->BOOLEAN = str_to_bool(yytext);
        lxr_print_token(BOOLEAN, yylval, ctx);
        return BOOLEAN;

        /* Arithmetic */
    case ADD:  lxr_print_token(ADD, yylval, ctx); return ADD;
    case SUB:  lxr_print_token(SUB, yylval, ctx); return SUB;
    case MUL:  lxr_print_token(MUL, yylval, ctx); return MUL;
    case DIV:  lxr_print_token(DIV, yylval, ctx); return DIV;
    case MOD:  lxr_print_token(MOD, yylval, ctx); return MOD;

        /* Logical */
    case AND:  lxr_print_token(AND, yylval, ctx); return AND;
    case OR:   lxr_print_token(OR,  yylval, ctx); return OR;
    case XOR:  lxr_print_token(XOR, yylval, ctx); return XOR;

        /* Unary / Comparison */
    case NOT:  lxr_print_token(NOT, yylval, ctx); return NOT;
    case LT:   lxr_print_token(LT,  yylval, ctx); return LT;
    case LE:   lxr_print_token(LE,  yylval, ctx); return LE;
    case GT:   lxr_print_token(GT,  yylval, ctx); return GT;
    case GE:   lxr_print_token(GE,  yylval, ctx); return GE;
    case NE:   lxr_print_token(NE,  yylval, ctx); return NE;
    case EQ:   lxr_print_token(EQ,  yylval, ctx); return EQ;

        /* Declaration and assignment */
    case TYPE_SPEC_INTEGER: lxr_print_token(TYPE_SPEC_INTEGER,  yylval, ctx); return TYPE_SPEC_INTEGER;
    case TYPE_SPEC_BOOLEAN: lxr_print_token(TYPE_SPEC_BOOLEAN,  yylval, ctx); return TYPE_SPEC_BOOLEAN;
    case ASSIGN: lxr_print_token(ASSIGN,  yylval, ctx); return ASSIGN;

    case IDENT: {
        if (yylval) yylval->IDENT = strdup(yytext);
        lxr_print_token(IDENT, yylval, ctx);
        return IDENT;
    }


        /* Keywords */
    case PRNT: lxr_print_token(PRNT, yylval, ctx); return PRNT;
    case WHILE: lxr_print_token(WHILE, yylval, ctx); return WHILE;

    case LPRN:
        if (ctx) ctx->open_parens++;
        lxr_print_token(LPRN, yylval, ctx);
        return LPRN;

    case RPRN:
        if (ctx && ctx->open_parens > 0) ctx->open_parens--;
        lxr_print_token(RPRN, yylval, ctx);
        return RPRN;

    case LBRACE:
        if (ctx) ctx->open_braces++;
        lxr_print_token(LBRACE, yylval, ctx);
        return LBRACE;

    case RBRACE:
        if (ctx && ctx->open_braces > 0) ctx->open_braces--;
        lxr_print_token(RBRACE, yylval, ctx);
        return RBRACE;

    case NEWLINE:
        if (ctx && ctx->open_parens > 0) return LEX_IGNR;
        /* Actual Newline */
        lxr_print_token(NEWLINE, yylval, ctx);
        return token_type;

        /* Error */
    case LEX_ERR:
        fprintf(stderr,
                "[lexing error]: Unexpected %s at %d:%d-%d:%d\n",
                yytext, yylloc->first_line, yylloc->first_column,
                yylloc->last_line, yylloc->last_column);
        return LEX_ERR;
    }

    return LEX_ERR;
}
