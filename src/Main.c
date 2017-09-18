#include <stdio.h>

#include "../include/CalendarParser.h"

// Property* createProperty(char* propName, size_t nameSize, char propDescr[], size_t descrSize) {
//   size_t size = sizeof(Property) + descrSize*sizeof(char*) + nameSize;
//   Property* p = malloc(size);
//   strcpy(p->propName, propName);
//   strcpy(p->propDescr, propDescr);
//   return p;
// }

char* getLast(char* line) {
  if (!line) { // If the line is NULL return null
    return NULL;
  }

  int colonIndex = strcspn(line, ":");
  size_t lineLength = strlen(line);

  if (colonIndex == lineLength) { // If there is no colon or the length is 0
    return NULL;
  }

  char* description = malloc(colonIndex * sizeof(char) + 1); // Allocate that much room and add 1 for null terminator
  strncpy(description, &line[0], colonIndex); // Copy 'size' chars into description starting from the index of ":" + 1
  description[colonIndex] = '\0'; // Add null terminator because strncpy wont

  return description;
}

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
