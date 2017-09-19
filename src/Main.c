#include <stdio.h>

#include "../include/CalendarParser.h"

int main(int argc, char const *argv[]) {
// char* c = getLast(NULL);
// printf("%s\n", c);
// if (c) {
//   free(c);
// }

  // Property* p = createProperty("Hello", sizeof(char)*strlen("Hello"), "hello", sizeof(char)*strlen("hello"));
  // char* out = pprop(p);
  // printf("out: %s\nsize: %lu\n", out, strlen(out));
  // free(out);
  // compareProp(p, "BEGIN:VCALENDAR");

  Calendar* c = (Calendar*)malloc(sizeof(Calendar));
  printf("%s\n", printError(createCalendar("cal.ics", &c)));
  deleteCalendar(c);

  // FILE * fp;
  // char * line = NULL;
  // size_t len = 0;
  // ssize_t read;
  //
  // while ((read = getline(&line, &len, fp)) != -1) {
  //     printf("Retrieved line of length %zu :\n", read);
  //     printf("%s", line);
  // }
  // if (line) {
  //   free(line);
  // }


  // char* string = malloc(sizeof(char) * strlen("BEGIN:VCALENDAR"));
  // strcpy(string, "BEGIN:VCALENDAR");
  //
  // printf("string(%s) len:%lu size:%lu\n", string, strlen(string), sizeof(&string));
  // cleanLine(string);
  // char* first = getLast(line);
  // // printf("%s\n", line);
  // // line[strcspn(line, "\n")] = '\0';
  // printf("instance: %lu\nlength: %lu\n", strcspn(first, "\n"), strlen(first));
  //
  // printf("%s\n", first);
  // size_t s = sizeof(Property) + strlen("VCALENDAR") * sizeof(char);
  // Property* p = malloc(sizeof(*p) + sizeof(char[strlen("VCALENDAR")]));
  // printf("%lu\n", );
  // strcpy(p->propDescr, "VCALENDAR");
  // printf("%s\n", p->propDescr);
  // free(p);

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
