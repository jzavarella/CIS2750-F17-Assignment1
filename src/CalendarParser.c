/*
 * CIS2750 F2017
 * Assignment 1
 * Jackson Zavarella 0929350
 * This file parses iCalendar Files
 * No code was used from previous classes/ sources
 */

#define _GNU_SOURCE

#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "CalendarParser.h"
#include "HelperFunctions.h"

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
  *obj = malloc(sizeof(Calendar));
  Calendar* calendar = *obj;
  strcpy(calendar->prodID, ""); // Ensure that this field is not blank to prevent uninitialized conditinoal jump errors in valgrind
  // calendar->version = -1;
  Event* event = newEmptyEvent(); // Create an empty event
  calendar->event = event; // Assign the empty event

  List iCalPropertyList = initializeList(&printPropertyListFunction, &deletePropertyListFunction, &comparePropertyListFunction); // Create a list to store all properties/ lines
  ErrorCode lineCheckError = readLinesIntoList(fileName, &iCalPropertyList, 512); // Read the lines of the file into a list of properties
  if (lineCheckError != OK) { // If any of the lines were invalid, this will not return OK
    clearList(&iCalPropertyList); // Clear the list before returning
    return lineCheckError; // Return the error that was produced
  }

  // List to store calendar properties
  List betweenVCalendarTags = initializeList(&printPropertyListFunction, &deletePropertyListFunction, &comparePropertyListFunction);
  // Get the properties between event tags
  ErrorCode betweenVCalendarTagsError = extractBetweenTags(iCalPropertyList, &betweenVCalendarTags, INV_CAL, "VCALENDAR");
  if (betweenVCalendarTagsError != OK) { // If there was a problem parsing
    clearList(&iCalPropertyList); // Free lists before returning
    clearList(&betweenVCalendarTags);
    return betweenVCalendarTagsError; // Return the error that was produced
  }

  // List to store event properties
  List betweenVEventTags = initializeList(&printPropertyListFunction, &deletePropertyListFunction, &comparePropertyListFunction);
  // Extract all properties between event tags
  ErrorCode betweenVEventTagsError = extractBetweenTags(betweenVCalendarTags, &betweenVEventTags, INV_EVENT, "VEVENT");
  // Check to see if there is an event at all
  if (!betweenVEventTags.head || !betweenVEventTags.tail) {
    clearList(&iCalPropertyList); // Clear lists before returning
    clearList(&betweenVCalendarTags);
    clearList(&betweenVEventTags);
    return INV_CAL; // If there is no event, then the calendar is invalid
  }
  // If there is an event, check the event error
  if (betweenVEventTagsError != OK) {
    clearList(&iCalPropertyList); // Clear lists before returning
    clearList(&betweenVCalendarTags);
    clearList(&betweenVEventTags);
    return betweenVEventTagsError; // Return the error that was produced
  }

  ErrorCode eventError = createEvent(betweenVEventTags, event); // Create and event given the event properties that were extracted
  if (eventError != OK) { // If there was a problem
    clearList(&iCalPropertyList); // Clear list before returning
    clearList(&betweenVCalendarTags);
    clearList(&betweenVEventTags);
    return eventError; // Return the error that was produced
  }

  removeIntersectionOfLists(&betweenVCalendarTags, betweenVEventTags); // Remove the event properties from the calendar tags
  deleteProperty(&betweenVCalendarTags, "BEGIN:VEVENT"); // Remove the begin tag
  deleteProperty(&betweenVCalendarTags, "END:VEVENT"); // Remove the begin tag

  // // We should only have VERSION and PRODID now
  ErrorCode iCalIdErrors = parseRequirediCalTags(&betweenVCalendarTags, *obj); // Place UID and version in the obj
  if (iCalIdErrors != OK) { // If there was a problem
    clearList(&iCalPropertyList); // Clear lists before returning
    clearList(&betweenVCalendarTags);
    clearList(&betweenVEventTags);
    return iCalIdErrors; // Return the error that was produced
  }


  clearList(&iCalPropertyList); // Clear lists before returning
  clearList(&betweenVCalendarTags);
  clearList(&betweenVEventTags);
  return OK; // Congratulations you parsed the file. Return OK
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
  if (event != NULL) { // If there is an event
    List* props = &event->properties; // Grab the props
    if (props) { // If there are props
      clearList(props); // Set them free
    }
    List* alarms = &event->alarms; // Grab the alarms
    if (alarms) { // If the alarms exist
      clearList(alarms); // Set them free
    }
    free(event); // Set it free
  }

  free(obj); // Free object? I Like free objects
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

  return string; // Beam me up
}


