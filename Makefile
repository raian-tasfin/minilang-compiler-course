####################
# General Commands #
####################
RM = -rm -f


##########
# Lexing #
##########
LEXER = flex

LEXER_DIR = lexer
LEXER_SRC = $(LEXER_DIR)/lexer.l
LEXER_C   = $(LEXER_DIR)/lex.yy.c
LEXER_H   = $(LEXER_DIR)/lex.yy.h


##########
# C Code #
##########
MAIN_C = main.c
SRCS_C = $(MAIN_C) $(LEXER_C)
OBJS   = $(SRCS_C:%.c=%.o)

EXEC = minilang

###########
# Recipes #
###########
all: $(EXEC)


$(LEXER_C) $(LEXER_H): $(LEXER_SRC)
	$(LEXER) --outfile=$(LEXER_C) --header-file=$(LEXER_H) $<

$(EXEC): $(OBJS)
	$(CC) -o $@ $^


##########################
# No-Action Dependencies #
##########################
$(MAIN_C): $(LEXER_H)

clean:
	$(RM) $(LEXER_C)
	$(RM) $(LEXER_H)
	$(RM) $(OBJS)
	$(RM) $(EXEC)
