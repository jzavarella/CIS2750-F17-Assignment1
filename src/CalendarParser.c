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
#include "../include/CalendarParser.h"
#include "../include/HelperFunctions.h"

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

  List alarmPropList = initializeList(&printPropertyListFunction, &deletePropertyListFunction, &comparePropertyListFunction);
  ErrorCode alarmPropListError = extractBetweenTags(eventList, &alarmPropList, INV_EVENT, "VALARM");
  printf("Alarm error: %s\n", printError(alarmPropListError));
  printList(alarmPropList);

  List newEventList = copyPropList(eventList);
  List alarmList = initializeList(&printAlarmListFunction, &deleteAlarmListFunction, &compareAlarmListFunction);
  Alarm* a = createAlarmFromPropList(alarmPropList);
  if (a) {
    List l = a->properties;
    printList(l);
    insertBack(&alarmList, a);
    removeIntersectionOfLists(&newEventList, alarmPropList);
    deleteProperty(&newEventList, "BEGIN:VALARM"); // Delete UID from event properties
    deleteProperty(&newEventList, "END:VALARM"); // Delete DTSTAMP from event properties
  } else {
    printf("%s\n", "Alarm is null");
  }

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
        return INV_EVENT; // DTSTAMP has already been assigned or propDesc is null or empty
      }
      DTSTAMP = eventList.printData(prop); // Set the DTSTAMP flag
      ErrorCode e = createTime(event, propDescr);
      if (e != OK) {
        return e;
      }
    }
  }

  if (!UID || !DTSTAMP) { // If wecould not find UID or DTSTAMP
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
  clearList(&alarmPropList);

  event->alarms = alarmList;
  event->properties = newEventList;

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
      if (!description || !match(description, "^(:|;).+$")) {
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
  if (betweenVEventTagsError != OK) {
    clearList(&iCalPropertyList);
    clearList(&betweenVCalendarTags);
    return betweenVEventTagsError;
  }

  Event* event = malloc(sizeof(Event));
  ErrorCode eventError = createEvent(betweenVEventTags, event);
  if (eventError != OK) {
    clearList(&iCalPropertyList);
    clearList(&betweenVCalendarTags);
    return eventError;
  }

  Calendar* calendar = *obj;

  calendar->event = event;

  // printList(betweenVCalendarTags);
  removeIntersectionOfLists(&betweenVCalendarTags, betweenVEventTags);
  deleteProperty(&betweenVCalendarTags, "BEGIN:VEVENT");
  deleteProperty(&betweenVCalendarTags, "END:VEVENT");
  // printList(betweenVCalendarTags);

  // printList(event->alarms);

  // if (!checkEnclosingTags(&iCalPropertyList)) { // Check to see if the enclosing lines are correct
  //   return freeAndReturn(&iCalPropertyList, INV_CAL); // Return invalid calendar if they are not
  // }
  //
  // ErrorCode eventCode = parseEvent(&iCalPropertyList, *obj);
  // if (eventCode != OK) { // If the event error code is not OK
  //   return freeAndReturn(&iCalPropertyList, eventCode); // Return the error
  // }
  //
  // // We should only have VERSION and PRODID now
  ErrorCode iCalIdErrors = parseRequirediCalTags(&betweenVCalendarTags, *obj); // Place UID and version in the obj
  if (iCalIdErrors != OK) {
    return freeAndReturn(&iCalPropertyList, iCalIdErrors);
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
  if (!obj) {
    return NULL;
  }
  char* string;
  size_t stringSize = 0;
  size_t lineLength = 0;
  size_t longestLine = 0;

  // PRODUCT ID: Something\n
  if (strlen(obj->prodID) == 0) {
    return NULL;
  }
  lineLength += strlen(" PRODUCT ID: \n");
  lineLength += strlen(obj->prodID);

  if (lineLength > longestLine) {
    longestLine = lineLength;
  }
  stringSize += lineLength;
  lineLength = 0;

  // VERSION: 2.0\n
  if (!obj->version) {
    return NULL;
  }
  char vString[snprintf(NULL, 0, "%f", obj->version) + 1];
  snprintf(vString, sizeof(vString) + 1, "%f", obj->version);
  lineLength += strlen(" VERSION: \n"); // VERSION:
  lineLength += strlen(vString); // 2.0

  if (lineLength > longestLine) {
    longestLine = lineLength;
  }
  stringSize += lineLength;
  lineLength = 0;

  stringSize += 1; // newline

  if (obj->event) {
    Event* event = obj->event;
    lineLength += strlen(" CALENDAR EVENT: \n");
    if (lineLength > longestLine) {
      longestLine = lineLength;
    }
    stringSize += lineLength;
    lineLength = 0;

    // UID: some uid\n
    if (strlen(event->UID) == 0) {
      return NULL;
    }
    lineLength += strlen("  UID: \n");
    lineLength += strlen(event->UID);
    if (lineLength > longestLine) {
      longestLine = lineLength;
    }
    stringSize += lineLength;
    lineLength = 0;

    // CREATION TIMESTAMP: some time\n
    char* dtString = printDatePretty(event->creationDateTime);
    lineLength += strlen("  CREATION TIMESTAMP: \n");
    lineLength += strlen(dtString);
    safelyFreeString(dtString);
    if (lineLength > longestLine) {
      longestLine = lineLength;
    }
    stringSize += lineLength;
    lineLength = 0;

    List propsList = event->properties;
    if (propsList.head) {
      lineLength += strlen("  EVENT PROPERTIES: \n");
      if (lineLength > longestLine) {
        longestLine = lineLength;
      }
      stringSize += lineLength;
      lineLength = 0;

      // Get length of each property
      ListIterator propsIter = createIterator(propsList);
      Property* p;

      while ((p = nextElement(&propsIter)) != NULL) {
        char* printedProp = printPropertyListFunction(p);
        lineLength += strlen(printedProp);
        lineLength += strlen("   \n"); // Make room for tabs and new line
        if (lineLength > longestLine) {
          longestLine = lineLength;
        }
        stringSize += lineLength;
        lineLength = 0;

        safelyFreeString(printedProp);
      }
    }
  }

  longestLine += 1;
  char cap[longestLine];
  for (size_t i = 0; i < longestLine - 1; i++) {
    cap[i] = '-';
  }
  cap[longestLine - 1] = '\n';
  cap[longestLine] = '\0';
  string = malloc(stringSize * sizeof(char) + (2 * longestLine) + 1);


  strcpy(string, cap); // Header
  // PRODUCT ID: Something\n
  strcat(string, " PRODUCT ID: ");
  strcat(string, obj->prodID);
  strcat(string, "\n");
  // VERSION: 2.0\n
  strcat(string, " VERSION: ");
  strcat(string, vString);
  strcat(string, "\n");
  // newline
  strcat(string, "\n");
  // CALENDAR EVENT:\n
  if (obj->event) {
    Event* event = obj->event;
    strcat(string, " CALENDAR EVENT: \n");
    // UID: some uid\n
    strcat(string, "  UID: ");
    strcat(string, event->UID);
    strcat(string, "\n");
    // CREATION TIMESTAMP: some time\n
    char* dtString = printDatePretty(event->creationDateTime);
    strcat(string, "  CREATION TIMESTAMP: ");
    strcat(string, dtString);
    strcat(string, "\n");
    safelyFreeString(dtString);
    // EVENT PROPERTIES: \n
    List propsList = event->properties;
    if (propsList.head) {
      strcat(string, "  EVENT PROPERTIES: \n");

      // print each property
      ListIterator propsIter = createIterator(propsList);
      Property* p;

      while ((p = nextElement(&propsIter)) != NULL) {
        char* printedProp = printPropertyListFunction(p);
        strcat(string, "   ");
        strcat(string, printedProp);
        strcat(string, "\n");
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
