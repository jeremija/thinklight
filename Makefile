CFLAGS=-Wall -g
CC=gcc
EXECUTABLE=thinklight

.PHONY : all
all:
	$(CC) $(CFLAGS) -c thinklight.c logger/logger.c logger/string_allocation.c
	$(CC) $(CFLAGS) thinklight.o logger.o string_allocation.o -o $(EXECUTABLE)

.PHONY : clean
clean:
	rm -f $(EXECUTABLE)
	rm -f thinklight.o
	rm -f string_allocation.o
	rm -f logger.o
