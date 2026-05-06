%{
#include <stdio.h>
#include <stdlib.h>
int yylex(void);
void yyerror(const char *s);
%}

%define parse.error verbose

%token LBLOCK RBLOCK
%token IDENTIFIER ASSIGNMENT_OP
%token INTEGER_SPECIFIER BOOLEAN_SPECIFIER
%token PRINT LPAREN RPAREN
%token WHILE
%token IF ELSEIF ELSE
%token ADD SUB MUL DIV
%token LE EQ GE LEQ GEQ NEQ
%token NOT
%token NUMBER BOOLEAN NIL
%token NEWLINE
%token STRING_DELIM TEXT ESCAPE LBRACE RBRACE

/* resolve ambiguity: all binary ops left-associative, precedence low->high */
%left EQ NEQ
%left LE GE LEQ GEQ
%left ADD SUB
%left MUL DIV
%right NOT UMINUS

%%

program
    : group
    ;

group
    : group newlines statement
    | group newlines
    | statement
    | /* empty */
    ;

newlines
    : newlines NEWLINE
    | NEWLINE
    ;

block
    : LBLOCK newlines group newlines RBLOCK
    | LBLOCK group RBLOCK
    ;

statement
    : unit
    | block
    ;

unit
    : declaration
    | assignment
    | initialized_declaration
    | print_statement
    | if_group
    | while_group
    ;

/* ----- Declaration & Assignment ----- */

type_specifier
    : INTEGER_SPECIFIER
    | BOOLEAN_SPECIFIER
    ;

initialized_declaration
    : type_specifier IDENTIFIER ASSIGNMENT_OP expression
    ;

declaration
    : type_specifier IDENTIFIER
    ;

assignment
    : IDENTIFIER ASSIGNMENT_OP expression
    ;

/* ----- Print ----- */

print_statement
    : PRINT LPAREN fstring_literal RPAREN
    ;

/* ----- While ----- */

while_group
    : WHILE expression block
    ;

/* ----- If ladder ----- */

if_group
    : if_block elseif_ladder optional_else_block
    ;

elseif_ladder
    : elseif_ladder newlines elseif_block
    | /* empty */
    ;

optional_else_block
    : newlines else_block
    | /* empty */
    ;

if_block
    : IF expression block
    ;

elseif_block
    : ELSEIF expression block
    ;

else_block
    : ELSE block
    ;

/* ----- Expressions ----- */

expression
    : expression ADD expression
    | expression SUB expression
    | expression MUL expression
    | expression DIV expression
    | expression LE  expression
    | expression EQ  expression
    | expression GE  expression
    | expression LEQ expression
    | expression GEQ expression
    | expression NEQ expression
    | NOT expression
    | SUB expression  %prec UMINUS
    | LPAREN expression RPAREN
    | IDENTIFIER
    | NUMBER
    | BOOLEAN
    | NIL
    ;

/* ----- Fstring ----- */

fstring_literal
    : STRING_DELIM fstring_content STRING_DELIM
    ;

fstring_content
    : fstring_content fstring_part
    | /* empty */
    ;

fstring_part
    : TEXT
    | ESCAPE
    | LBRACE expression RBRACE
    ;

%%
