CC = gcc
FLAGS = -g -Wall --std=c99
PROG = lmd
export PATH := $(PATH):$$HOME/myProgs

all : $(PROG)

clean :
	rm *.o

cleanall : clean
	rm $(PROG)

exe : $(PROG) clean

$(PROG) : main.o compiler.o preamble.o usefulFunctions.o pile.o files.o texToPdf.o
	$(CC) $(FLAGS) -o $(PROG) main.o compiler.o preamble.o usefulFunctions.o pile.o files.o texToPdf.o

main.o : main.c error.h compiler.o files.o texToPdf.o usefulFunctions.o
	$(CC) $(FLAGS) -c main.c

compiler.o : compiler.c compiler.h error.h preamble.o pile.o usefulFunctions.o files.o
	$(CC) $(FLAGS) -c compiler.c

preamble.o : preamble.h preamble.c error.h usefulFunctions.o
	$(CC) $(FLAGS) -c preamble.c

usefulFunctions.o : usefulFunctions.h usefulFunctions.c
	$(CC) $(FLAGS) -c usefulFunctions.c

pile.o : pile.h pile.c
	$(CC) $(FLAGS) -c pile.c

files.o : files.h files.c error.h
	$(CC) $(FLAGS) -c files.c

texToPdf.o : texToPdf.h texToPdf.c error.h files.o
	$(CC) $(FLAGS) -c texToPdf.c

install : $(PROG)
	mkdir -p ~/myProgs;
	cp lmd ~/myProgs
	@echo
	@echo 'Add this line to ~/.bashrc : "export PATH="$$PATH:$$HOME/myProgs"" and run "source ~/.bashrc"'
	@echo 'This will add directory "~/myProgs" to $$PATH variable, to be able to use "lmd" instead of "~/myProgs/lmd" in command line'
