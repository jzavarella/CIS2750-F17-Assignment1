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

run:
	./$(TARGET)

list:
	$(CC) $(CFLAGS) -c $(LINKEDLISTC) -o $(LISTO)
	ar cr $(LIBLIST) $(LISTO)

parser:
	$(CC) $(CFLAGS) -c $(CALENDARPARSERC) -o  $(CALENDARO) -I $(INCLUDES)
	ar cr $(LIBCPARSE) $(CALENDARO)

main:
	$(CC) $(CFLAGS) $(MAINC) -o $(MAINO) -c -I $(INCLUDES)
	$(CC) $(CFLAGS) $(MAINO) -o $(TARGET) -Lbin/ $(LIBS)

valgrind:
	valgrind --leak-check=full ./$(TARGET)

clean:
	rm -f $(LIBLIST) $(LIBCPARSE) $(CALENDARO) $(LISTO) $(MAINO) $(TARGET)
