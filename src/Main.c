#include <stdio.h>

#include "CalendarParser.h"

void test(char* fileName, ErrorCode expectedResult);

int main(int argc, char const *argv[]) {

  // printf("%d\n", match("thisshuld match!!~~", "^[^[:cntrl:][:space:]\x80-\xFF\"\\,:;]+$")); // Fails as expected because we are not including lower case letters
  //
  test(NULL, INV_FILE); // Filename is NULL
  test("", INV_FILE); // Empty String
  test("tests/doesnt_exist.ics", INV_FILE);
  test("tests/no_permissions.ics", INV_FILE); // Make sure you remove permissions
  test("tests/no_file_extension", INV_FILE);
  printf("----------------------\n");
  test("tests/blank.ics", INV_CAL);
  test("tests/just_open_tag.ics", INV_CAL);
  test("tests/just_close_tag.ics", INV_CAL);
  test("tests/just_open_close_tags.ics", INV_CAL);
  test("tests/no_prod_id.ics", INV_CAL);
  test("tests/no_version.ics", INV_CAL);
  test("tests/no_event.ics", INV_CAL);
  printf("----------------------\n");
  test("tests/duplicate_version.ics", DUP_VER);
  test("tests/malformed_version.ics", INV_VER);
  test("tests/blank_version.ics", INV_VER);
  printf("----------------------\n");
  test("tests/duplicate_prod_id.ics", DUP_PRODID);
  test("tests/invalid_prod_id.ics", INV_PRODID);
  test("tests/blank_prod_id.ics", INV_PRODID);
  printf("----------------------\n");
  test("tests/invalid_created_t.ics", INV_CREATEDT);
  test("tests/blank_created_t.ics", INV_CREATEDT);
  printf("----------------------\n");
  test("tests/no_created_t.ics", INV_EVENT);
  printf("----------------------\n");
  test("tests/valid_one_alarm.ics", OK);
  test("tests/valid_multiple_alarms.ics", OK);
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
