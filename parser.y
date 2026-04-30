%{
#include <stdio.h>
#include <stdlib.h>
#include "parser.tab.h"

extern int yylex();
extern int line_num;
void yyerror(const char *s);
%}

%code requires {
#include "ast.h"
}


%define parse.error verbose

/**
 * Token storage
 */
%union {
    int num;
    char * str;
    ASTNode * node;
}
/***
 * Types
 */
%type <node> expression statement declaration assignment print_stmt statement_list

/**
 * Token definitions
 */
%token VAR PRINT ASSIGN_OP PLUS MINUS MUL DIV LPAREN RPAREN NEWLINE
%token <str> IDENTIFIER
%token <num> NUMBER

/**
 * Precedence
 */
%left PLUS MINUS
%left MUL DIV

/**
 * Starting rule
 */
%start program

%%

/**
 * Grammar
 */

program:
    statement_list {
        ast_root = $1;
    }
    | /* empty program */ {
        printf("Empty program.\n");
    }
    ;

statement_list:
    statement {
        $$ = $1;
    }
    | statement_list NEWLINE statement {
        ASTNode * cur = $1;
        while (cur->next) {
            cur = cur->next;
        }
        cur->next = $3;
        $$ = $1;
    }
    | statement_list NEWLINE {
        $$ = $1;
    }
    ;

statement:
    declaration  {$$ = $1; }
    | assignment {$$ = $1; }
    | print_stmt {$$ = $1; }
    ;

declaration:
    VAR IDENTIFIER {
        $$ = ast_declare($2);
        free($2);
    }
    | VAR IDENTIFIER ASSIGN_OP expression {
        $$ = ast_declare_assign($2, $4);
        free($2);
    }
    ;

assignment:
    IDENTIFIER ASSIGN_OP expression {
        $$ = ast_assign($1, $3);
        free($1);
    }
    ;

print_stmt:
    PRINT LPAREN expression RPAREN {
        $$ = ast_print($3);
    }
    ;

expression:
    NUMBER                        { $$ = ast_number($1); }
    | IDENTIFIER                  { $$ = ast_identifier($1); free($1); }
    | expression PLUS  expression { $$ = ast_binop(NODE_ADD, $1, $3); }
    | expression MINUS expression { $$ = ast_binop(NODE_SUB, $1, $3); }
    | expression MUL   expression { $$ = ast_binop(NODE_MUL, $1, $3); }
    | expression DIV   expression { $$ = ast_binop(NODE_DIV, $1, $3); }
    | LPAREN expression RPAREN    { $$ = $2; }
    ;

%%

/**
 * Error
 */
void yyerror(const char *s) {
    fprintf(stderr, "Syntax Error on line %d: %s\n", line_num, s);
}
