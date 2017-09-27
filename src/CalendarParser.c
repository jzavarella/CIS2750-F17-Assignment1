/*
 * CIS2750 F2017
 * Assignment 1
 * Jackson Zavarella 0929350
 * This file parses iCalendar Files
 * No code was used from previous classes/ sources
 */

#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
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
int matchTEXTField(const char* propDescription);
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
void printList(List list);
Event* newEmptyEvent();
void clearManyLists(List** lists, size_t s);

List copyPropList(List toBeCopied) {
  List newList = initializeList(&printPropertyListFunction, &deletePropertyListFunction, &comparePropertyListFunction);
  ListIterator iter = createIterator(toBeCopied);
  Property* p;

  while ((p = nextElement(&iter)) != NULL) {
    char* c = toBeCopied.printData(p);
    Property* p = extractPropertyFromLine(c);
    insertBack(&newList, p);
    safelyFreeString(c);
  }
  return newList;
}

ErrorCode createEvent(List eventList, Event* event) {
  if (!event) {
    return INV_EVENT;
  }

  // Creating a property list that wil store all of the alarm props
  List alarmPropList = initializeList(&printPropertyListFunction, &deletePropertyListFunction, &comparePropertyListFunction);
  // Copy a new event list
  List newEventList = copyPropList(eventList);

  // While we still have VALARM tags
  while (extractBetweenTags(newEventList, &alarmPropList, INV_EVENT, "VALARM") != INV_EVENT) {
    Alarm* a = createAlarmFromPropList(alarmPropList);
    if (a) {
      // printf("trigger: %s\n", a->trigger);
      insertBack(&event->alarms, a);
      removeIntersectionOfLists(&newEventList, alarmPropList); // Remove all elements from alarmPropList in newEventList
      deleteProperty(&newEventList, "BEGIN:VALARM");
      deleteProperty(&newEventList, "END:VALARM");
    } else {
      clearList(&newEventList);
      clearList(&alarmPropList);
      return INV_EVENT;
    }
    clearList(&alarmPropList); // Clear the list
  }
  // ErrorCode alarmPropListError = extractBetweenTags(eventList, &alarmPropList, INV_EVENT, "VALARM");
  // printf("Alarm error: %s\n", printError(alarmPropListError));
  // printList(alarmPropList);

  // List alarmList = initializeList(&printAlarmListFunction, &deleteAlarmListFunction, &compareAlarmListFunction);


  // Alarm* a = createAlarmFromPropList(alarmPropList);
  // if (a) {
  //   insertBack(&event->alarms, a);
  //   removeIntersectionOfLists(&newEventList, alarmPropList);
  //   deleteProperty(&newEventList, "BEGIN:VALARM"); // Delete UID from event properties
  //   deleteProperty(&newEventList, "END:VALARM"); // Delete DTSTAMP from event properties
  // }

  ListIterator eventIterator = createIterator(newEventList);
  Property* prop;
  char* UID = NULL;
  char* DTSTAMP = NULL;
  while ((prop = nextElement(&eventIterator)) != NULL) {
    char* propName = prop->propName;
    char* propDescr = prop->propDescr;
    if (match(propName, "^UID$")) {
      if (UID != NULL || !propDescr || !strlen(propDescr)) {
        safelyFreeString(UID);
        safelyFreeString(DTSTAMP);
        clearList(&newEventList);
        return INV_EVENT; // UID has already been assigned or propDesc is null or empty
      }
      if (match(propDescr, "^(;|:)")) {
        char temp[strlen(propDescr)];
        memcpy(temp, propDescr + 1*sizeof(char), strlen(propDescr));
        strcpy(event->UID, temp);
      } else {
        strcpy(event->UID, propDescr);
      }

      UID = eventList.printData(prop);
    } else if (match(propName, "^DTSTAMP$")) {
      if (DTSTAMP != NULL || !propDescr || !strlen(propDescr)) {
        safelyFreeString(UID);
        safelyFreeString(DTSTAMP);
        clearList(&newEventList);
        return INV_EVENT; // DTSTAMP has already been assigned or propDesc is null or empty
      }
      DTSTAMP = eventList.printData(prop); // Set the DTSTAMP flag
      ErrorCode e = createTime(event, propDescr);
      if (e != OK) {
        safelyFreeString(UID); // Free stored UID
        safelyFreeString(DTSTAMP); // Free stored DTSTAMP
        clearList(&newEventList);
        return e;
      }
    }
  }

  if (!UID || !DTSTAMP) { // If wecould not find UID or DTSTAMP
    safelyFreeString(UID); // Free stored UID
    safelyFreeString(DTSTAMP); // Free stored DTSTAMP
    clearList(&newEventList);
    return INV_EVENT;
  }

  deleteProperty(&newEventList, UID); // Delete UID from event properties
  deleteProperty(&newEventList, DTSTAMP); // Delete DTSTAMP from event properties
  safelyFreeString(UID); // Free stored UID
  safelyFreeString(DTSTAMP); // Free stored DTSTAMP
  // printList(newEventList);
  // printList(alarmList);

  // List alarmPropList = initializeList(&printPropertyListFunction, &deletePropertyListFunction, &comparePropertyListFunction);
  // ErrorCode e = extractBetweenTags(eventList, &alarmPropList, INV_EVENT, "VALARM");
  // if (e != OK) {
  //   return freeAndReturn(&alarmPropList, e);
  // }
  //
  // List alarmList = initializeList(&printAlarmListFunction, &deleteAlarmListFunction, &compareAlarmListFunction);
  // //
  // Alarm* a = createAlarmFromPropList(alarmPropList);
  // if (a) {
  //   // return freeAndReturn(&alarmPropList, INV_EVENT);
  //   deleteProperty(&eventList, "BEGIN:VALARM");
  //   deleteProperty(&eventList, "END:VALARM");
  //   insertBack(&alarmList, a);
  // }
  //
  // removeIntersectionOfLists(&eventList, alarmPropList);

  event->properties = newEventList;
  clearList(&alarmPropList);

  // event->alarms = alarmList;

  return OK;
}

