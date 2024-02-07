lexer: spec/lexer.l inc/assembler.hpp
	flex -l spec/lexer.l
	mv lex.yy.c src

parser: spec/lexer.l spec/parser.y inc/assembler.hpp
	bison -d spec/parser.y
	mv parser.tab.c src
	mv parser.tab.h src

combine: src/assembler.cpp src/parser.tab.c src/lex.yy.c
	rm -rf *.o assembler
	g++ -o assembler src/assembler.cpp src/parser.tab.c src/lex.yy.c 
	
asembler: clean lexer parser combine

linker: src/linker.cpp inc/linker.hpp
	g++ src/linker.cpp -g -o linker

emulator: src/emulator.cpp inc/emulator.hpp
	g++ src/emulator.cpp -g -o emulator

clean:
		rm -rf *.o src/lex.yy.c src/parser.tab.c src/parser.tab.h assembler linker emulator izlaz.*

# ./linker -o banana.hex -hex main.o isr_software.o isr_terminal.o isr_timer.o math.o handler.o -place=banana@0xFF00FF00