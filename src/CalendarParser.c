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

char* printAlarm(void *toBePrinted) {
  Alarm* a = (Alarm*) toBePrinted;
  size_t finalSize = 0;
  char c;
  size_t i = 0;
  while ((c = a->action[i]) != '\0') {
    i ++;
  }
  finalSize += i;
  i = 0;
  while ((c = a->trigger[i]) != '\0') {
    i ++;
  }
  finalSize += i;
  finalSize += 4; // Add room for "|"
  char* propListString = toString(a->properties);
  // printf("%s\n", propListString);
  i = 0;
  while ((c = propListString[i]) != '\0') {
    i ++;
  }
  finalSize += i;
  char* string = malloc(finalSize + 1); // +1 to make room for NULL terminator
  strcpy(string, "|");
  strcat(string, a->action);
  strcat(string, "|");
  strcat(string, a->trigger);
  strcat(string, "|");
  strcat(string, propListString);
  strcat(string, "|");
  string[finalSize] = '\0'; // Set null terminator just in case strcat didnt
  safelyFreeString(propListString);
  return string;
}

//Comparing strings is done by strcmp
int compareAlarm(const void *first, const void *second){
  char* s1 = printProp((void*)first);
  char* s2 = printProp((void*)second);
  int result = strcmp(s1, s2);

  safelyFreeString(s1);
  safelyFreeString(s2);
	return result;
}

//Freeing a string is also straightforward
void deleteAlarmFunction(void *toBeDeleted){
  Alarm* a = (Alarm*) toBeDeleted;
  if (!a) {
    return; // Cant free nothing
  }
  if (a->trigger) {
    free(a->trigger);
  }
  clearList(&a->properties);
	free(a);
}

//Freeing a string is also straightforward
void deletePropertyFunction(void *toBeDeleted){
  Property* p = (Property*) toBeDeleted;
  if (!p) {
    return; // Cant free nothing
  }
	free(p);
}

Property* createProperty(char* propName, char* propDescr) {
  Property* p = malloc(sizeof(Property) + strlen(propDescr)*sizeof(char*) + strlen(propName)); // Allocate room for the property and the flexible array member
  strcpy(p->propName, propName);
  strcpy(p->propDescr, propDescr);
  return p;
}

Alarm* createAlarm(char* action, char* trigger, List properties) {
  if (!action || !trigger) {
    return NULL;
  }
  Alarm* alarm = malloc(sizeof(Alarm));
  strcpy(alarm->action, action);
  alarm->trigger = malloc(strlen(trigger) + 1);

  if (!alarm->trigger) {
    free(alarm);
    return NULL;
  }

  strcpy(alarm->trigger, trigger);
  alarm->properties = properties;

  return alarm;
}

