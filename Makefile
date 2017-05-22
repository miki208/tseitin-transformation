PROG = main
CC = g++
CCFLAGS = -std=c++11 -g
LEXER = flex
PARSER = bison

$(PROG): main.o prop_logic.o parser.o lexer.o
	$(CC) $(CCFLAGS) -o $@ $^

main.o: main.cpp prop_logic.h
	$(CC) $(CCFLAGS) -c -o $@ $<

prop_logic.o : prop_logic.cpp prop_logic.h
	$(CC) $(CCFLAGS) -c -o $@ $<

parser.o: parser.cpp prop_logic.h
	$(CC) $(CCFLAGS) -c -o $@ $<

lexer.o: lexer.cpp parser.hpp prop_logic.h
	$(CC) $(CCFLAGS) -c -o $@ $<

parser.cpp: parser.ypp
	$(PARSER) -d -o $@ $<

lexer.cpp: lexer.lpp
	$(LEXER) -o $@ $<

clean:
	rm -f *.o
	rm -f *~
	rm -f parser.cpp
	rm -f lexer.cpp
	rm -f parser.hpp
	rm -f $(PROG)
	rm -f *.swp

