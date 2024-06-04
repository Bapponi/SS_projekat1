lexer: misc/lexer.l inc/assembler.hpp
	flex -l misc/lexer.l
	mv lex.yy.c src

parser: misc/lexer.l misc/parser.y inc/assembler.hpp
	bison -d misc/parser.y
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
		rm -rf *.o *txt *hex src/lex.yy.c src/parser.tab.c src/parser.tab.h assembler linker emulator 

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

asembler1: a1 a2 a3 a4 a5 a6

linker1:
	./linker -o program.hex -hex handler.o math.o main.o isr_terminal.o isr_timer.o isr_software.o -place=my_code@0x40000000 -place=math@0xF0000000

emulator1:
	./emulator program.hex