Alarm* createAlarmFromPropList(List props) {
  List alarmProps = initializeList(&printProp, &deletePropertyFunction, &compareProp);
  ListIterator propsIterator = createIterator(props);

  char* ACTION = NULL;
  char* TRIGGER = NULL;

  Property* prop;
  while ((prop = nextElement(&propsIterator)) != NULL) {
    char* propName = prop->propName;
    char* propDescr = prop->propDescr;
    if (match(propName, "ACTION")) {
      if (ACTION || !propDescr || !match(propDescr, "^(AUDIO|DISPLAY|EMAIL)$")) {
        clearList(&alarmProps);
        return NULL; // Already have an ACTION or description is null
      }
      ACTION = propDescr;
    } else if (match(propName, "^TRIGGER$")) {
      if (TRIGGER || !propDescr) {
        clearList(&alarmProps);
        return NULL; // Already have trigger or description is null
      } else {
        TRIGGER = propDescr;
      }
    } else if (match(propName, "^REPEAT$")) {
      if (!match(propDescr, "^[0-9]+$")) {
        clearList(&alarmProps);
        return NULL; // Repeat must be an integer
      }
      Property* p = createProperty(propName, propDescr);
      insertBack(&alarmProps, p);
    }
  }

  if (!ACTION || !TRIGGER) {
    return NULL;
  }

  Alarm* a = createAlarm(ACTION, TRIGGER, alarmProps);
  return a;
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

Property* extractPropertyFromLine(char* line) {
  char* propName = NULL;
  char* propDescr = NULL;
  propName = extractSubstringBefore(line, ":");
  propDescr = extractSubstringAfter(line, ":");
  if (!propName || !propDescr) {
    propName = extractSubstringBefore(line, ";");
    propDescr = extractSubstringAfter(line, ";");
  }
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
ErrorCode readLinesIntoList(char* fileName, List* list, int bufferSize) {
  FILE* file; // Going to be used to store the file

  // If the fileName is NULL or does not match the regex expression *.ics or cannot be opened
  if (!fileName || !match(fileName, ".+\\.ics") || (file = fopen(fileName, "r")) == NULL) {
    return INV_FILE; // The file is invalid
  }

  char * line = NULL;
  size_t len = 0;
  ssize_t read;

  while ((read = getline(&line, &len, file)) != -1) {
    if (match(line, "^;")) {
      continue; // This is a line comment
    }
    if (match(line, "^[A-Z]+(:|;).*$")) {
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
  fclose(file);
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

ErrorCode extractBetweenTags(List props, List* extracted, ErrorCode onFailError, char* tag) {
  clearList(extracted); // Clear the list just in case
  ListIterator propsIterator = createIterator(props);

  size_t tagSize = strlen(tag);
  size_t beginTagSize = (strlen("^BEGIN:$") + tagSize) * sizeof(char);
  char beginTag[beginTagSize];
  strcpy(beginTag, "^BEGIN:");
  strcat(beginTag, tag);
  strcat(beginTag, "$");

  size_t endTagSize = (strlen("^END:$") + tagSize) * sizeof(char);
  char endTag[endTagSize];
  strcpy(endTag, "^END:");
  strcat(endTag, tag);
  strcat(endTag, "$");

  Property* prop;
  // These two ints count as flags to see if we have come across an event open/close
  int beginCount = 0;
  int endCount = 0;
  while ((prop = nextElement(&propsIterator)) != NULL) {
    char* line = props.printData(prop);
    if (match(line, beginTag)) {
      beginCount ++;
      if (beginCount != 1) {
        safelyFreeString(line);
        return onFailError; // Opened another event without closing the previous
      }
    } else if (match(line, endTag)) {
      endCount ++;
      if (endCount != beginCount) {
        safelyFreeString(line);
        return onFailError; // Closed an event without opening one
      }
      safelyFreeString(line);
      break; // We have parsed out the event
    } else if (beginCount == 1 && endCount == 0) { // If begin is 'open', add this line to the list
      Property* p = extractPropertyFromLine(line);
      insertBack(extracted, p);
    }
    safelyFreeString(line);
  }

  if (beginCount == 1 && endCount != beginCount) { // If there is no end to the VEVENT
    return onFailError;
  }

  return OK; // If we made it here, we have extracted the eventList
}

ErrorCode createTime(Event* event, char* timeString) {
  if (!timeString || !event || !match(timeString, "^[0-9]{8}T[0-9]{6}Z{0,1}$")) {
    return INV_CREATEDT;
  }
  char* numberDate = extractSubstringBefore(timeString, "T");
  char* timeS;
  char* temp = extractSubstringAfter(timeString, "T");
  if (match(timeString, "Z$")) { // If UTC
    timeS = extractSubstringBefore(temp, "Z"); // Get the time between the T and Z
    event->creationDateTime.UTC = true;
    safelyFreeString(temp); // Free temp
  } else {
    timeS = temp; // Get the time after T
    event->creationDateTime.UTC = false;
  }
  if (!numberDate || !temp || !timeS) {
    return INV_CREATEDT;
  }
  strcpy(event->creationDateTime.date, numberDate);
  strcpy(event->creationDateTime.time, timeS);
  safelyFreeString(numberDate);
  safelyFreeString(timeS);
  return OK;
}

void removeIntersectionOfLists(List* l1, List l2) {
  ListIterator eventIterator = createIterator(l2);
  char* line;
  Property* prop;
  while ((prop = nextElement(&eventIterator)) != NULL) {
    line = l1->printData(prop);
    deleteProperty(l1, line); // Delete this line from the iCalendar line list and free the data
    safelyFreeString(line);
  }
}

ErrorCode createEvent(List* eventList, Event* event) {
  if (!event) {
    return INV_EVENT;
  }
  ListIterator eventIterator = createIterator(*eventList);
  Property* prop;
  char* UID = NULL;
  char* DTSTAMP = NULL;
  while ((prop = nextElement(&eventIterator)) != NULL) {
    char* propName = prop->propName;
    char* propDescr = prop->propDescr;
    if (match(propName, "^UID$")) {
      if (UID != NULL || !propDescr || !strlen(propDescr)) {
        return INV_EVENT; // UID has already been assigned or propDesc is null or empty
      }
      UID = eventList->printData(prop);
      strcpy(event->UID, prop->propDescr);
    } else if (match(propName, "^DTSTAMP$")) {
      if (DTSTAMP != NULL || !propDescr || !strlen(propDescr)) {
        return INV_EVENT; // DTSTAMP has already been assigned or propDesc is null or empty
      }
      DTSTAMP = eventList->printData(prop); // Set the DTSTAMP flag
      ErrorCode e = createTime(event, propDescr);
      if (e != OK) {
        return e;
      }
    }
  }

  if (!UID || !DTSTAMP) { // If wecould not find UID or DTSTAMP
    return INV_EVENT;
  }

  deleteProperty(eventList, UID); // Delete UID from event properties
  deleteProperty(eventList, DTSTAMP); // Delete DTSTAMP from event properties
  safelyFreeString(UID); // Free stored UID
  safelyFreeString(DTSTAMP); // Free stored DTSTAMP

  List alarmPropList = initializeList(&printProp, &deletePropertyFunction, &compareProp);
  ErrorCode e = extractBetweenTags(*eventList, &alarmPropList, INV_EVENT, "VALARM");
  if (e != OK) {
    return freeAndReturn(&alarmPropList, e);
  }

  Alarm* a = createAlarmFromPropList(alarmPropList);
  if (!a) {
    return freeAndReturn(&alarmPropList, INV_EVENT);
  }
  List alarmList = initializeList(&printAlarm, &deleteAlarmFunction, &compareAlarm);
  insertBack(&alarmList, a);

  removeIntersectionOfLists(eventList, alarmPropList);
  deleteProperty(eventList, "BEGIN:VALARM");
  deleteProperty(eventList, "END:VALARM");

  clearList(&alarmPropList);

  event->alarms = alarmList;
  event->properties = *eventList;

  return OK;
}

ErrorCode parseEvent(List* iCalLines, Calendar* calendar) {

  // List that will store the lines between the VEVENT open and close tags
  List eventList = initializeList(&printProp, &deletePropertyFunction, &compareProp);

  ErrorCode errorCode = extractBetweenTags(*iCalLines, &eventList, INV_EVENT, "VEVENT");
  if (errorCode != OK) {
    return freeAndReturn(&eventList, errorCode);
  }

  deleteProperty(iCalLines, "BEGIN:VEVENT"); // Remove the begin event tag
  deleteProperty(iCalLines, "END:VEVENT"); // Remove the end event tag
  removeIntersectionOfLists(iCalLines, eventList); // Removes all properties from iCalLines that are in eventList

  Event* event = malloc(sizeof(Event));
  errorCode = createEvent(&eventList, event);
  if (errorCode != OK) {
    free(event);
    return freeAndReturn(&eventList, errorCode);
  }
  calendar->event = event;
  return OK;
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

  List iCalPropertyList = initializeList(&printProp, &deletePropertyFunction, &compareProp);
  ErrorCode lineCheckError = readLinesIntoList(fileName, &iCalPropertyList, 512); // Read the lines of the file into a list of properties

  if (lineCheckError != OK) { // If any of the lines were invalid, this will not return OK
    return freeAndReturn(&iCalPropertyList, lineCheckError);
  }
  
  if (!checkEnclosingTags(&iCalPropertyList)) { // Check to see if the enclosing lines are correct
    return freeAndReturn(&iCalPropertyList, INV_CAL); // Return invalid calendar if they are not
  }

  ErrorCode eventCode = parseEvent(&iCalPropertyList, *obj);
  if (eventCode != OK) { // If the event error code is not OK
    return freeAndReturn(&iCalPropertyList, eventCode); // Return the error
  }

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

  Event* event = obj->event;
  if (event) {
    List* props = &event->properties;
    if (props) {
      clearList(props);
    }
    List* alarms = &event->alarms;
    if (alarms) {
      clearList(alarms);
    }
    free(event);
  }

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
