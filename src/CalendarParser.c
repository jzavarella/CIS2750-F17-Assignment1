/*

 * CIS2750 F2017

 * Assignment 0

 * Jackson Zavarella 0929350

 * This file contains the implementation of the linked List API.

 * No code was used from previous classes/ sources

 */


#include <regex.h>
#include <stdio.h>
#include <string.h>
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

/**
  *Checks to see if a string is allocated before freeing it
  *This function is used to save space
  TODO: REPLAE ALL STATEMENTS
*/
void safelyFreeString(char* c) {
  if (c) {
    free(c);
  }
}

char* printProp(void *toBePrinted) {
  Property* p = (Property*) toBePrinted;
  size_t finalSize = 0;
  char c;
  size_t i = 0;
  while ((c = p->propName[i]) != '\0') {
    i ++;
  }
  finalSize += i;
  i = 0;
  while ((c = p->propDescr[i]) != '\0') {
    i ++;
  }
  finalSize += i;
  finalSize += 1; // Add room for ":"
  char* string = malloc(finalSize + 1); // +1 to make room for NULL terminator
  strcpy(string, p->propName);
  strcat(string, ":");
  strcat(string, p->propDescr);
  string[finalSize] = '\0'; // Set null terminator just in case strcat didnt
  return string;
}

//Comparing strings is done by strcmp
int compareProp(const void *first, const void *second){
  char* s1 = printProp((void*)first);
  char* s2 = printProp((void*)second);
  int result = strcmp(s1, s2);

  safelyFreeString(s1);
  safelyFreeString(s2);
	return result;
}

//Freeing a string is also straightforward
void deleteFunc(void *toBeDeleted){
  Property* p = (Property*) toBeDeleted;
  if (!p) {
    return; // Cant free nothing
  }
	free(p);
}

char* extractPropertyName(char* line) {

  if (!line) { // If the line is NULL return null
    return NULL;
  }

  int colonIndex = strcspn(line, ":");
  size_t lineLength = strlen(line);

  if (colonIndex == lineLength) { // If there is no colon or the length is 0
    return NULL;
  }

  char* name = malloc(colonIndex * sizeof(char) + 1); // Allocate that much room and add 1 for null terminator
  strncpy(name, &line[0], colonIndex); // Copy 'size' chars into description starting from the index of ":" + 1
  name[colonIndex] = '\0'; // Add null terminator because strncpy wont

  return name;
}

char* extractPropertyDescription(char* line) {
  if (!line) { // If the line is NULL return null
    return NULL;
  }

  int colonIndex = strcspn(line, ":");
  size_t lineLength = strlen(line);

  if (colonIndex == lineLength) { // If there is no colon or the length is 0
    return NULL;
  }

  size_t size = lineLength - (colonIndex + 1); // Calculate the number of chars in the final (+1 on colon asto not include the colon)
  char* description = malloc(size * sizeof(char) + 1); // Allocate that much room and add 1 for null terminator
  strncpy(description, &line[colonIndex + 1], size); // Copy 'size' chars into description starting from the index of ":" + 1
  description[size] = '\0'; // Add null terminator because strncpy wont
  return description;
}

Property* createProperty(char* propName, char* propDescr) {
  Property* p = malloc(sizeof(Property) + strlen(propDescr)*sizeof(char*) + strlen(propName)); // Allocate room for the property and the flexible array member
  strcpy(p->propName, propName);
  strcpy(p->propDescr, propDescr);
  return p;
}

