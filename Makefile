###########
# General #
###########
RM = -rm -f

REPORT_DIR = report


#######
# CLI #
#######
CLI_DIR = cli
CLI_H   = $(CLI_DIR)/cli.h
CLI_C   = $(CLI_DIR)/cli.c


##########
# Lexing #
##########
LEXER = flex

LEXER_DIR = lexer
LEXER_SRC = $(LEXER_DIR)/lexer.l
LEXER_C   = $(LEXER_DIR)/lex.yy.c
LEXER_H   = $(LEXER_DIR)/lex.yy.h

LEXER_UTIL_H = $(LEXER_DIR)/lexer_util.h
LEXER_UTIL_C = $(LEXER_DIR)/lexer_util.c

##########
# Parser #
##########
PARSER = bison

PARSER_DIR = parser
PARSER_SRC = $(PARSER_DIR)/parser.y
PARSER_H   = $(PARSER_DIR)/parser.tab.h
PARSER_C   = $(PARSER_DIR)/parser.tab.c


#######
# AST #
#######
AST_DIR = ast
AST_H   = $(AST_DIR)/ast.h
AST_C   = $(AST_DIR)/ast.c


##########
# C Code #
##########
MAIN_C = main.c

SRCS_C =
SRCS_C += $(MAIN_C)
SRCS_C += $(LEXER_C)
SRCS_C += $(PARSER_C)
SRCS_C += $(LEXER_UTIL_C)
SRCS_C += $(AST_C)
SRCS_C += $(CLI_C)

OBJS   = $(SRCS_C:%.c=%.o)

EXEC = minilang

###########
# Recipes #
###########
all: $(EXEC)


$(LEXER_C) $(LEXER_H): $(LEXER_SRC)
	$(LEXER) --outfile=$(LEXER_C) --header-file=$(LEXER_H) $<

$(PARSER_C) $(PARSER_H): $(PARSER_SRC)
	$(PARSER) -d -o $(PARSER_C) $<

$(EXEC): $(OBJS)
	$(CC) -o $@ $^


##########################
# No-Action Dependencies #
##########################
$(MAIN_C): $(LEXER_H) $(PARSER_H) $(LEXER_UTIL_H) $(AST_H) $(CLI_H)
$(LEXER_UTIL_H): $(PARSER_H) $(LEXER_H)
$(CLI_C): $(CLI_H)


clean:
	$(RM) $(LEXER_C)
	$(RM) $(LEXER_H)
	$(RM) $(PARSER_C)
	$(RM) $(PARSER_H)

	$(RM) $(OBJS)
	$(RM) $(EXEC)

	$(RM) $(REPORT_DIR)/*
