CC = gcc
CFLAGS = -Wall -std=c11 -g

CALENDARPARSERC = src/CalendarParser.c
CALENDARPARSERH = include/CalendarParser.h
CALENDARO = src/CalendarParser.o
LIBCPARSE = bin/libcparse.a

LINKEDLISTC = src/LinkedListAPI.c
LINKEDLISTH = include/LinkedListAPI.h
LISTO = src/LinkedListAPI.o
LIBLIST = bin/libllist.a

INCLUDES = include/

all:
	make list
	make parser
	
list: $(LINKEDLISTC) $(LINKEDLISTH)
	$(CC) $(CFLAGS) -c $(LINKEDLISTC) -o $(LISTO) -I $(INCLUDES)
	ar cr $(LIBLIST) $(LISTO)

parser: $(LINKEDLISTC) $(LINKEDLISTH) $(CALENDARPARSERC) $(CALENDARPARSERH)
	$(CC) $(CFLAGS) -c $(CALENDARPARSERC) -o  $(CALENDARO) -I $(INCLUDES)
	ar cr $(LIBCPARSE) $(CALENDARO)

clean:
	rm -f $(LIBLIST) $(LIBCPARSE) $(CALENDARO) $(LISTO)
