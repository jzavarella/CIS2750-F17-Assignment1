CC = gcc
CFLAGS = -Wall -std=c11 -g

CALENDARPARSERC = src/CalendarParser.c
CALENDARO = src/CalendarParser.o
LIBCPARSE = bin/libcparse.a

LINKEDLISTC = src/LinkedListAPI.c
LISTO = src/LinkedListAPI.o
LIBLIST = bin/libllist.a

INCLUDES = include/

all:
	make list
	make parser

list:
	$(CC) $(CFLAGS) -c $(LINKEDLISTC) -o $(LISTO)
	ar cr $(LIBLIST) $(LISTO)

parser:
	$(CC) $(CFLAGS) -c $(CALENDARPARSERC) -o  $(CALENDARO) -I $(INCLUDES)
	ar cr $(LIBCPARSE) $(CALENDARO)

clean:
	rm -f $(LIBLIST) $(LIBCPARSE) $(CALENDARO) $(LISTO)
