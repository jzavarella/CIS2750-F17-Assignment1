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
int match(const char* string, char* pattern); // Matches a given string against aregex expression
void safelyFreeString(char* c); // Frees a string but checks to see if it is null first
Property* createProperty(char* propName, char* propDescr); // Create a property from a name and a description
Alarm* createAlarm(char* action, char* trigger, List properties); // Create an alarm given and action and a trigger and a list of properties
Alarm* createAlarmFromPropList(List props); // Creates an alarm from a list of properties
char* extractSubstringBefore(char* line, char* terminator); // returns a copy of the string up to the terminator
char* extractSubstringAfter(char* line, char* terminator); // Returns a copy of the string after the terminator
Property* extractPropertyFromLine(char* line); // Given a line, extract a property from it
int matchTEXTField(const char* propDescription); // checks to see if a string matches a valid ICAL TEXT field
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
// Deletes a property from a list given the string representation of it
void deleteProperty(List* propList, char* line);
ErrorCode extractBetweenTags(List props, List* extracted, ErrorCode onFailError, char* tag);
char* printDatePretty(DateTime dt); // Prints a pretty version of a date
ErrorCode createTime(Event* event, char* timeString); // Creates a DateTime and allocates it to the given event if the timeString can be parsed
void removeIntersectionOfLists(List* l1, List l2); // Removes all nodes from l1 that are found in l2
Event* newEmptyEvent(); // Creates an empty event
List copyPropList(List toBeCopied); // Returns a new list with the sent list's nodes copied into it
void updateLongestLineAndIncrementStringSize(size_t* longestLine, size_t* lineLength, size_t* stringSize); // Calculates the longest line
void concatenateLine(char* string, const char* c, ... ); // concatenate all 'c's to string
void calculateLineLength(size_t* lineLength, const char* c, ... ); // Sums the length of all strings sent to it

/**
  *Main function to create an event
*/
ErrorCode createEvent(List eventList, Event* event);
/**
  *Checks to see if the required iCalendar tags are present
*/
ErrorCode parseRequirediCalTags(List* list, Calendar* cal);
