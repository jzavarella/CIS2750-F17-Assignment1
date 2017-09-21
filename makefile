CC = gcc
CFLAGS = -Wall -std=c99 -g
LDFLAGS =
OBJFILES = src/Main.o src/LinkedListAPI.o src/CalendarParser.o src/HelperFunctions.o
TARGET = iCalendar

all: $(TARGET)

$(TARGET) : $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES) $(LDFLAGS)

run:
	./$(TARGET)

valgrind:
	valgrind --leak-check=full ./$(TARGET)

clean:
	rm -f $(OBJFILES) $(TARGET) *~ .DS_Store
	rm -rf $(TARGET).dSYM
