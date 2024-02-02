CC = g++
SRCS = src/mainA.cpp assembler.cpp parser.c lexer.cpp
SRCL = src/mainL.cpp src/linker.cpp
SRCE = src/mainE.cpp src/emulator.cpp

lexer: spec/lexer.l inc/assembler.hpp
	flex -l spec/lexer.l
	mv lex.yy.c src

parser: spec/lexer.l spec/parser.y inc/assembler.hpp
	bison -d spec/parser.y
	mv parser.tab.c src
	mv parser.tab.h src

preas: lexer parser

test: src/assembler.cpp src/parser.tab.c src/lex.yy.c
	rm -rf *.o assembler
	g++ -o assembler src/assembler.cpp src/parser.tab.c src/lex.yy.c 
	
asmStart: clean preas test

assembler: $(SRCA) inc/assembler.hpp
	$(CC) $(SRCA) -g -o assembler

linker: $(SRCL) inc/linker.hpp
	$(CC) $(SRCL) -g -o linker

emulator: $(SRCE) inc/emulator.hpp
	$(CC) $(SRCE) -g -o emulator

clean:
		rm -rf *.o src/lex.yy.c src/parser.tab.c src/parser.tab.h my_test test2

