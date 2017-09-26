CC = gcc
CFLAGS = -Wall -std=c11 -g
MAINC = src/Main.c
MAINO = src/Main.o

CALENDARPARSERC = src/CalendarParser.c
CALENDARO = src/CalendarParser.o
LIBCPARSE = bin/libcparse.a

LINKEDLISTC = src/LinkedListAPI.c
LISTO = src/LinkedListAPI.o
LIBLIST = bin/libllist.a

INCLUDES = include/
LIBS = -lllist -lcparse

TARGET = iCalendar

all:
	make list
	make parser
	make main

list:
	$(CC) -c $(LINKEDLISTC) -o $(LISTO)
	ar cr $(LIBLIST) $(LISTO)

parser:
	$(CC) -c $(CALENDARPARSERC) -o  $(CALENDARO) -I $(INCLUDES)
	ar cr $(LIBCPARSE) $(CALENDARO)

main:
	$(CC) $(MAINC) -o $(MAINO) -c -I $(INCLUDES)
	$(CC) $(MAINO) -o $(TARGET) -Lbin/ $(LIBS)

valgrind:
	valgrind --leak-check=full ./$(TARGET)

clean:
	rm -f $(LIBLIST) $(LIBCPARSE) $(CALENDARO) $(LISTO) $(MAINO) $(TARGET)
