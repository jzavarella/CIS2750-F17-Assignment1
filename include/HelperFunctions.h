#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "LinkedListAPI.h"
#include "CalendarParser.h"

/** Function to match the given string to the regex expression
  Returns 1 if the string matches the pattern is a match
  Returns 0 if the string does not match the pattern
*/
int match(const char* string, char* pattern);

void safelyFreeString(char* c);

char* extractSubstringBefore(char* line, char* terminator);

char* extractSubstringAfter(char* line, char* terminator);

/**
  *Frees the list and returns the sent error code
*/
ErrorCode freeAndReturn(List* list, ErrorCode e);

void removeIntersectionOfLists(List* l1, List l2);

void deletePropertyFromList(List* propList, char* line);

Property* extractPropertyFromLine(char* line);
