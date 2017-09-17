/*

 * CIS2750 F2017

 * Assignment 0

 * Jackson Zavarella 0929350

 * This file contains the implementation of the linked List API.

 * No code was used from previous classes/ sources

 */


#include <regex.h>
#include "../include/CalendarParser.h"

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

//Printing a string requires a simple cast
char* printFunc(void *toBePrinted){
	return (char*)toBePrinted;
}

//Comparing strings is done by strcmp
int compareFunc(const void *first, const void *second){
	return strcmp((char*)first, (char*)second);
}

//Freeing a string is also straightforward
void deleteFunc(void *toBeDeleted){
	free(toBeDeleted);
}

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
List readLinesIntoList(FILE* file, int bufferSize) {
  List list = initializeList(&printFunc, &deleteFunc, &compareFunc);

  char lineBuffer[bufferSize]; // Buffer for the line to be read into
  char* line;
  while (fgets(lineBuffer, bufferSize, file) != NULL) {
    line = calloc(bufferSize * sizeof(char), 1); // Allocate enough memory for the line
    strcpy(line, lineBuffer); // Copy the contents into the line
    insertBack(&list, line); // Insert the line into the list
  }

  return list;
}

/**
  *Frees the list and returns the sent error code
*/
ErrorCode freeAndReturn(List* list, ErrorCode e) {
  clearList(list);
  return e;
}

/**
  *Checks to see if the first and last lines of the list are valid
  *Deletes the front and back nodes after because they do not contain any meaningful data
*/
int checkEnclosingTags(List* iCalLines) {
  char* line = getFromFront(*iCalLines); // Pull the first line off the list
  if (!match(line, "^BEGIN:VCALENDAR\n$")) { // Check if it is not valid
    return 0; // Retrun 0 (false)
  }
  free(deleteDataFromList(iCalLines, line)); // Delete the node contaning this line
  line = getFromBack(*iCalLines); // Pull the last line off the list
  if (!match(line, "^END:VCALENDAR\n$")) { // Check if line is not valid
    return 0; // Retrun 0 (false)
  }
  free(deleteDataFromList(iCalLines, line)); // Delete the node contaning this line

  return 1; // If we got here, return 1 (true)
}

ErrorCode extractEventList(List iCalLines, List* eventList) {
  clearList(eventList); // Clear the list just in case
  ListIterator calendarIterator = createIterator(iCalLines);

  char* line;
  // These two ints count as flags to see if we have come across an event open/close
  int beginCount = 0;
  int endCount = 0;
  while ((line = (char*)nextElement(&calendarIterator)) != NULL) {
    if (match(line, "^BEGIN:VEVENT\n$")) {
      beginCount ++;
      if (beginCount != 1) {
        return INV_EVENT; // Opened another event without closing the previous
      }
    } else if (match(line, "^END:VEVENT\n$")) {
      endCount ++;
      if (endCount != beginCount) {
        return INV_EVENT; // Closed an event without opening one
      }
      break; // We have parsed out the event
    } else if (beginCount == 1 && endCount == 0) { // If begin is 'open', add this line to the list
      int lineSize = strlen(line) * sizeof(char);
      char* tmp = calloc(lineSize, 1);
      strncpy(tmp, line, lineSize);
      insertBack(eventList, tmp);
    }
  }

  if (beginCount == 1 && endCount != beginCount) { // If there is no end to the VEVENT
    return INV_EVENT;
  }

  return OK; // If we made it here, we have extracted the eventList
}

ErrorCode parseEvent(List* iCalLines, Calendar* calendar) {

  // List that will store the lines between the VEVENT open and close tags
  List eventList = initializeList(&printFunc, &deleteFunc, &compareFunc);

  ErrorCode e = extractEventList(*iCalLines, &eventList);
  if (e != OK) {
    return freeAndReturn(&eventList, e);
  }

  free(deleteDataFromList(iCalLines, "BEGIN:VEVENT\n")); // Delete this line from the iCalendar line list and free the data
  free(deleteDataFromList(iCalLines, "END:VEVENT\n")); // Delete this line from the iCalendar line list and free the data

  ListIterator eventIterator = createIterator(eventList);

  char* line;
  while ((line = (char*)nextElement(&eventIterator)) != NULL) {


    free(deleteDataFromList(iCalLines, line)); // Delete this line from the iCalendar line list and free the data
  }

  return freeAndReturn(&eventList, OK);
}

