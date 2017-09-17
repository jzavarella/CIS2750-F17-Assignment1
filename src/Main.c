#include <stdio.h>

#include "../include/CalendarParser.h"

int main(int argc, char const *argv[]) {
  Calendar* c = (Calendar*)malloc(sizeof(Calendar));
  printf("%s\n", printError(createCalendar("cal.ics", &c)));
  deleteCalendar(c);

  // Test error codes
  // const char* error;
  // int i;
  // for (i = 0; i <= 9; i++) {
  //   error = printError(i);
  //   if (error) {
  //     printf("%s\n", error);
  //   }
  // }
  return 0;
}
