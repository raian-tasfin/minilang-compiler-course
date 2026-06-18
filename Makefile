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


#################
# Source Buffer #
#################
SRCBUF_DIR = srcbuf
SRCBUF_C   = $(SRCBUF_DIR)/srcbuf.c
SRCBUF_H   = $(SRCBUF_DIR)/srcbuf.h


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


#########
# Types #
#########
TYPES_DIR = types
SCALAR_H  = $(TYPES_DIR)/scalars.h
SCALAR_C  = $(TYPES_DIR)/scalars.c


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
AST_KIND_H   = $(AST_DIR)/ast_kind.h
AST_KIND_C   = $(AST_DIR)/ast_kind.c


################
# Symbol Table #
################
SYM_DIR = symtable
SYM_C   = $(SYM_DIR)/symtable.c
SYM_H   = $(SYM_DIR)/symtable.h


#####################
# Semantic Analyzer #
#####################
SEMAN_DIR = seman
SEMAN_C = $(SEMAN_DIR)/seman.c
SEMAN_H = $(SEMAN_DIR)/seman.h


###############################
# Intermediate Representation #
###############################
IR_DIR = intrep
IR_C   = $(IR_DIR)/interp.c
IR_H   = $(IR_DIR)/interp.h


###################
# Code Generation #
###################
CG_DIR = cg
CG_C = $(CG_DIR)/cg.c
CG_H = $(CG_DIR)/cg.h


#################
# Dynamic Array #
#################
DARR_DIR = darr
DARR_C = $(DARR_DIR)/darr.c
DARR_H = $(DARR_DIR)/darr.h

##########
# Bitset #
##########
BITSET_DIR = bitset
BITSET_C = $(BITSET_DIR)/bitset.c
BITSET_H = $(BITSET_DIR)/bitset.h



###########
# Boolean #
###########
BOOLEAN_DIR = boolean
BOOLEAN_C = $(BOOLEAN_DIR)/boolean.c
BOOLEAN_H = $(BOOLEAN_DIR)/boolean.h



##########
# C Code #
##########
MAIN_C = main.c

SRCS_C =
SRCS_C += $(MAIN_C)
SRCS_C += $(SRCBUF_C)
SRCS_C += $(LEXER_C)
SRCS_C += $(PARSER_C)
SRCS_C += $(LEXER_UTIL_C)
SRCS_C += $(AST_C)
SRCS_C += $(AST_KIND_C)
SRCS_C += $(CLI_C)
SRCS_C += $(SYM_C)
SRCS_C += $(DARR_C)
SRCS_C += $(BOOLEAN_C)
SRCS_C += $(SEMAN_C)
SRCS_C += $(IR_C)
# SRCS_C += $(CG_C)
SRCS_C += $(SYM_C)
SRCS_C += $(SCALAR_C)
SRCS_C += $(BITSET_C)

OBJS   = $(SRCS_C:%.c=%.o)

EXEC = minilang

###########
# Recipes #
###########
all: $(EXEC)


$(LEXER_C) $(LEXER_H): $(LEXER_SRC) $(PARSER_H)
	$(LEXER) --outfile=$(LEXER_C) --header-file=$(LEXER_H) $<

$(PARSER_C) $(PARSER_H): $(PARSER_SRC)
	$(PARSER) -d -o $(PARSER_C) $<

$(EXEC): $(OBJS)
	$(CC) -o $@ $^


##########################
# No-Action Dependencies #
##########################
$(MAIN_C): $(LEXER_H)
$(MAIN_C): $(PARSER_H)
$(MAIN_C): $(LEXER_UTIL_H)
$(MAIN_C): $(AST_H)
$(MAIN_C): $(CLI_H)
$(MAIN_C): $(SYM_H)
$(MAIN_C): $(IR_H)
$(MAIN_C): $(CG_H)
$(MAIN_C): $(DARR_H)
$(MAIN_C): $(BOOLEAN_H)
$(MAIN_C): $(SEMAN_H)
$(MAIN_C): $(SRCBUF_H)

$(LEXER_UTIL_H): $(PARSER_H) $(LEXER_H)
$(CLI_C): $(CLI_H)
$(SYM_C): $(SYM_H)
$(IR_C): $(IR_H)
$(CG_C): $(CG_H)
$(DARR_C): $(DARR_H)
$(BOOLEAN_C): $(BOOLEAN_H)
$(SEMAN_C): $(SEMAN_H)
$(SRCBUF_C): $(SRCBUF_H)
$(SYM_C): $(SYM_H)
$(SCALAR_C): $(SCALAR_H)
$(BITSET_C): $(BITSET_H)

clean:
	$(RM) $(LEXER_C)
	$(RM) $(LEXER_H)
	$(RM) $(PARSER_C)
	$(RM) $(PARSER_H)

	$(RM) $(OBJS)
	$(RM) $(EXEC)

	$(RM) $(REPORT_DIR)/*
