#include <stdio.h>

#include "../include/CalendarParser.h"

int main(int argc, char const *argv[]) {
  const char* error;
  // printf("%s\n", error);
  int i;
  for (i = 0; i <= 9; i++) {
    error = printError(i);
    if (error) {
      printf("%s\n", error);
    }
  }
  // printf("%d\n", OK);
  return 0;
}