/** Function to "convert" the ErrorCode into a humanly redabale string.
 *@return a string contaning a humanly readable representation of the error code by indexing into
          the descr array using rhe error code enum value as an index
 *@param err - an error code
**/
char* printError(ErrorCode err) {
  char* error;
  switch (err) {
    case OK: // OK
      error = malloc(sizeof("OK"));
      strcpy(error, "OK");
      break;
    case INV_FILE: // INV_FILE
      error = malloc(sizeof("Invalid File"));
      strcpy(error, "Invalid File");
      break;
    case INV_CAL: // INV_CAL
      error = malloc(sizeof("Invalid Calendar"));
      strcpy(error, "Invalid Calendar");
      break;
    case INV_VER: // INV_VER
      error = malloc(sizeof("Malformed Version"));
      strcpy(error, "Malformed Version");
      break;
    case DUP_VER: // DUP_VER
      error = malloc(sizeof("Duplicate Version"));
      strcpy(error, "Duplicate Version");
      break;
    case INV_PRODID: // INV_PRODID
      error = malloc(sizeof("Malformed Product ID"));
      strcpy(error, "Malformed Product ID");
      break;
    case DUP_PRODID: // DUP_PRODID
      error = malloc(sizeof("Duplicate Product ID"));
      strcpy(error, "Duplicate Product ID");
      break;
    case INV_EVENT: // INV_EVENT
      error = malloc(sizeof("Invalid Event"));
      strcpy(error, "Invalid Event");
      break;
    case INV_CREATEDT: // INV_CREATEDT
      error = malloc(sizeof("Malformed Date"));
      strcpy(error, "Malformed Date");
      break;
    case OTHER_ERROR:
      error = malloc(sizeof("Generic Error")); // This error may be generic, but you aren't :)
      strcpy(error, "Generic Error");
      break;
    default:
      error = malloc(sizeof("NULL"));
      strcpy(error, "NULL");
      break;
  }
  return error;
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
  if ((d = regcomp(&regex, pattern, REG_EXTENDED|REG_NOSUB|REG_ICASE)) != 0) {
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
  return match(propDescription, "^(;|:){0,1}[^[:cntrl:]\"\\,:;]+$"); // This regex matches valid text characters
}

// Check if the string is allocated before freeing it
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
    i ++; // Calculate length of the prop name
  }
  finalSize += i; // add it to the total
  i = 0; // reset
  while ((c = p->propDescr[i]) != '\0') {
    i ++; // Calculate length of description
  }
  finalSize += i; // add it up again
  if (p->propDescr[0] != ';' && p->propDescr[0] != ':') {
    finalSize ++;
  }
  char* string = calloc(finalSize + 1, 1); // +1 to make room for NULL terminator
  strcpy(string, p->propName);
  if (p->propDescr[0] != ';' && p->propDescr[0] != ':') {
    strcat(string, ":");
  }
  strcat(string, p->propDescr);
  string[finalSize] = '\0'; // Set null terminator just in case strcat didnt
  return string;
}

int comparePropertyListFunction(const void *first, const void *second) {
  char* s1 = printPropertyListFunction((void*)first); // Print each prop
  char* s2 = printPropertyListFunction((void*)second);
  int result = strcmp(s1, s2); // Compare the strings

  safelyFreeString(s1); // BECAUSE I'M FREEEEE!!!!! FREE FALLIN'
  safelyFreeString(s2);
	return result; // Return the result
}

