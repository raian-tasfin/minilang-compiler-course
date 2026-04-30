############
# Commands #
############
CC     = gcc
CFLAGS = -Wall -g

##########
# Files  #
##########
LEX    = lexer.l
YACC   = parser.y
CSRC   = main.c ast.c
LEX_C  = lex.yy.c
YACC_C = parser.tab.c
YACC_H = parser.tab.h
TARGET = minicompiler

all: $(TARGET)

$(TARGET): $(LEX_C) $(YACC_C) $(CSRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(LEX_C) $(YACC_C) $(CSRC)

$(LEX_C): $(LEX) $(YACC_H)
	flex $(LEX)

$(YACC_C) $(YACC_H): $(YACC)
	bison -d $(YACC)

clean:
	rm -f $(TARGET) $(LEX_C) $(YACC_C) $(YACC_H)
