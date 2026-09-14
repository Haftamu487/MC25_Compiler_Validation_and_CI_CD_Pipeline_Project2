CC = gcc
CFLAGS = -Wall -Wextra -g

all: mc25

mc25: lex.yy.c y.tab.c
	$(CC) $(CFLAGS) -o mc25 y.tab.c lex.yy.c symbol_table.c codegen.c ast.c -lfl

y.tab.c y.tab.h: parser.y
	bison -d -y parser.y

lex.yy.c: lexer.l y.tab.h
	flex lexer.l

clean:
	rm -f mc25 lex.yy.c y.tab.c y.tab.h

test: mc25
	./test_suite.sh