void deletePropertyListFunction(void *toBeDeleted) {
  Property* p = (Property*) toBeDeleted;
  if (!p) {
    return; // Cant free nothing. Someone must have done our job before we got here >.>
  }
	free(p);
}

// This method never gets called but whatev
char* printAlarmListFunction(void *toBePrinted) {
  Alarm* a = (Alarm*) toBePrinted;
  size_t finalSize = 0;
  char c;
  size_t i = 0;
  while ((c = a->action[i]) != '\0') {
    i ++; // Calculate size of action
  }
  finalSize += i;
  i = 0;
  while ((c = a->trigger[i]) != '\0') {
    i ++; // Calculate size of trigger
  }
  finalSize += i;
  finalSize += 4; // Add room for "|"
  char* propListString = toString(a->properties); // Print the properties
  i = 0;
  while ((c = propListString[i]) != '\0') {
    i ++; // Calculate size of list string
  }
  finalSize += i;
  // Mash it all up together
  char* string = malloc(finalSize + 1); // +1 to make room for NULL terminator
  strcpy(string, "|");
  strcat(string, a->action);
  strcat(string, "|");
  strcat(string, a->trigger);
  strcat(string, "|");
  strcat(string, propListString);
  strcat(string, "|");
  string[finalSize] = '\0'; // Set null terminator just in case strcat didnt
  safelyFreeString(propListString); // Bye felicia
  return string; // Return the string
}

// This hasnt been required yet
int compareAlarmListFunction(const void *first, const void *second) {
  char* s1 = printAlarmListFunction((void*)first); // Print each prop
  char* s2 = printAlarmListFunction((void*)second);
  int result = strcmp(s1, s2); // Compare the strings

  safelyFreeString(s1); // BECAUSE I'M FREEEEE!!!!! FREE FALLIN'
  safelyFreeString(s2);
	return result; // Return the result
}

void deleteAlarmListFunction(void *toBeDeleted) {
  Alarm* a = (Alarm*) toBeDeleted;
  if (!a) {
    return; // Cant free nothing
  }
  if (a->trigger) {
    free(a->trigger); // Free trigger if it exists
  }
  clearList(&a->properties); // Clear properties
	free(a); // Bye
}

Property* createProperty(char* propName, char* propDescr) {
  Property* p = malloc(sizeof(Property) + strlen(propDescr)*sizeof(char*) + strlen(propName)); // Allocate room for the property and the flexible array member
  strcpy(p->propName, propName); // Copy prop name over
  strcpy(p->propDescr, propDescr); // Copy prop description over
  return p; // Send it back
}

Alarm* createAlarm(char* action, char* trigger, List properties) {
  if (!action || !trigger) {
    return NULL; // If the action or trigger is null then nothing can save you
  }
  Alarm* alarm = malloc(sizeof(Alarm));
  if (action[0] == ':' || action[0] == ';') { // Remove the beginning ; or : if it exists
    memmove(action, action + 1, strlen(action));
  }
  strcpy(alarm->action, action); // Copy the action
  alarm->trigger = malloc(strlen(trigger) + 1); // Allocate room for trigger

  if (!alarm->trigger) {
    free(alarm); // Free alarm before returning
    return NULL; // If we were unable to allocate memory
  }

  if (trigger[0] == ':' || trigger[0] == ';') { // Remove the beginning ; or : if it exists
    memmove(trigger, trigger + 1, strlen(trigger));
  }
  strcpy(alarm->trigger, trigger); // Copy trigger
  alarm->properties = properties; // Set the properties

  return alarm; // Send it back
}

