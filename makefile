CC = gcc
CFLAGS = -Wall -std=c11
LDFLAGS =
OBJFILES = src/Main.o src/LinkedListAPI.o src/CalendarParser.o
TARGET = iCalendar

all: $(TARGET)

$(TARGET) : $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES) $(LDFLAGS)

run:
	./$(TARGET)

valgrind:
	valgrind ./$(TARGET)

clean:
	rm -f $(OBJFILES) $(TARGET) *~ .DS_Store
	rm -rf $(TARGET).dSYM
