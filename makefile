CC = gcc
FLAGS = -g -Wall --std=c99
PROG = lmd

all : $(PROG)

clean :
	rm *.o

cleanall : clean
	rm $(PROG)

exe : $(PROG) clean

$(PROG) : main.o
	$(CC) $(FLAGS) -o $(PROG) main.o

main.o : main.c error.h
	$(CC) $(FLAGS) -c main.c
