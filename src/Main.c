#include <stdio.h>

#include "../include/CalendarParser.h"

void test(char* fileName, ErrorCode expectedResult);

int main(int argc, char const *argv[]) {

  test("tests/doesnt_exist.ics", INV_FILE);
  test("tests/no_file_extension", INV_FILE);
  printf("----------------------\n");
  test("tests/blank.ics", INV_CAL);
  test("tests/just_open_tag.ics", INV_CAL);
  test("tests/just_close_tag.ics", INV_CAL);
  test("tests/just_open_close_tags.ics", INV_CAL); // TODO: FIX THIS!
  test("tests/no_prod_id.ics", INV_CAL);
  test("tests/no_version_id.ics", INV_CAL);
  printf("----------------------\n");
  test("tests/duplicate_version.ics", DUP_VER);
  test("tests/malformed_version.ics", INV_VER);
  printf("----------------------\n");
  test("tests/duplicate_prod_id.ics", DUP_PRODID);
  printf("----------------------\n");
  test("tests/no_event.ics", INV_EVENT);
  printf("----------------------\n");
  test("tests/valid_no_alarm.ics", OK);

  Calendar* c = (Calendar*)malloc(sizeof(Calendar));
  ErrorCode e = createCalendar("cal.ics", &c);
  printf("%s\n", printError(e));
  char* out = printCalendar(c);
  printf("%s\n", out);
  free(out);
  deleteCalendar(c);
  return 0;
}

void test(char* fileName, ErrorCode expectedResult) {
  Calendar* c = (Calendar*)malloc(sizeof(Calendar));
  ErrorCode e = createCalendar(fileName, &c);
  if (e != expectedResult) {
    printf("**FAIL**: %s %s was expected but recieved %s\n", fileName, printError(expectedResult), printError(e));
  } else {
    printf("PASS: %s %s was expected\n", fileName, printError(expectedResult));
  }
  if (c) {
    deleteCalendar(c);
  }
}
