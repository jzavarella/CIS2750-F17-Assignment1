#include "../include/HelperFunctions.h"

/** Function to match the given string to the regex expression
  Returns 1 if the string matches the pattern is a match
  Returns 0 if the string does not match the pattern
*/
int match(const char* string, char* pattern) {
  int status;
  regex_t regex;

  if (regcomp(&regex, pattern, REG_EXTENDED|REG_NOSUB) != 0) {
    return 0;
  }

  status = regexec(&regex, string, (size_t) 0, NULL, 0);
  regfree(&regex);
  if (status != 0) {
    return 0;
  }
  return(1);
}

void safelyFreeString(char* c) {
  if (c) {
    free(c);
  }
}

char* extractSubstringBefore(char* line, char* terminator) {

  if (!line || !terminator) { // If the line is NULL return null
    return NULL;
  }

  int terminatorIndex = strcspn(line, terminator);
  size_t lineLength = strlen(line);

  if (terminatorIndex == lineLength) { // If there is no colon or the length is 0
    return NULL;
  }

  char* substring = malloc(terminatorIndex * sizeof(char) + 1); // Allocate that much room and add 1 for null terminator
  strncpy(substring, &line[0], terminatorIndex); // Copy 'size' chars into description starting from the index of ":" + 1
  substring[terminatorIndex] = '\0'; // Add null terminator because strncpy wont

  return substring;
}

char* extractSubstringAfter(char* line, char* terminator) {
  if (!line || !terminator) { // If the line is NULL return null
    return NULL;
  }

  int terminatorIndex = strcspn(line, terminator);
  size_t lineLength = strlen(line);

  if (terminatorIndex == lineLength) { // If there is no terminator or the length is 0
    return NULL;
  }

  size_t size = lineLength - (terminatorIndex + 1); // Calculate the number of chars in the final (+1 on terminator as to not include the colon)
  char* substring = malloc(size * sizeof(char) + 1); // Allocate that much room and add 1 for null terminator
  strncpy(substring, &line[terminatorIndex + 1], size); // Copy 'size' chars into description starting from the index of ":" + 1
  substring[size] = '\0'; // Add null terminator because strncpy wont
  return substring;
}

/**
  *Frees the list and returns the sent error code
*/
ErrorCode freeAndReturn(List* list, ErrorCode e) {
  clearList(list);
  return e;
}

void removeIntersectionOfLists(List* l1, List l2) {
  ListIterator eventIterator = createIterator(l2);
  char* line;
  Property* prop;
  while ((prop = nextElement(&eventIterator)) != NULL) {
    line = l1->printData(prop);
    deletePropertyFromList(l1, line); // Delete this line from the iCalendar line list and free the data
    safelyFreeString(line);
  }
}

Property* extractPropertyFromLine(char* line) {
  char* propName = extractSubstringBefore(line, ":");
  char* propDescr = extractSubstringAfter(line, ":");
  Property* p = createProperty(propName, propDescr);
  safelyFreeString(propName);
  safelyFreeString(propDescr);
  return p;
}

void deletePropertyFromList(List* propList, char* line) {
  Property* p = extractPropertyFromLine(line);
  safelyFreeString(deleteDataFromList(propList, p));
  free(p);
}
