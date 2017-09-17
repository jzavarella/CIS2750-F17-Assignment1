/*

 * CIS2750 F2017

 * Assignment 0

 * Jackson Zavarella 0929350

 * This file contains the implementation of the linked List API.

 * No code was used from previous classes/ sources

 */



#include "../include/CalendarParser.h"

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

}


/** Function to delete all calendar content and free all the memory.
 *@pre Calendar object exists, is not null, and has not been freed
 *@post Calendar object had been freed
 *@return none
 *@param obj - a pointer to a Calendar struct
**/
void deleteCalendar(Calendar* obj) {

}


/** Function to create a string representation of a Calendar object.
 *@pre Calendar object exists, is not null, and is valid
 *@post Calendar has not been modified in any way, and a string representing the Calndar contents has been created
 *@return a string contaning a humanly readable representation of a Calendar object
 *@param obj - a pointer to a Calendar struct
**/
char* printCalendar(const Calendar* obj) {

}


/** Function to "convert" the ErrorCode into a humanly redabale string.
 *@return a string contaning a humanly readable representation of the error code by indexing into
          the descr array using rhe error code enum value as an index
 *@param err - an error code
**/
const char* printError(ErrorCode err) {
  switch (err) {
    case 0: // OK
      return "OK";
    case 1: // INV_FILE
      return "Invalid File";
    case 2: // INV_CAL
      return "Invalid Calendar";
    case 3: // INV_VER
      return "Malformed Version";
    case 4: // DUP_VER
      return "Non-Unique Version";
    case 5: // INV_PRODID
      return "Malformed Product ID";
    case 6: // DUP_PRODID
      return "Non-Unique Product ID";
    case 7: // INV_EVENT
      return "Malformed Event";
    case 8: // INV_CREATEDT
      return "Malformed Date-Time";
    default:
      return "NULL";
  }
}
