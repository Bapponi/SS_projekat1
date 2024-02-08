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


a1:
	./assembler -o main.o main.s

a2:
	./assembler -o math.o math.s

a3:
	./assembler -o handler.o handler.s

a4:
	./assembler -o isr_software.o isr_software.s

a5:
	./assembler -o isr_terminal.o isr_terminal.s
	
a6:
	./assembler -o isr_timer.o isr_timer.s

linker1:
	./linker -o program.hex -hex handler.o math.o main.o isr_terminal.o isr_timer.o isr_software.o -place=my_code@0x00000500 -place=math@0x0000A00

emulator1:
	./emulator program.hex