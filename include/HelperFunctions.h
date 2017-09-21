#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "LinkedListAPI.h"
#include "CalendarParser.h"

// Print function for property list
char* printPropertyListFunction(void *toBePrinted);
// Compare function for property list
int comparePropertyListFunction(const void *first, const void *second);
// Delete function for property list
void deletePropertyListFunction(void *toBeDeleted);
// Print function for alarm list
char* printAlarmListFunction(void *toBePrinted);
// Compare function for alarm list
int compareAlarmListFunction(const void *first, const void *second);
// Delete functino for alarm list
void deleteAlarmListFunction(void *toBeDeleted);

int match(const char* string, char* pattern);
void safelyFreeString(char* c);
Property* createProperty(char* propName, char* propDescr);
Alarm* createAlarm(char* action, char* trigger, List properties);
Alarm* createAlarmFromPropList(List props);
char* extractSubstringBefore(char* line, char* terminator);
char* extractSubstringAfter(char* line, char* terminator);
Property* extractPropertyFromLine(char* line);

/**
  *Takes a file that has been opened and reads the lines into a linked list of chars*
  *with each node being one line of the file.
  *@param: file
  * The file to be read. Note the file must be open
  *@param: bufferSize
  * The maximum size of each line
  *@return: list
  * The list with each line read into it
*/
ErrorCode readLinesIntoList(char* fileName, List* list, int bufferSize);

// Frees the list and returns the sent error code
ErrorCode freeAndReturn(List* list, ErrorCode e);
// Deletes a property from a list given the string representation of it
void deleteProperty(List* propList, char* line);
ErrorCode extractBetweenTags(List props, List* extracted, ErrorCode onFailError, char* tag);

/**
  *Checks to see if the first and last lines of the list are valid
  *Deletes the front and back nodes after because they do not contain any meaningful data
*/
int checkEnclosingTags(List* iCalLines);
char* printDatePretty(DateTime dt);
ErrorCode createTime(Event* event, char* timeString);
void removeIntersectionOfLists(List* l1, List l2);