// ErrorCode parseEvent(List* iCalLines, Calendar* calendar) {
//
//   // List that will store the lines between the VEVENT open and close tags
//   List eventList = initializeList(&printPropertyListFunction, &deletePropertyListFunction, &comparePropertyListFunction);
//
//   ErrorCode errorCode = extractBetweenTags(*iCalLines, &eventList, INV_EVENT, "VEVENT");
//   if (errorCode != OK) {
//     return freeAndReturn(&eventList, errorCode);
//   }
//
//   deleteProperty(iCalLines, "BEGIN:VEVENT"); // Remove the begin event tag
//   deleteProperty(iCalLines, "END:VEVENT"); // Remove the end event tag
//   removeIntersectionOfLists(iCalLines, eventList); // Removes all properties from iCalLines that are in eventList
//
//   Event* event = malloc(sizeof(Event));
//   errorCode = createEvent(&eventList, event);
//   if (errorCode != OK) {
//     free(event);
//     return freeAndReturn(&eventList, errorCode);
//   }
//   calendar->event = event;
//   return OK;
// }

ErrorCode parseRequirediCalTags(List* list, Calendar* cal) {
  ListIterator iterator = createIterator(*list);
  Property* p;
  char* VERSION = NULL;
  char* PRODID = NULL;
  while ((p = nextElement(&iterator)) != NULL) {
    char* name = p->propName;
    char* description = p->propDescr;

    if (match(name, "^VERSION$")) {
      if (VERSION) {
        safelyFreeString(PRODID); // PROD might be allocated so we must remove it before returning
        safelyFreeString(VERSION); // Version might be allocated so we must remove it before returning
        return DUP_VER;
      }
      if (!description || !match(description, "^(:|;)[[:digit:]]+(\\.[[:digit:]]+)*$")) {
        safelyFreeString(PRODID); // PROD might be allocated so we must remove it before returning
        safelyFreeString(VERSION); // Version might be allocated so we must remove it before returning
        return INV_VER;
      }

      VERSION = malloc(strlen(description) * sizeof(char));
      memmove(VERSION, description+1, strlen(description)); // remove the first character as it is (; or :)
    } else if (match(name, "^PRODID$")) {
      if (PRODID) {
        safelyFreeString(PRODID); // PROD might be allocated so we must remove it before returning
        safelyFreeString(VERSION); // Version might be allocated so we must remove it before returning
        return DUP_PRODID;
      }

      if (!description || !matchTEXTField(description)) {
        safelyFreeString(PRODID); // PROD might be allocated so we must remove it before returning
        safelyFreeString(VERSION); // Version might be allocated so we must remove it before returning
        return INV_PRODID;
      }
      PRODID = malloc(strlen(description) * sizeof(char));
      memmove(PRODID, description+1, strlen(description)); // remove the first character as it is (; or :)
    }
  }

  if (!VERSION || !PRODID) {
    safelyFreeString(PRODID); // PROD might be allocated so we must remove it before returning
    safelyFreeString(VERSION); // Version might be allocated so we must remove it before returning
    return INV_CAL; // We are missing required tags
  }

  cal->version = atof(VERSION);
  strcpy(cal->prodID, PRODID);
  safelyFreeString(PRODID);
  safelyFreeString(VERSION);
  return OK;
}

