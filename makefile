CC = g++
SRCS = src/mainA.cpp assembler.cpp parser.c lexer.cpp
SRCL = src/mainL.cpp src/linker.cpp
SRCE = src/mainE.cpp src/emulator.cpp

lexer: spec/lexer.l inc/assembler.hpp
	flex spec/lexer.l

parser: spec/lexer.l spec/parser.y inc/assembler.hpp
	bison spec/parser.y

preas: lexer parser

assembler: $(SRCA) inc/assembler.hpp
	$(CC) $(SRCA) -g -o assembler

linker: $(SRCL) inc/linker.hpp
	$(CC) $(SRCL) -g -o linker

emulator: $(SRCE) inc/emulator.hpp
	$(CC) $(SRCE) -g -o emulator

clean:
		rm -rf *.o lexer.c lexer.h parser.c parser.h parser *.hex assembler linker emulator parser.tab.c