Alarm* createAlarmFromPropList(List props) {
  // Create a list for the props
  List alarmProps = initializeList(&printPropertyListFunction, &deletePropertyListFunction, &comparePropertyListFunction);

  char* ACTION = NULL; // Declare action and trigger
  char* TRIGGER = NULL;

  Property* prop; // Declare prop
  ListIterator propsIterator = createIterator(props); // Do I really need to re-iterate myself here?

  while ((prop = nextElement(&propsIterator)) != NULL) {
    char* propName = prop->propName; // Get name
    char* propDescr = prop->propDescr; // Get description

    if (strcmp(propDescr, "") == 0 || strcmp(propName, "") == 0) { // If no descripting, we are in trouble
      clearList(&alarmProps); // Clear before returning
      return NULL; // Bye
    }
    char tempDescription[strlen(propDescr) + 2]; //
    strcpy(tempDescription, propDescr);
    if (match(tempDescription, "^(;|:)")) {
      memmove(tempDescription, tempDescription+1, strlen(tempDescription)); // remove the first character as it is (; or :)
    }
    if (match(propName, "^ACTION$")) {
      if (ACTION || !match(tempDescription, "^(AUDIO|DISPLAY|EMAIL)$")) {
        clearList(&alarmProps);
        return NULL; // Already have an ACTION or description is null
      }
      ACTION = malloc(strlen(tempDescription) + 1);
      strcpy(ACTION, tempDescription);

    } else if (match(propName, "^TRIGGER$")) {
      if (TRIGGER) {
        clearList(&alarmProps);
        return NULL; // Already have trigger
      } else {
        TRIGGER = malloc(strlen(tempDescription) + 1);
        strcpy(TRIGGER, tempDescription);
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
    safelyFreeString(ACTION);
    safelyFreeString(TRIGGER);
    return NULL;
  }
  Alarm* a = createAlarm(ACTION, TRIGGER, alarmProps);
  safelyFreeString(ACTION);
  safelyFreeString(TRIGGER);
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
  size_t lineLength = strlen(line);
  char tempLine[lineLength + 1];
  strcpy(tempLine, line);
  char* propName = strtok(line, ":;");
  char* temp;
  size_t descriptionLength = 0;
  while ((temp = strtok(NULL, ";:"))) {
    descriptionLength += strlen(temp);
    descriptionLength += 1; // strtok removes the ';'
  }
  // descriptionLength += 1; // strtok removes the ';'
  char propDescr[descriptionLength + 1];
  size_t substring = lineLength - descriptionLength + 1;
  memcpy(propDescr, tempLine + substring, descriptionLength);
  propDescr[descriptionLength] = '\0';
  Property* p = createProperty(propName, propDescr);
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
    if (match(line, "^[a-zA-Z\\-]*(:|;).*\n{0,1}$")) {
      if (line[strlen(line) - 1] == '\n') { // Remove new line from end of line
        if ((int)strlen(line) - 2 >= 0 && line[strlen(line) - 2] == '\r') { // Remove carriage return if it exists
          line[strlen(line) - 2] = '\0';
        } else {
          safelyFreeString(line);
          fclose(file);
          return INV_CAL;
        }
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

void deleteProperty(List* propList, char* line) {
  char temp[strlen(line) + 1]; // Make a temp variable that is allocated because extractPropertyFromLine uses strtok which cannot use a non existant memory address
  strcpy(temp, line);
  Property* p = extractPropertyFromLine(temp);
  safelyFreeString(deleteDataFromList(propList, p));
  free(p);
}

ErrorCode extractBetweenTags(List props, List* extracted, ErrorCode onFailError, char* tag) {
  clearList(extracted); // Clear the list just in case
  ListIterator propsIterator = createIterator(props);

  // Build the regex expressions for begin and end tags
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
      beginCount ++; // Set begin flag
      if (beginCount != 1) {
        safelyFreeString(line);
        return onFailError; // Opened another event without closing the previous
      }
    } else if (match(line, endTag)) {
      endCount ++; // Set end flag
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
    safelyFreeString(line); // Bye
  }
  if (beginCount == 0 && endCount == 0) { // If we didnt parse any tags at all
    return onFailError;
  }

  if (beginCount == 1 && endCount != beginCount) { // If there is no matching end tag
    return onFailError;
  }

  return OK; // If we made it here, we have extracted between the tags
}

// Make a string that is pretty (Just like you)
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

// Creates a time and puts it into the sent event
ErrorCode createTime(Event* event, char* timeString) {
  if (!timeString || !event || !match(timeString, "^(:|;){0,1}[[:digit:]]{8}T[[:digit:]]{6}Z{0,1}$")) { // If its null or doesnt match the required regex
    return INV_CREATEDT;
  }
  char* numberDate;
  char* timeS;
  if (match(timeString, "^(:|;)")) {
    numberDate = extractSubstringBefore(&timeString[1], "T"); // Get substring before the T but start past the (semi)colon
  } else {
    numberDate = extractSubstringBefore(timeString, "T"); // Get the substring before the T
  }
  char* temp = extractSubstringAfter(timeString, "T"); // Extract after the T
  if (match(timeString, "Z$")) { // If UTC
    timeS = extractSubstringBefore(temp, "Z"); // Get the time between the T and Z
    event->creationDateTime.UTC = true; // UTC is true
    safelyFreeString(temp); // Free temp
  } else {
    timeS = temp; // Get the time after T
    event->creationDateTime.UTC = false;
  }
  if (!numberDate || !temp || !timeS) { // We dont have all of the things we need
    safelyFreeString(numberDate); // Free before returning
    safelyFreeString(timeS);
    return INV_CREATEDT; // We failed :'(
  }
  strcpy(event->creationDateTime.date, numberDate); // Copy the values
  strcpy(event->creationDateTime.time, timeS);

  safelyFreeString(numberDate); // Free before returning
  safelyFreeString(timeS);
  return OK; // You're OK but I have a girlfriend, sorry
}

// Remove the properties that are in l2 from l1
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

Event* newEmptyEvent() {
  Event* e = malloc(sizeof(Event)); // MAKE ROOM FOR ME, GOSH!
  e->properties = initializeList(&printPropertyListFunction, &deletePropertyListFunction, &comparePropertyListFunction); // Set the lists
  e->alarms = initializeList(&printAlarmListFunction, &deleteAlarmListFunction, &compareAlarmListFunction);
  return e; // We done
}

List copyPropList(List toBeCopied) {
  List newList = initializeList(&printPropertyListFunction, &deletePropertyListFunction, &comparePropertyListFunction); // Make a new list

  ListIterator iter = createIterator(toBeCopied); // Make an iterator for ... well ... iterating
  Property* p;

  while ((p = nextElement(&iter)) != NULL) {
    char* c = toBeCopied.printData(p); // Print the data
    Property* p = extractPropertyFromLine(c); // Extract a property from the line
    insertBack(&newList, p); // ... And the bus driver said, to the back, to the back
    safelyFreeString(c); // Bye
  }
  return newList; // Return the new list
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
      insertBack(&event->alarms, a); // Put the alarm in
      removeIntersectionOfLists(&newEventList, alarmPropList); // Remove all elements from alarmPropList in newEventList
      deleteProperty(&newEventList, "BEGIN:VALARM"); // Remove Begin and end tags
      deleteProperty(&newEventList, "END:VALARM");
    } else {
      clearList(&newEventList); // Clear lists before returning
      clearList(&alarmPropList);
      return INV_EVENT; // Invalid event
    }
    clearList(&alarmPropList); // Clear the list
  }

  ListIterator eventIterator = createIterator(newEventList); // Iterate over remaining props
  Property* prop;

  char* UID = NULL; // Placeholders for uid and dstamp
  char* DTSTAMP = NULL;

  while ((prop = nextElement(&eventIterator)) != NULL) {
    char* propName = prop->propName; // make these for better readability
    char* propDescr = prop->propDescr;
    if (strcmp(propDescr, "") == 0 || strcmp(propName, "") == 0) { // If no descripting, we are in trouble
      safelyFreeString(UID); // Free strings before returning
      safelyFreeString(DTSTAMP);
      clearList(&newEventList); // Clear list before returning
      return INV_EVENT;
    }
    if (match(propName, "^UID$")) { // If this is the UID
      if (UID != NULL || !propDescr || !strlen(propDescr)) { // If there is a problem with it
        safelyFreeString(UID); // Free strings before returning
        safelyFreeString(DTSTAMP);
        clearList(&newEventList); // Clear list before returning
        return INV_EVENT; // UID has already been assigned or propDesc is null or empty
      }
      if (match(propDescr, "^(;|:)")) { // If the description starts with (semi)colon
        char temp[strlen(propDescr)];
        memcpy(temp, propDescr + 1*sizeof(char), strlen(propDescr)); // Shift the memory to get rid of the (semi)colon
        strcpy(event->UID, temp); // Copy it over
      } else {
        strcpy(event->UID, propDescr); // Copy it over
      }

      UID = eventList.printData(prop); // Set the UID
    } else if (match(propName, "^DTSTAMP$")) {
      if (DTSTAMP != NULL || !propDescr) { // If the date is problematic
        safelyFreeString(UID); // Free before returning
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

  event->properties = newEventList; // Set properties
  clearList(&alarmPropList);

  return OK;
}

ErrorCode parseRequirediCalTags(List* list, Calendar* cal) {
  ListIterator iterator = createIterator(*list); // Iterate over props
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
      if (!description || !match(description, "^(:|;){0,1}[[:digit:]]+(\\.[[:digit:]]+)*$")) {
        safelyFreeString(PRODID); // PROD might be allocated so we must remove it before returning
        safelyFreeString(VERSION); // Version might be allocated so we must remove it before returning
        return INV_VER;
      }

      VERSION = malloc(strlen(description) * sizeof(char) + 1);
      if (match(description, "^(;|:)")) {
        memmove(VERSION, description+1, strlen(description)); // remove the first character as it is (; or :)
      } else {
        strcpy(VERSION, description);
      }
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
      PRODID = malloc(strlen(description) * sizeof(char) + 1);
      if (match(description, "^(;|:)")) {
        memmove(PRODID, description+1, strlen(description)); // remove the first character as it is (; or :)
      } else {
        printf("nope: %s\n", description);
        strcpy(PRODID, description);
      }
    }
  }

  if (!VERSION || !PRODID) {
    safelyFreeString(PRODID); // PROD might be allocated so we must remove it before returning
    safelyFreeString(VERSION); // Version might be allocated so we must remove it before returning
    return INV_CAL; // We are missing required tags
  }

  cal->version = atof(VERSION);
  strcpy(cal->prodID, PRODID);
  safelyFreeString(PRODID); // Free strings before returning
  safelyFreeString(VERSION);
  return OK;
}

void updateLongestLineAndIncrementStringSize(size_t* longestLine, size_t* lineLength, size_t* stringSize) {
  if (*lineLength > *longestLine) { // If this line is greater than the previous longest line
    *longestLine = *lineLength; // Congrats, you are the new longest line
  }
  *stringSize += *lineLength; // Add this line length to the string size
  *lineLength = 0; // Reset the line length
}

void calculateLineLength(size_t* lineLength, const char* c, ... ) {
   va_list valist;
   va_start(valist, c);

   // Iterate over all variable params
   while (c) {
        *lineLength += strlen(c); // Add the length of this string to the total
        c = va_arg(valist, const char*); // Move to the next
    }
   // Clear the va_list
   va_end(valist);
}

void concatenateLine(char* string, const char* c, ... ) {
   va_list valist;
   va_start(valist, c);

    // Iterate over all variable params
   while (c) {
        strcat(string, c); // concatenate each value onto the string
        c = va_arg(valist, const char*); // Move to the next
    }
   // Clear the va_list
   va_end(valist);
}

// If you made it this far, you win. Too bad the prize is nothing
