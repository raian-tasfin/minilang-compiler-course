%{
#include <stdio.h>
#include <stdlib.h>
#include "parser.tab.h"

extern int yylex();
extern int line_num;
void yyerror(const char *s);
%}

%define parse.error verbose

/**
 * Token storage
 */
%union {
    int num;
    char* str;
}

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
        printf("Successfully parsed a complete PROGRAM.\n");
    }
    | /* empty program */ {
        printf("Empty program.\n");
    }
    ;

statement_list:
    statement_list NEWLINE statement
    | statement
    | statement_list NEWLINE
    ;

statement:
    declaration
    | assignment
    | print_stmt
    ;

declaration:
    VAR IDENTIFIER {
        printf("Parsed Declaration: var %s\n", $2);
        free($2);
    }
    | VAR assignment {
        printf("Parsed Declaration with Assignment.\n");
    }
    ;

assignment:
    IDENTIFIER ASSIGN_OP expression {
        printf("Parsed Assignment: %s :=\n", $1);
        free($1);
    }
    ;

print_stmt:
    PRINT LPAREN expression RPAREN {
        printf("Parsed Print Statement.\n");
    }
    ;

expression:
    expression PLUS expression    { printf(" [Math: Add] "); }
    | expression MINUS expression { printf(" [Math: Sub] "); }
    | expression MUL expression   { printf(" [Math: Mul] "); }
    | expression DIV expression   { printf(" [Math: Div] "); }
    | LPAREN expression RPAREN    { printf(" [Math: Parentheses] "); }
    | IDENTIFIER                  { printf(" [Var: %s] ", $1); free($1); }
    | NUMBER                      { printf(" [Num: %d] ", $1); }
    ;

%%

/**
 * Error
 */
void yyerror(const char *s) {
    fprintf(stderr, "Syntax Error on line %d: %s\n", line_num, s);
}