Property* extractPropertyFromLine(char* line) {
  char* propName = extractPropertyName(line);
  char* propDescr = extractPropertyDescription(line);
  Property* p = createProperty(propName, propDescr);
  safelyFreeString(propName);
  safelyFreeString(propDescr);
  return p;
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
ErrorCode readLinesIntoList(FILE* file, List* list, int bufferSize) {

  char * line = NULL;
  size_t len = 0;
  ssize_t read;

  while ((read = getline(&line, &len, file)) != -1) {
    if (match(line, "^[A-Z]+:.*$")) {
      if (line[strlen(line) - 1] == '\n') { // Remove new line from end of line
        line[strlen(line) - 1] = '\0';
      }
      Property* p = extractPropertyFromLine(line);
      insertBack(list, p); // Insert the property into the list
    } else {
      safelyFreeString(line);
      return INV_CAL;
    }
  }

  safelyFreeString(line);

  return OK;
}

/**
  *Frees the list and returns the sent error code
*/
ErrorCode freeAndReturn(List* list, ErrorCode e) {
  clearList(list);
  return e;
}

void deleteProperty(List* propList, char* line) {
  Property* p = extractPropertyFromLine(line);
  safelyFreeString(deleteDataFromList(propList, p));
  free(p);
}

/**
  *Checks to see if the first and last lines of the list are valid
  *Deletes the front and back nodes after because they do not contain any meaningful data
*/
int checkEnclosingTags(List* iCalLines) {
  Property* p = getFromFront(*iCalLines);
  if (!p) {
    return 0; // If we couldnt pull off the list
  }
  char* line = iCalLines->printData(p); // Pull the first line off the list
  if (!line) {
    return 0; // If we couldnt pull off the list
  }
  if (!match(line, "^BEGIN:VCALENDAR$")) { // Check if it is not valid
    safelyFreeString(line);
    return 0; // Retrun 0 (false)
  }

  deleteProperty(iCalLines, line); // Remove the begin tag as it has no meaningful information
  safelyFreeString(line);

  p = getFromBack(*iCalLines);
  if (!p) {
    return 0; // If we couldnt pull off the list
  }
  line = iCalLines->printData(p); // Pull the last line off the list
  if (!line) {
    return 0; // If we couldnt pull off the list
  }
  if (!match(line, "^END:VCALENDAR$")) { // Check if line is not valid
    safelyFreeString(line);
    return 0; // Retrun 0 (false)
  }
  deleteProperty(iCalLines, line); // Remove the end tag as it is no meaningful information anymore
  safelyFreeString(line);

  return 1; // If we got here, return 1 (true)
}

ErrorCode extractEventList(List iCalLines, List* eventList) {
  clearList(eventList); // Clear the list just in case
  ListIterator calendarIterator = createIterator(iCalLines);

  Property* prop;
  // These two ints count as flags to see if we have come across an event open/close
  int beginCount = 0;
  int endCount = 0;
  while ((prop = nextElement(&calendarIterator)) != NULL) {
    char* line = iCalLines.printData(prop);
    if (match(line, "^BEGIN:VEVENT$")) {
      beginCount ++;
      if (beginCount != 1) {
        safelyFreeString(line);
        return INV_EVENT; // Opened another event without closing the previous
      }
    } else if (match(line, "^END:VEVENT$")) {
      endCount ++;
      if (endCount != beginCount) {
        safelyFreeString(line);
        return INV_EVENT; // Closed an event without opening one
      }
      safelyFreeString(line);
      break; // We have parsed out the event
    } else if (beginCount == 1 && endCount == 0) { // If begin is 'open', add this line to the list
      Property* p = extractPropertyFromLine(line);
      insertBack(eventList, p);
    }
    safelyFreeString(line);
  }

  if (beginCount == 1 && endCount != beginCount) { // If there is no end to the VEVENT
    return INV_EVENT;
  }

  return OK; // If we made it here, we have extracted the eventList
}

ErrorCode parseEvent(List* iCalLines, Calendar* calendar) {

  // List that will store the lines between the VEVENT open and close tags
  List eventList = initializeList(&printProp, &deleteFunc, &compareProp);

  ErrorCode e = extractEventList(*iCalLines, &eventList);
  if (e != OK) {
    return freeAndReturn(&eventList, e);
  }

  char* out = toString(*iCalLines);
  printf("%s\n", out);
  free(out);

  deleteProperty(iCalLines, "BEGIN:VEVENT"); // Remove the begin event tag
  deleteProperty(iCalLines, "END:VEVENT"); // Remove the end event tag

  out = toString(*iCalLines);
  printf("%s\n", out);
  free(out);

  ListIterator eventIterator = createIterator(eventList);

  char* line;
  Property* prop;
  while ((prop = nextElement(&eventIterator)) != NULL) {
    line = eventList.printData(prop);


    deleteProperty(iCalLines, line); // Delete this line from the iCalendar line list and free the data
    safelyFreeString(line);
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

  List iCalPropertyList = initializeList(&printProp, &deleteFunc, &compareProp);
  ErrorCode lineCheckError = readLinesIntoList(iCalFile, &iCalPropertyList, 512); // Read the lines of the file into a list of properties
  fclose(iCalFile); // Close the file since we are done with it

  if (lineCheckError != OK) { // If any of the lines were invalid, this will not return OK
    return freeAndReturn(&iCalPropertyList, lineCheckError);
  }
  // Property* p = createProperty("BEGIN", sizeof(char)*strlen("BEGIN"), "VCALENDAR", sizeof(char)*strlen("VCALENDAR"));
  // compareProp(getFromFront(iCalPropertyList), getFromFront(iCalPropertyList));
  // char* out = toString(iCalPropertyList);
  // printf("%s\n", out);
  // free(out);

  if (!checkEnclosingTags(&iCalPropertyList)) { // Check to see if the enclosing lines are correct
    return freeAndReturn(&iCalPropertyList, INV_CAL); // Return invalid calendar if they are not
  }

  ErrorCode eventCode = parseEvent(&iCalPropertyList, *obj);
  if (eventCode != OK) { // If the event error code is not OK
    return freeAndReturn(&iCalPropertyList, eventCode); // Return the error
  }

  // char* out = toString(iCalPropertyList);
  // printf("%s\n", out);
  // free(out);

  return freeAndReturn(&iCalPropertyList, OK); // All good, return OK
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
