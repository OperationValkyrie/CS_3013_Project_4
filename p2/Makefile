# Jonathan Chang																																																																																																																																																																																																																																																																																																																																																																																												# Jonathan Chang
# CS 3013 C02
# Project 2

CFLAGS = -Wall -g

all: vm

vm: vm.o
	gcc $(CFLAGS) vm.o -o vm -lm

vm.o: vm.c vm.h
	gcc $(CFLAGS) -c vm.c

clean:
	rm -f *.o vm