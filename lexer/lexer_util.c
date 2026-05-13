#include "lexer_util.h"
#include "../lexer/lex.yy.h"


/**************/
/* Public API */
/**************/
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
        if (yylval) *yylval = atoi(yytext);
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
