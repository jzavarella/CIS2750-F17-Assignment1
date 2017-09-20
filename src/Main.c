#include <stdio.h>

#include "../include/CalendarParser.h"

int main(int argc, char const *argv[]) {
  Calendar* c = (Calendar*)malloc(sizeof(Calendar));
  printf("%s\n", printError(createCalendar("cal.ics", &c)));
  deleteCalendar(c);
  return 0;
}