/** Function to create a Calendar object based on the contents of an iCalendar file.
 *@pre File name cannot be an empty string or NULL.  File name must have the .ics extension.
       File represented by this name must exist and must be readable.
 *@post Either:
        A valid calendar has been created, its address was stored in the variable obj, and OK was returned
		or
		An error occurred, the calendar was not created, all temporary memory was freed, obj was set to NULL, and the
		appropriate error code was returned
 *@return the error code indicating success or the error encountered when parsing the calendar
 *@param fileName - a string containing the name of the iCalendar file
 *@param a double pointer to a Calendar struct that needs to be allocated
**/
ErrorCode createCalendar(char* fileName, Calendar** obj) {
  FILE* iCalFile; // Going to be used to store the file

  // If the fileName is NULL or does not match the regex expression *.ics or cannot be opened
  if (!fileName || !match(fileName, ".+\\.ics") || !(iCalFile = fopen(fileName, "r"))) {
    return INV_FILE; // The file is invalid
  }

  List iCalLines = readLinesIntoList(iCalFile, 512); // Read the lines of the file into a list of strings
  fclose(iCalFile); // Close the file since we are done with it

  if (!checkEnclosingTags(&iCalLines)) { // Check to see if the enclosing lines are correct
    return freeAndReturn(&iCalLines, INV_CAL); // Return invalid calendar if they are not
  }

  ErrorCode eventCode = parseEvent(&iCalLines, *obj);
  if (eventCode != OK) { // If the event error code is not OK
    return freeAndReturn(&iCalLines, eventCode); // Return the error
  }

  char* out = toString(iCalLines);
  printf("%s\n", out);
  free(out);

  return freeAndReturn(&iCalLines, OK); // All good, return OK
}


/** Function to delete all calendar content and free all the memory.
 *@pre Calendar object exists, is not null, and has not been freed
 *@post Calendar object had been freed
 *@return none
 *@param obj - a pointer to a Calendar struct
**/
void deleteCalendar(Calendar* obj) {
  if (!obj) {
    return; // No need to be freed if the object is NULL
  }

  /*
    TODO: make a function to delete an event
  */

  free(obj);
}


/** Function to create a string representation of a Calendar object.
 *@pre Calendar object exists, is not null, and is valid
 *@post Calendar has not been modified in any way, and a string representing the Calndar contents has been created
 *@return a string contaning a humanly readable representation of a Calendar object
 *@param obj - a pointer to a Calendar struct
**/
char* printCalendar(const Calendar* obj) {

  return NULL;
}


/** Function to "convert" the ErrorCode into a humanly redabale string.
 *@return a string contaning a humanly readable representation of the error code by indexing into
          the descr array using rhe error code enum value as an index
 *@param err - an error code
**/
const char* printError(ErrorCode err) {
  switch (err) {
    case OK: // OK
      return "OK";
    case INV_FILE: // INV_FILE
      return "Invalid File";
    case INV_CAL: // INV_CAL
      return "Invalid Calendar";
    case INV_VER: // INV_VER
      return "Malformed Version";
    case DUP_VER: // DUP_VER
      return "Non-Unique Version";
    case INV_PRODID: // INV_PRODID
      return "Malformed Product ID";
    case DUP_PRODID: // DUP_PRODID
      return "Non-Unique Product ID";
    case INV_EVENT: // INV_EVENT
      return "Malformed Event";
    case INV_CREATEDT: // INV_CREATEDT
      return "Malformed Date-Time";
    default:
      return "NULL";
  }
}