void freeEvent(Event* event) {
  if (!event) {
    return; // Nothing to see here folks
  }
  clearList(&event->properties);
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
  Calendar* calendar = *obj;
  strcpy(calendar->prodID, ""); // Ensure that this field is not blank to prevent uninitialized conditinoal jump errors in valgrind

  Event* event = newEmptyEvent();
  calendar->event = event;
  List iCalPropertyList = initializeList(&printPropertyListFunction, &deletePropertyListFunction, &comparePropertyListFunction);
  ErrorCode lineCheckError = readLinesIntoList(fileName, &iCalPropertyList, 512); // Read the lines of the file into a list of properties

  if (lineCheckError != OK) { // If any of the lines were invalid, this will not return OK
    clearList(&iCalPropertyList);
    return lineCheckError;
  }

  List betweenVCalendarTags = initializeList(&printPropertyListFunction, &deletePropertyListFunction, &comparePropertyListFunction);

  ErrorCode betweenVCalendarTagsError = extractBetweenTags(iCalPropertyList, &betweenVCalendarTags, INV_CAL, "VCALENDAR");
  if (betweenVCalendarTagsError != OK) {
    clearList(&iCalPropertyList);
    clearList(&betweenVCalendarTags);
    return betweenVCalendarTagsError;
  }

  List betweenVEventTags = initializeList(&printPropertyListFunction, &deletePropertyListFunction, &comparePropertyListFunction);

  ErrorCode betweenVEventTagsError = extractBetweenTags(betweenVCalendarTags, &betweenVEventTags, INV_EVENT, "VEVENT");
  // Check to see if there is an event at all
  if (!betweenVEventTags.head || !betweenVEventTags.tail) {
    // clearManyLists([&iCalPropertyList, &betweenVCalendarTags, &betweenVEventTags], 3);
    clearList(&iCalPropertyList);
    clearList(&betweenVCalendarTags);
    clearList(&betweenVEventTags);
    return INV_CAL; // If there is no event, then the calendar is invalid
  }
  // If there is an event, check the event error
  if (betweenVEventTagsError != OK) {
    clearList(&iCalPropertyList);
    clearList(&betweenVCalendarTags);
    clearList(&betweenVEventTags);
    return betweenVEventTagsError;
  }

  ErrorCode eventError = createEvent(betweenVEventTags, event);
  if (eventError != OK) {
    clearList(&iCalPropertyList);
    clearList(&betweenVCalendarTags);
    clearList(&betweenVEventTags);
    return eventError;
  }

  removeIntersectionOfLists(&betweenVCalendarTags, betweenVEventTags);
  deleteProperty(&betweenVCalendarTags, "BEGIN:VEVENT");
  deleteProperty(&betweenVCalendarTags, "END:VEVENT");

  // // We should only have VERSION and PRODID now
  ErrorCode iCalIdErrors = parseRequirediCalTags(&betweenVCalendarTags, *obj); // Place UID and version in the obj
  if (iCalIdErrors != OK) {
    clearList(&iCalPropertyList);
    clearList(&betweenVCalendarTags);
    clearList(&betweenVEventTags);
    return iCalIdErrors;
  }


  clearList(&iCalPropertyList);
  clearList(&betweenVCalendarTags);
  clearList(&betweenVEventTags);
  return OK;
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
  if (event != NULL) {
    List* props = &event->properties;
    // printf("%p\n", props->head);
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

void updateLongestLineAndIncrementStringSize(size_t* longestLine, size_t* lineLength, size_t* stringSize) {
  if (*lineLength > *longestLine) {
    *longestLine = *lineLength;
  }
  *stringSize += *lineLength;
  *lineLength = 0;
}

void calculateLineLength(size_t* lineLength, const char* c, ... ) {
   va_list valist;
   va_start(valist, c);

   /* access all the arguments assigned to valist */
   while (c) {
        *lineLength += strlen(c);
        c = va_arg(valist, const char*);
    }
   /* clean memory reserved for valist */
   va_end(valist);
}

void concatenateLine(char* string, const char* c, ... ) {
   va_list valist;
   va_start(valist, c);

   /* access all the arguments assigned to valist */
   while (c) {
        strcat(string, c); // concatenate each value onto the string
        c = va_arg(valist, const char*);
    }
   /* clean memory reserved for valist */
   va_end(valist);
}

/** Function to create a string representation of a Calendar object.
 *@pre Calendar object exists, is not null, and is valid
 *@post Calendar has not been modified in any way, and a string representing the Calndar contents has been created
 *@return a string contaning a humanly readable representation of a Calendar object
 *@param obj - a pointer to a Calendar struct
**/
char* printCalendar(const Calendar* obj) {
  if (!obj) {
    return NULL; // If the object does not exist dont do anything
  }
  char* string;
  size_t stringSize = 0; // Total size of the completed string
  size_t lineLength = 0; // Size of the current line we are calculating
  size_t longestLine = 0; // Length of the lonest line

  // PRODUCT ID: Something\n
  if (strlen(obj->prodID) == 0) {
    return NULL; // Must have a prodID
  }
  calculateLineLength(&lineLength, "  PRODUCT ID: ", obj->prodID, "\n", NULL); // Add the length of these strings to the lineLength
  updateLongestLineAndIncrementStringSize(&longestLine, &lineLength, &stringSize); // Update the variable that stores the line with the greatest length

  // VERSION: 2.0\n
  if (!obj->version) {
    return NULL; // Must have a version
  }
  // Make room for the version string
  char vString[snprintf(NULL, 0, "%f", obj->version) + 1];
  snprintf(vString, sizeof(vString) + 1, "%f", obj->version);

  calculateLineLength(&lineLength, "  VERSION: ", vString, "\n", NULL); // Add the length of these strings to the lineLength
  updateLongestLineAndIncrementStringSize(&longestLine, &lineLength, &stringSize); // Update the variable that stores the line with the greatest length

  stringSize += 1; // newline

  if (obj->event) {
    Event* event = obj->event;
    calculateLineLength(&lineLength, " CALENDAR EVENT: \n" , NULL); // Add the length of these strings to the lineLength
    updateLongestLineAndIncrementStringSize(&longestLine, &lineLength, &stringSize); // Update the variable that stores the line with the greatest length

    // UID: some uid\n
    if (strlen(event->UID) == 0) {
      return NULL;
    }

    calculateLineLength(&lineLength, "  UID: ", event->UID, "\n" , NULL); // Add the length of these strings to the lineLength
    updateLongestLineAndIncrementStringSize(&longestLine, &lineLength, &stringSize); // Update the variable that stores the line with the greatest length

    // CREATION TIMESTAMP: some time\n
    char* dtString = printDatePretty(event->creationDateTime);
    calculateLineLength(&lineLength, "  CREATION TIMESTAMP: ", dtString, "\n" , NULL); // Add the length of these strings to the lineLength
    safelyFreeString(dtString);
    updateLongestLineAndIncrementStringSize(&longestLine, &lineLength, &stringSize); // Update the variable that stores the line with the greatest length

    List alarms = event->alarms;
    if (alarms.head) {
      ListIterator alarmIterator = createIterator(alarms);

      Alarm* a;
      while ((a = nextElement(&alarmIterator)) != NULL) { // Loop through each alarm
        calculateLineLength(&lineLength, "  ALARM: \n" , NULL); // Add the length of these strings to the lineLength
        updateLongestLineAndIncrementStringSize(&longestLine, &lineLength, &stringSize); // Update the variable that stores the line with the greatest length

        if (strlen(a->action) == 0) {
          return NULL; // Action is empty return null
        }
        // Get the length of the Action line
        calculateLineLength(&lineLength, "    ACTION: ", a->action, "\n" , NULL); // Add the length of these strings to the lineLength
        updateLongestLineAndIncrementStringSize(&longestLine, &lineLength, &stringSize); // Update the variable that stores the line with the greatest length

        if (strlen(a->trigger) == 0) {
          return NULL; // Action is empty return null
        }
        // Get the length of the Trigger line
        calculateLineLength(&lineLength, "    TRIGGER: ", a->trigger ,"\n" , NULL); // Add the length of these strings to the lineLength
        updateLongestLineAndIncrementStringSize(&longestLine, &lineLength, &stringSize); // Update the variable that stores the line with the greatest length

        List alarmProps = a->properties;
        // Output the props
        if (alarmProps.head) {
          calculateLineLength(&lineLength, "    ALARM PROPERTIES: \n" , NULL); // Add the length of these strings to the lineLength
          updateLongestLineAndIncrementStringSize(&longestLine, &lineLength, &stringSize); // Update the variable that stores the line with the greatest length

          // Get length of each property
          ListIterator propsIter = createIterator(alarmProps);
          Property* p;

          while ((p = nextElement(&propsIter)) != NULL) {
            char* printedProp = printPropertyListFunction(p); // Get the string for this prop
            calculateLineLength(&lineLength, "      ", printedProp, "\n", NULL); // Add the length of these strings to the lineLength
            updateLongestLineAndIncrementStringSize(&longestLine, &lineLength, &stringSize); // Update the variable that stores the line with the greatest length
            safelyFreeString(printedProp); // Free the string
          }
        }
      }
    }

    List propsList = event->properties;
    if (propsList.head) {

      calculateLineLength(&lineLength, "    EVENT PROPERTIES: \n", NULL); // Add the length of these strings to the lineLength
      updateLongestLineAndIncrementStringSize(&longestLine, &lineLength, &stringSize); // Update the variable that stores the line with the greatest length

      // Get length of each property
      ListIterator propsIter = createIterator(propsList);
      Property* p;
      while ((p = nextElement(&propsIter)) != NULL) {
        char* printedProp = printPropertyListFunction(p); // Get the string for this prop
        calculateLineLength(&lineLength, "    ", printedProp, "\n", NULL); // Add the length of these strings to the lineLength
        updateLongestLineAndIncrementStringSize(&longestLine, &lineLength, &stringSize); // Update the variable that stores the line with the greatest length
        safelyFreeString(printedProp); //  Free the string
      }
    }
  }

  longestLine += 1; // Make room for null terminator

  //Create the cap and footer of the string representation using by concatonating '-' n times for however long the longest line is
  char cap[longestLine];
  for (size_t i = 0; i < longestLine - 1; i++) {
    cap[i] = '-';
  }
  cap[longestLine - 1] = '\n';
  cap[longestLine] = '\0';

  string = malloc(stringSize * sizeof(char) + (2 * longestLine) + 1); // Allocate memory for the completed string

  strcpy(string, cap); // Header

  // PRODUCT ID: Something\n
  concatenateLine(string, " PRODUCT ID: ", obj->prodID, "\n", NULL);
  // VERSION: 2.0\n
  concatenateLine(string, " VERSION: ", vString, "\n", NULL);
  // newline
  concatenateLine(string, "\n", NULL);
  // CALENDAR EVENT:\n
  if (obj->event) {
    Event* event = obj->event;
    concatenateLine(string, " CALENDAR EVENT: \n", NULL);
    // UID: some uid\n
    concatenateLine(string, "   UID: ", event->UID, "\n", NULL);
    // CREATION TIMESTAMP: some time\n
    char* dtString = printDatePretty(event->creationDateTime);
    concatenateLine(string, "  CREATION TIMESTAMP: ", dtString, "\n", NULL);
    safelyFreeString(dtString);

    List alarms = event->alarms;
    if (alarms.head) {
      ListIterator alarmIterator = createIterator(alarms);

      Alarm* a;
      while ((a = nextElement(&alarmIterator)) != NULL) { // Loop through each alarm
        concatenateLine(string, "  ALARM: \n", NULL); // Alarm header
        concatenateLine(string, "    ACTION: ", a->action, "\n", NULL); // Alarm action
        concatenateLine(string, "    TRIGGER: ", a->trigger, "\n", NULL); // Alarm trigger

        List alarmProps = a->properties;
        // Output the props
        if (alarmProps.head) {
          concatenateLine(string, "    ALARM PROPERTIES: \n", NULL); // Alarm properties header

          // Get each property string
          ListIterator propsIter = createIterator(alarmProps);
          Property* p;

          while ((p = nextElement(&propsIter)) != NULL) {
            char* printedProp = printPropertyListFunction(p);
            concatenateLine(string, "      ", printedProp, "\n", NULL); // Alarm properties
            safelyFreeString(printedProp); // Free the string
          }
        }
      }
    }

    // EVENT PROPERTIES: \n
    List propsList = event->properties;
    if (propsList.head) {
      concatenateLine(string, "  EVENT PROPERTIES: \n", NULL); // Event properties header

      // print each property
      ListIterator propsIter = createIterator(propsList);
      Property* p;

      while ((p = nextElement(&propsIter)) != NULL) {
        char* printedProp = printPropertyListFunction(p);
        concatenateLine(string, "   ", printedProp, "\n", NULL); // Event properties
        safelyFreeString(printedProp);
      }
    }
  }

  strcat(string, cap); // Footer

  return string;
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

// <------START OF HELPER FUNCTIONS----->

/** Function to match the given string to the regex expression
  Returns 1 if the string matches the pattern is a match
  Returns 0 if the string does not match the pattern
*/
int match(const char* string, char* pattern) {
  int status;
  regex_t regex;
  int d;
  if ((d = regcomp(&regex, pattern, REG_EXTENDED|REG_NOSUB)) != 0) {
    return 0;
  }

  status = regexec(&regex, string, (size_t) 0, NULL, 0);
  regfree(&regex);
  if (status != 0) {
    return 0;
  }
  return(1);
}

int matchTEXTField(const char* propDescription) {
  return match(propDescription, "^(;|:)[^[:cntrl:]\"\\,:;]+$");
}

void safelyFreeString(char* c) {
  if (c) {
    free(c);
  }
}


char* printPropertyListFunction(void *toBePrinted) {
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
  char* string = malloc(finalSize + 1); // +1 to make room for NULL terminator
  strcpy(string, p->propName);
  strcat(string, p->propDescr);
  string[finalSize] = '\0'; // Set null terminator just in case strcat didnt
  return string;
}

int comparePropertyListFunction(const void *first, const void *second) {
  char* s1 = printPropertyListFunction((void*)first);
  char* s2 = printPropertyListFunction((void*)second);
  int result = strcmp(s1, s2);

  safelyFreeString(s1);
  safelyFreeString(s2);
	return result;
}

void deletePropertyListFunction(void *toBeDeleted) {
  Property* p = (Property*) toBeDeleted;
  if (!p) {
    return; // Cant free nothing
  }
	free(p);
}

char* printAlarmListFunction(void *toBePrinted) {
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

int compareAlarmListFunction(const void *first, const void *second) {
  //TODO: Create this
  return 0;
}

void deleteAlarmListFunction(void *toBeDeleted) {
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
  List alarmProps = initializeList(&printPropertyListFunction, &deletePropertyListFunction, &comparePropertyListFunction);
  ListIterator propsIterator = createIterator(props);

  char* ACTION = NULL;
  char* TRIGGER = NULL;

  Property* prop;
  while ((prop = nextElement(&propsIterator)) != NULL) {
    char* propName = prop->propName;
    char* propDescr = prop->propDescr;
    if (!propDescr) {
      clearList(&alarmProps);
      return NULL;
    }
    char tempDescription[strlen(propDescr) + 2];
    strcpy(tempDescription, propDescr);
    memmove(tempDescription, tempDescription+1, strlen(tempDescription)); // remove the first character as it is (; or :)
    if (match(propName, "^ACTION")) {
      if (ACTION || !match(tempDescription, "^(AUDIO|DISPLAY|EMAIL)$")) {
        clearList(&alarmProps);
        return NULL; // Already have an ACTION or description is null
      }
      ACTION = propDescr;
    } else if (match(propName, "^TRIGGER$")) {
      if (TRIGGER) {
        clearList(&alarmProps);
        return NULL; // Already have trigger or description is null
      } else {
        TRIGGER = propDescr;
      }
    } else if (match(propName, "^REPEAT$")) {
      if (!match(tempDescription, "^[[:digit:]]+$")) {
        clearList(&alarmProps);
        return NULL; // Repeat must be an integer
      }
      Property* p = createProperty(propName, propDescr);
      insertBack(&alarmProps, p);
    } else {
      Property* p = createProperty(propName, propDescr);
      insertBack(&alarmProps, p);
    }
  }

  if (!ACTION || !TRIGGER) {
    clearList(&alarmProps);
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
  char* propDescr = strpbrk(line, ":;");
  if (!propDescr) {
    return NULL;
  }
  propName = extractSubstringBefore(line, ":");
  if (!propName) {
    propName = extractSubstringBefore(line, ";");
    if (!propName) {
      return NULL;
    }
  }
  Property* p = createProperty(propName, propDescr);
  safelyFreeString(propName);
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
  if (!fileName || !match(fileName, ".+\\.ics$") || (file = fopen(fileName, "r")) == NULL) {
    return INV_FILE; // The file is invalid
  }

  char * line = NULL;
  size_t len = 0;
  ssize_t read;

  while ((read = getline(&line, &len, file)) != -1) {
    if (match(line, "^;")) {
      continue; // This is a line comment
    }
    if (match(line, "^[[:blank:]]+.+")) { // if this line starts with a space or tab and isnt blank
      Node* tailNode = list->tail;
      if (!tailNode) {
        fclose(file);
        return INV_CAL; // This is the first line in the list and the first line cannot be a line continuation
      }

      Property* p = (Property*) tailNode->data;
      if (!p) {
        fclose(file);
        return INV_CAL; // Something has gone wrong if the data is null
      }

      if (line[strlen(line) - 1] == '\n') { // Remove new line from end of line
        line[strlen(line) - 1] = '\0';
      }
      memmove(line, line + 1, strlen(line)); // Shift the contents of the string right by one as to not include the space
      size_t currentSize = sizeof(*p) + strlen(p->propDescr)*sizeof(char); // Calculatethe current size of the property
      size_t newSize = currentSize + strlen(line)*sizeof(char); // Calculate how much memory we need for the new property
      tailNode->data = realloc(p, newSize + 1); // Reallocate memory for the new property
      p = (Property*) tailNode->data;
      strcat(p->propDescr, line); // Concat this line onto the property description

      continue; // Continue to the next line
    }
    if (match(line, "^[[:alpha:]]+(:|;).*$")) {
      if (line[strlen(line) - 1] == '\n') { // Remove new line from end of line
        line[strlen(line) - 1] = '\0';
      }
      Property* p = extractPropertyFromLine(line);
      insertBack(list, p); // Insert the property into the list
    } else {
      safelyFreeString(line);
      fclose(file);
      return INV_CAL;
    }
  }

  safelyFreeString(line);
  fclose(file);

  if (!list->head) {
    return INV_CAL; // If the file was empty
  }
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

  if (beginCount == 0 && endCount == 0) {
    return onFailError;
  }

  if (beginCount == 1 && endCount != beginCount) { // If there is no end to the VEVENT
    return onFailError;
  }

  return OK; // If we made it here, we have extracted the eventList
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

char* printDatePretty(DateTime dt) {
  size_t size = 0;
  size += strlen(dt.date);
  size += strlen("// ");
  size += strlen(dt.time);
  if (dt.UTC) {
    size += strlen("Z");
  }

  char* string = malloc(size + 1 * (sizeof(char)));
  strcpy(string, dt.date);
  strcat(string, " ");
  strcat(string, dt.time);
  if (dt.UTC) {
    strcat(string, "Z");
  }

  return string;
}

ErrorCode createTime(Event* event, char* timeString) {
  if (!timeString || !event || !match(timeString, "^(:|;){0,1}[[:digit:]]{8}T[[:digit:]]{6}Z{0,1}$")) {
    return INV_CREATEDT;
  }
  char* numberDate;
  char* timeS;
  if (match(timeString, "^(:|;)")) {
    numberDate = extractSubstringBefore(&timeString[1], "T");
  } else {
    numberDate = extractSubstringBefore(timeString, "T");
  }
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

void printList(List list) {
  char* i = toString(list);
  printf("%s\n\n", i);
  free(i);
}

Event* newEmptyEvent() {
  Event* e = malloc(sizeof(Event));
  e->properties = initializeList(&printPropertyListFunction, &deletePropertyListFunction, &comparePropertyListFunction);
  e->alarms = initializeList(&printAlarmListFunction, &deleteAlarmListFunction, &compareAlarmListFunction);
  return e;
}

void clearManyLists(List** lists, size_t s) {
  for (size_t i = 0; i < s; i++) {
    clearList(lists[i]);
  }
}
