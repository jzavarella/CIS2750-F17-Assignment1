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
        return INV_CAL; // This is the first line in the list and the first line cannot be a line continuation
      }

      Property* p = (Property*) tailNode->data;
      if (!p) {
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
